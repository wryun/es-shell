dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.64])
AC_INIT
AC_CONFIG_SRCDIR([access.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

dnl AC_CMDSTDOUT_CPP(variable, command, headers)
AC_DEFUN([AC_CMDSTDOUT_CPP],
[cat > conftest.$ac_ext <<EOF
[#]line __oline__ "configure"
#include "confdefs.h"
[$3]
EOF
$1=`(eval "$ac_cpp conftest.$ac_ext") 2>&AS_MESSAGE_LOG_FD | $2`
rm -f conftest*
])

AC_CANONICAL_HOST

dnl Build configuration.

AC_ARG_ENABLE(strict, AS_HELP_STRING([--enable-strict], [enable strict compiler flags for testing and debugging; not all compilers support these]), opt_strict_mode=$enableval, opt_strict_mode=no)

AC_ARG_VAR(YACC, [yacc command. es requires bison, byacc, or something else that supports generating pure parsers.])

if test $opt_strict_mode = yes; then
	AC_SUBST([STRICT_CFLAGS], ["-ansi -pedantic -DGCDEBUG=1 -D_POSIX_C_SOURCE=200112L -DREF_ASSERTIONS=1"])
fi

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MKDIR_P

dnl Custom yacc check, because we don't want POSIX yacc.
dnl If we don't find either bison or byacc, just set it to yacc and see if it
dnl works or barfs on the extension.

AC_CHECK_PROGS(YACC, bison byacc, yacc)

dnl ----------------------------
dnl CHECK FOR /dev/fd FILESYSTEM
dnl ----------------------------
AC_CACHE_CHECK(for /dev/fd filesystem, es_cv_sys_dev_fd,
[test -d /dev/fd && es_cv_sys_dev_fd=yes || es_cv_sys_dev_fd=no])
if test $es_cv_sys_dev_fd = yes; then
  AC_DEFINE(HAVE_DEV_FD, [1], [Do you have a /dev/fd/ directory?])
fi

AC_SYS_INTERPRETER
if test "$ac_cv_sys_interpreter" = yes
then
	AC_DEFINE(KERNEL_POUNDBANG, [1], [Does your kernel support #!?])
fi


dnl Checks for libraries.

AC_CHECK_LIB(sun, getpwuid)

ES_WITH_READLINE

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h limits.h sys/ioctl.h sys/time.h unistd.h memory.h stdarg.h sys/cdefs.h)


dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_SIZE_T

dnl Checks for library functions.
AC_TYPE_GETGROUPS
AC_FUNC_MMAP

AC_CHECK_FUNCS(strerror strtol lstat setrlimit sigrelse sighold sigaction \
sysconf sigsetjmp getrusage mmap mprotect)

AC_CACHE_CHECK(whether getenv can be redefined, es_cv_local_getenv,
[if test "$ac_cv_header_stdlib_h" = no || test "$ac_cv_header_stdc" = no; then
	es_cv_local_getenv=yes
fi
if test "$ac_cv_header_stdlib_h" = yes || test "$ac_cv_header_stdc" = yes; then
AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdlib.h>
static char *sentinel = "a value";
static int   in_main  = 0;

char *getenv(const char *name) { return in_main ? sentinel : NULL; }

int main(int argc, char **argv) {
	char *v;
	in_main = 1;
	v = getenv("key");
	return (v == sentinel) ? 0 : 1;
}
]])],[es_cv_local_getenv=yes],[es_cv_local_getenv=no])
fi
])

if test "$es_cv_local_getenv" = yes
then
	AC_DEFINE(LOCAL_GETENV, [1], [Can getenv() be defined locally?])
fi

dnl Check to see if you can assign to a va_list
AC_CACHE_CHECK(whether assignment to va_list ok?, es_cv_assign_va_list,
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#ifndef HAVE_STDARG_H
choke me
#else
#include <stdarg.h>
#endif]], [[va_list first, second; first = second; return 0;]])],[es_cv_assign_va_list=yes],[es_cv_assign_va_list=no]))
if test "$es_cv_assign_va_list" = no
then
	AC_DEFINE(NO_VA_LIST_ASSIGN, [1], [NO_VA_LIST_ASSIGN])
fi

dnl check for a u_quad_t or something like that
AC_CACHE_CHECK(for rlimit type ..., es_cv_rlimit_t,
AC_CMDSTDOUT_CPP(es_cv_rlimit_t, 
grep rlim_cur | sed -e 's/rlim_cur.*//' -e 's/^ //g' -e 's/^	//g' -e q,
[#ifdef HAVE_SETRLIMIT
# include <sys/resource.h>
#endif],long))

AC_CACHE_CHECK(for files to extract signal information from,
es_cv_sigfiles,
AC_CMDSTDOUT_CPP(es_cv_sigfiles,
[changequote(,)
grep -E '^#[ 	]+1[	 ]+' | sed 's/.*"\(.*\)".*/\1/' |sort -u |  
grep '^/' |tr '\012' ' ' 
changequote([,])],
[#include <signal.h>], /usr/include/signal.h))

AC_SUBST(SIGFILES, $es_cv_sigfiles)

AC_DEFINE_UNQUOTED(LIMIT_T, $es_cv_rlimit_t, [What type are your limits?])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
