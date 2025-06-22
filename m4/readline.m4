# _ES_CHECK_READLINE([CFLAGS], [LIBS], [ACTION-IF-SUCCESS], [ACTION-IF-FAILURE])
# ------------------------------------------------------------------------------
# Any CFLAGS provided are cached in es_cv_readline_cflags if it isn't already set.
# The first working library combination is cached in es_cv_readline_libs.
AC_DEFUN([_ES_CHECK_READLINE], [
  AC_REQUIRE([AC_PROG_CC])dnl
  AS_VAR_COPY([saved_CFLAGS], [CFLAGS])
  AS_VAR_COPY([saved_LIBS], [LIBS])
  AS_IF([test -n "$1"], [
    AS_VAR_SET([es_cv_readline_cflags], ["$1"])
    AS_VAR_APPEND([CFLAGS], [" $1"])
  ])
  AC_LANG_CONFTEST([AC_LANG_PROGRAM([[
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
  ]], [[
    char *line = readline("prompt> ");
    if (!line) add_history(line);
    return line != (char *)0;
  ]])])
  AC_CACHE_CHECK([for readline libraries], [es_cv_readline_libs], [
    AS_IF([test -n "$2"], [
      AS_VAR_APPEND([LIBS], [" $2"])
      AC_LINK_IFELSE([], [
        AS_VAR_SET([es_cv_readline_libs], ["$2"])
      ], [
        AS_VAR_SET([es_cv_readline_libs], [no])
      ])
    ], [
      dnl The m4_foreach macro expands into what is effectively an unrolled loop.
      dnl It can make Autoconf time slightly longer, but it means configure can be faster.
      m4_foreach([TERMLIB], [,curses,tinfo,ncurses,terminfo,termcap], [
        AS_VAR_SET_IF([es_cv_readline_libs], [
        ], [
          m4_pushdef([es_Rllibs], [-lreadline]m4_ifval(TERMLIB, [ -l]TERMLIB))dnl
          AS_VAR_COPY([LIBS], [saved_LIBS])
          AS_VAR_APPEND([LIBS], [" ]es_Rllibs["])
          AC_LINK_IFELSE([], [AS_VAR_SET([es_cv_readline_libs], ["[]es_Rllibs[]"])])
          m4_popdef([es_Rllibs])dnl
        ])
      ])dnl
      AS_VAR_SET_IF([es_cv_readline_libs], [
      ], [
        AS_VAR_SET([es_cv_readline_libs], [no])
      ])
    ])
  ])
  rm -f conftest.$ac_ext
  AS_VAR_COPY([LIBS], [saved_LIBS])
  AS_VAR_COPY([CFLAGS], [saved_CFLAGS])
  AS_VAR_IF([es_cv_readline_libs], [no], [$4], [$3])
])# _ES_CHECK_READLINE

# ES_CHECK_READLINE([CFLAGS], [LIBS], [ACTION-IF-SUCCESS], [ACTION-IF-FAILURE])
# -----------------------------------------------------------------------------
# Verify that a program using readline functions and headers successfully compiles and links.
# Required/desired compiler flags may be specified in the first argument and will be cached in es_cv_readline_cflags.
# Similarly, -l'library' arguments for linking may be specified in the second argument.
#
# If the list of libraries is empty, linking is attempted using -lreadline alone.
# If linking fails, additional attempts are tried with one of the following terminal capability library arguments appended:
# - curses
# - tinfo
# - terminfo
# - termcap
#
# If linking fails after all attempts, es_cv_readline_libs is set to "no", and ACTION-IF-FAILURE is executed.
# Otherwise, es_cv_readline_libs contains the required libraries, the HAVE_READLINE preprocessor macro is defined, and ACTION-IF-SUCCESS is executed.
AC_DEFUN([ES_CHECK_READLINE], [
  _ES_CHECK_READLINE($@)
  AS_VAR_IF([es_cv_readline_libs], [no], [
    # Ensure we don't attempt to link es with readline.
    AC_SUBST([READLINE_CFLAGS], [])
    AC_SUBST([READLINE_LIBS], [])
  ], [
    AC_DEFINE([HAVE_READLINE], [1], [Define to 1 if you have readline.])
    AC_SUBST([READLINE_CFLAGS], [${es_cv_readline_cflags-}])
    AC_SUBST([READLINE_LIBS], [$es_cv_readline_libs])
  ])
])# ES_CHECK_READLINE

# ES_WITH_READLINE([ACTION-IF-SUCCESS], [ACTION-IF-FAILURE])
# ----------------------------------------------------------
# A simplified version of ES_CHECK_READLINE that additionally creates a --with-readline configure flag.
#
# Two precious variables, READLINE_CFLAGS and READLINE_LIBS, are also supported.
# The value of READLINE_CFLAGS is passed as the CFLAGS argument to ES_CHECK_READLINE.
# Similarly, READLINE_LIBS is the LIBS argument to ES_CHECK_READLINE.
# These are substituted into the generated Makefile.
#
# By default, if readline support is required using --with-readline=yes and ES_CHECK_READLINE fails, ACTION-IF-FAILURE uses AC_MSG_ERROR to terminate configure with an error message.
AC_DEFUN([ES_WITH_READLINE], [
  AC_ARG_WITH([readline],
              [AS_HELP_STRING([--with-readline],
                              [build with readline support @<:@default=check@:>@])],
              [],
              [AS_VAR_SET([with_readline], [check])])
  AC_ARG_VAR([READLINE_CFLAGS], [C compiler flags for readline])
  AC_ARG_VAR([READLINE_LIBS], [linker flags for readline])

  m4_pushdef([ES_REQUIRED_ERROR],
             [AC_MSG_FAILURE([readline support requested but unavailable])])dnl
  m4_pushdef([ES_DEFAULT_ACTIONS_IF_REQUIRED],
             m4_dquote(m4_case([$#],
                               [0], [[], ES_REQUIRED_ERROR],
                               [1], [[$1], ES_REQUIRED_ERROR],
                               [[$1], [$2]])))dnl
  m4_popdef([ES_REQUIRED_ERROR])dnl

  AS_CASE(["$with_readline"],
          [no],         [],
          [auto|check], [ES_CHECK_READLINE([${READLINE_CFLAGS-}],
                                           [${READLINE_LIBS-}],
                                           [$1], [$2])],
          [yes],        [ES_CHECK_READLINE([${READLINE_CFLAGS-}],
                                           [${READLINE_LIBS-}],
                                           ES_DEFAULT_ACTIONS_IF_REQUIRED)],
          [AC_MSG_ERROR([--with-readline: valid values are "yes", "no", "check" -- got "$with_readline"])])

  m4_popdef([ES_DEFAULT_ACTIONS_IF_REQUIRED])dnl
])# ES_WITH_READLINE
