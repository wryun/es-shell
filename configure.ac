dnl Process this file with autoconf to produce a configure script.
AC_INIT
AC_CONFIG_SRCDIR([access.c])
AC_CONFIG_HEADER(config.h)


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


use_readline=yes
use_editline=no

AC_ARG_WITH(readline,
--with-readline		Use GNU Readline, use_readline=yes)

AC_CANONICAL_HOST

case "$host" in
	*sun5* | *solaris2*)
		AC_DEFINE(SOLARIS, [], [Are we Solaris?])
	;;
esac

dnl saved_CFLAGS="$CFLAGS"

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MKDIR_P
AC_PROG_YACC

dnl CFLAGS="$CFLAGS $saved_CFLAGS"

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

if test "$use_readline" = yes || test "$use_editline" = yes
then
	AC_CHECK_LIB(terminfo, main)
	AC_CHECK_LIB(termcap, main)
	if test "$use_readline" = yes
	then
		AC_CHECK_LIB(readline, readline)
	elif test "$use_editline" = yes
	then
		AC_CHECK_LIB(edit, readline)
	fi

fi


dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h limits.h sys/ioctl.h sys/time.h unistd.h memory.h stdarg.h sys/cdefs.h)


dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_SIZE_T

dnl Checks for library functions.
AC_TYPE_GETGROUPS
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MMAP

AC_FUNC_WAIT3
AC_CHECK_FUNCS(strerror strtol lstat setrlimit sigrelse sighold sigaction \
sysconf setsid sigsetjmp)

AC_CACHE_CHECK(for an abused getenv, es_cv_abused_getenv,
AC_RUN_IFELSE([AC_LANG_SOURCE([[
changequote(,)
char *foo = 0;
char *foov[] = { "-a", "-bfoo", "bar" };
int fooc = (sizeof(foov) / sizeof(foov[0]));

int getenv() { return *foo; }
int main() { while (-1 != getopt(fooc, foov, "ab:")); exit(0); }
changequote([,])
]])],[es_cv_abused_getenv=no],[es_cv_abused_getenv=yes
AC_DEFINE([ABUSED_GETENV], [1], [Is getenv called by crt*.o or getopt?])],[]))

if test "$es_cv_abused_getenv" = yes
then
	rm -f core conftest.core
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
