dnl ==========================================================================
dnl File: configure.ac
dnl
dnl $URL$
dnl $Id$
dnl
dnl Description
dnl
dnl Revision History
dnl
dnl The revision history contains the date of a change, the author of the
dnl change and in the description at a minimum the Problem Report that
dnl required the change.
dnl
dnl  Date    Author   Description
dnl  ------- -------- --------------------------------------------------------
dnl  23Oct12 jcurl    Initial Version
dnl
dnl To Do:
dnl
dnl Notes:
dnl 
dnl ==========================================================================

dnl Check that we have Autoconf version 2.61 as a minimum. We probably don't
dnl need this, but hopefully it forces the developer to upgrade to the latest
dnl in any case.
AC_PREREQ(2.61)

dnl Initialise the configuration file. Make sure that when there are version
dnl changes, the version field here get's updated.
AC_INIT([CpuTemp], [1.0.0], [jcurl@arcor.de], [cputemptest])

dnl We want to have all the intermediate configuration files in a separate
dnl directory for cleanliness.
AC_CONFIG_SRCDIR(cputemp/cputemp.c)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR([m4])

dnl Plug in the revision of this file
AC_REVISION([$Id$])

dnl Figure out what we're building
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

dnl Configure the Automake tools
AM_INIT_AUTOMAKE([dist-bzip2])

dnl This speeds up some of the 'reconfigure' options if the user has auto-
dnl tools installed on their machine after CVS update. It does mean though
dnl that developers MUST run autoreconf before checking in their files to
dnl ensure that the 'Makefile.in's correspond correctly to all 'Makefile.am's
AM_MAINTAINER_MODE

dnl We are using headers to include that define the features of the software
AM_CONFIG_HEADER(cputemp/config.h)

AC_LANG([C])

dnl Add GNU support if it exists
AC_GNU_SOURCE

dnl Check for the compiler
AC_PROG_CC
AM_PROG_CC_C_O

dnl Check for the installer
AC_PROG_INSTALL

dnl Because we've got multiple Makefiles, we'll define the MAKE variable
AC_PROG_MAKE_SET

dnl =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
dnl Checks for particular features
dnl =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

dnl ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
dnl Check that we're using an ANSI C compiler. If we're not using an ANSI C
dnl compiler then 'configure' will exit fatally. The source software just
dnl includes the headers assuming ANSI.
AC_HEADER_STDC
if test "x$ac_cv_header_stdc" != xyes; then
  AC_MSG_ERROR([
**
** Standard C libraries don't appear to be present. 
** Please use an ANSI compliant C compiler (e.g. GNU CC)
**])
fi

dnl Check for headers
AC_CHECK_HEADERS([itemp.h], [], 
  [AC_CHECK_HEADERS([$PWD/${srcdir}/module/itemp.h], 
    [KERNEL_CFLAGS="-I$PWD/${srcdir}/module"
     AC_SUBST(KERNEL_CFLAGS)], 
    [AC_MSG_ERROR([
**
** You need to provide the location of the itemp header
** e.g. ./configure CFLAGS="-I\$PWD/module"
**])], [])], [])

AC_ARG_ENABLE(kernelmodule)
AC_MSG_CHECKING([if we should build the kernel module])
KERNEL_MODULE=yes
# Disable compiling the kernel module if explicitly told to do so
if test x$enable_kernelmodule == xno; then
  KERNEL_MODULE=no
fi
# Don't support cross-compiling the kernel module
if test x$cross_compiling == xyes; then
  KERNEL_MODULE=no
fi
# Don't build if it isn't present (e.g. VPATH builds)
if test ! -e ./module/Kbuild; then
  KERNEL_MODULE=no
fi
AM_CONDITIONAL([BUILD_KERNEL_MODULE], [test $KERNEL_MODULE == "yes"])
AC_MSG_RESULT([$KERNEL_MODULE])

dnl =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
dnl We're near the end now
dnl =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

dnl Generate Makefile from Makefile.in
AC_CONFIG_FILES([Makefile
                 cputemp/Makefile])

dnl Generate the makefile
AC_OUTPUT
