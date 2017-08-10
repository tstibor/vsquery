#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])

# Get version information from git.
m4_define([GIT_VERSION_STRING], m4_esyscmd([git describe --tags | sed 's/\(.*\)-.*/\1/' | tr -d '\n']))

AC_INIT([vsquery], [GIT_VERSION_STRING], [t.stibor@gsi.de], [], [https://github.com/tstibor/vsquery])
AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([src/vsquery.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_STDC

# Use the C language and compiler for the following checks.
AC_LANG([C])

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h])

# Propage flags and dirs among final Makefiles.
AC_SUBST([AM_CFLAGS])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_FUNC_STRTOD
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_LIB([m], [sqrt],
		     [], [AC_MSG_ERROR([cannot find math library, provide library path e.g. ./configure LDFLAGS='-L/<PATH_TO_LIB>'])])

AC_CHECK_LIB([curl], [curl_global_init],
		     [], [AC_MSG_ERROR([cannot find curl library, provide library path e.g. ./configure LDFLAGS='-L/<PATH_TO_LIB>'])])

AC_CHECK_LIB([nova], [ln_get_body_next_rst_horizon],
		     [], [AC_MSG_ERROR([cannot find nova library, provide library path e.g. ./configure LDFLAGS='-L/<PATH_TO_LIB>'])])

AC_CHECK_FUNCS([gettimeofday sqrt strstr strtol])

AC_CONFIG_FILES([Makefile
		 src/Makefile])

# Remove unneeded libraries.
LDFLAGS="$LDFLAGS -Wl,--as-needed"

# Halt on warnings.
CFLAGS="$CFLAGS -Wall -Werror"

AC_OUTPUT
