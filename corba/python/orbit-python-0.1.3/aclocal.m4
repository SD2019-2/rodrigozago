dnl aclocal.m4 generated automatically by aclocal 1.4

dnl Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.

# Do all the work for Automake.  This macro actually does too much --
# some checks are only needed if your package does certain things.
# But this isn't really a big deal.

# serial 1

dnl Usage:
dnl AM_INIT_AUTOMAKE(package,version, [no-define])

AC_DEFUN(AM_INIT_AUTOMAKE,
[AC_REQUIRE([AC_PROG_INSTALL])
PACKAGE=[$1]
AC_SUBST(PACKAGE)
VERSION=[$2]
AC_SUBST(VERSION)
dnl test to see if srcdir already configured
if test "`cd $srcdir && pwd`" != "`pwd`" && test -f $srcdir/config.status; then
  AC_MSG_ERROR([source directory already configured; run "make distclean" there first])
fi
ifelse([$3],,
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Name of package])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Version number of package]))
AC_REQUIRE([AM_SANITY_CHECK])
AC_REQUIRE([AC_ARG_PROGRAM])
dnl FIXME This is truly gross.
missing_dir=`cd $ac_aux_dir && pwd`
AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
AM_MISSING_PROG(MAKEINFO, makeinfo, $missing_dir)
AC_REQUIRE([AC_PROG_MAKE_SET])])

#
# Check to make sure that the build environment is sane.
#

AC_DEFUN(AM_SANITY_CHECK,
[AC_MSG_CHECKING([whether build environment is sane])
# Just in case
sleep 1
echo timestamp > conftestfile
# Do `set' in a subshell so we don't clobber the current shell's
# arguments.  Must try -L first in case configure is actually a
# symlink; some systems play weird games with the mod time of symlinks
# (eg FreeBSD returns the mod time of the symlink's containing
# directory).
if (
   set X `ls -Lt $srcdir/configure conftestfile 2> /dev/null`
   if test "[$]*" = "X"; then
      # -L didn't work.
      set X `ls -t $srcdir/configure conftestfile`
   fi
   if test "[$]*" != "X $srcdir/configure conftestfile" \
      && test "[$]*" != "X conftestfile $srcdir/configure"; then

      # If neither matched, then we have a broken ls.  This can happen
      # if, for instance, CONFIG_SHELL is bash and it inherits a
      # broken ls alias from the environment.  This has actually
      # happened.  Such a system could not be considered "sane".
      AC_MSG_ERROR([ls -t appears to fail.  Make sure there is not a broken
alias in your environment])
   fi

   test "[$]2" = conftestfile
   )
then
   # Ok.
   :
else
   AC_MSG_ERROR([newly created file is older than distributed files!
Check your system clock])
fi
rm -f conftest*
AC_MSG_RESULT(yes)])

dnl AM_MISSING_PROG(NAME, PROGRAM, DIRECTORY)
dnl The program must properly implement --version.
AC_DEFUN(AM_MISSING_PROG,
[AC_MSG_CHECKING(for working $2)
# Run test in a subshell; some versions of sh will print an error if
# an executable is not found, even if stderr is redirected.
# Redirect stdin to placate older versions of autoconf.  Sigh.
if ($2 --version) < /dev/null > /dev/null 2>&1; then
   $1=$2
   AC_MSG_RESULT(found)
else
   $1="$3/missing $2"
   AC_MSG_RESULT(missing)
fi
AC_SUBST($1)])

# Add --enable-maintainer-mode option to configure.
# From Jim Meyering

# serial 1

AC_DEFUN(AM_MAINTAINER_MODE,
[AC_MSG_CHECKING([whether to enable maintainer-specific portions of Makefiles])
  dnl maintainer-mode is disabled by default
  AC_ARG_ENABLE(maintainer-mode,
[  --enable-maintainer-mode enable make rules and dependencies not useful
                          (and sometimes confusing) to the casual installer],
      USE_MAINTAINER_MODE=$enableval,
      USE_MAINTAINER_MODE=no)
  AC_MSG_RESULT($USE_MAINTAINER_MODE)
  AM_CONDITIONAL(MAINTAINER_MODE, test $USE_MAINTAINER_MODE = yes)
  MAINT=$MAINTAINER_MODE_TRUE
  AC_SUBST(MAINT)dnl
]
)

# Define a conditional.

AC_DEFUN(AM_CONDITIONAL,
[AC_SUBST($1_TRUE)
AC_SUBST($1_FALSE)
if $2; then
  $1_TRUE=
  $1_FALSE='#'
else
  $1_TRUE='#'
  $1_FALSE=
fi])


# serial 40 AC_PROG_LIBTOOL
AC_DEFUN(AC_PROG_LIBTOOL,
[AC_REQUIRE([AC_LIBTOOL_SETUP])dnl

# Save cache, so that ltconfig can load it
AC_CACHE_SAVE

# Actually configure libtool.  ac_aux_dir is where install-sh is found.
CC="$CC" CFLAGS="$CFLAGS" CPPFLAGS="$CPPFLAGS" \
LD="$LD" LDFLAGS="$LDFLAGS" LIBS="$LIBS" \
LN_S="$LN_S" NM="$NM" RANLIB="$RANLIB" \
DLLTOOL="$DLLTOOL" AS="$AS" OBJDUMP="$OBJDUMP" \
${CONFIG_SHELL-/bin/sh} $ac_aux_dir/ltconfig --no-reexec \
$libtool_flags --no-verify $ac_aux_dir/ltmain.sh $host \
|| AC_MSG_ERROR([libtool configure failed])

# Reload cache, that may have been modified by ltconfig
AC_CACHE_LOAD

# This can be used to rebuild libtool when needed
LIBTOOL_DEPS="$ac_aux_dir/ltconfig $ac_aux_dir/ltmain.sh"

# Always use our own libtool.
LIBTOOL='$(SHELL) $(top_builddir)/libtool'
AC_SUBST(LIBTOOL)dnl

# Redirect the config.log output again, so that the ltconfig log is not
# clobbered by the next message.
exec 5>>./config.log
])

AC_DEFUN(AC_LIBTOOL_SETUP,
[AC_PREREQ(2.13)dnl
AC_REQUIRE([AC_ENABLE_SHARED])dnl
AC_REQUIRE([AC_ENABLE_STATIC])dnl
AC_REQUIRE([AC_ENABLE_FAST_INSTALL])dnl
AC_REQUIRE([AC_CANONICAL_HOST])dnl
AC_REQUIRE([AC_CANONICAL_BUILD])dnl
AC_REQUIRE([AC_PROG_RANLIB])dnl
AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_PROG_LD])dnl
AC_REQUIRE([AC_PROG_NM])dnl
AC_REQUIRE([AC_PROG_LN_S])dnl
dnl

# Check for any special flags to pass to ltconfig.
#
# the following will cause an existing older ltconfig to fail, so
# we ignore this at the expense of the cache file... Checking this 
# will just take longer ... bummer!
#libtool_flags="--cache-file=$cache_file"
#
test "$enable_shared" = no && libtool_flags="$libtool_flags --disable-shared"
test "$enable_static" = no && libtool_flags="$libtool_flags --disable-static"
test "$enable_fast_install" = no && libtool_flags="$libtool_flags --disable-fast-install"
test "$ac_cv_prog_gcc" = yes && libtool_flags="$libtool_flags --with-gcc"
test "$ac_cv_prog_gnu_ld" = yes && libtool_flags="$libtool_flags --with-gnu-ld"
ifdef([AC_PROVIDE_AC_LIBTOOL_DLOPEN],
[libtool_flags="$libtool_flags --enable-dlopen"])
ifdef([AC_PROVIDE_AC_LIBTOOL_WIN32_DLL],
[libtool_flags="$libtool_flags --enable-win32-dll"])
AC_ARG_ENABLE(libtool-lock,
  [  --disable-libtool-lock  avoid locking (might break parallel builds)])
test "x$enable_libtool_lock" = xno && libtool_flags="$libtool_flags --disable-lock"
test x"$silent" = xyes && libtool_flags="$libtool_flags --silent"

# Some flags need to be propagated to the compiler or linker for good
# libtool support.
case "$host" in
*-*-irix6*)
  # Find out which ABI we are using.
  echo '[#]line __oline__ "configure"' > conftest.$ac_ext
  if AC_TRY_EVAL(ac_compile); then
    case "`/usr/bin/file conftest.o`" in
    *32-bit*)
      LD="${LD-ld} -32"
      ;;
    *N32*)
      LD="${LD-ld} -n32"
      ;;
    *64-bit*)
      LD="${LD-ld} -64"
      ;;
    esac
  fi
  rm -rf conftest*
  ;;

*-*-sco3.2v5*)
  # On SCO OpenServer 5, we need -belf to get full-featured binaries.
  SAVE_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS -belf"
  AC_CACHE_CHECK([whether the C compiler needs -belf], lt_cv_cc_needs_belf,
    [AC_TRY_LINK([],[],[lt_cv_cc_needs_belf=yes],[lt_cv_cc_needs_belf=no])])
  if test x"$lt_cv_cc_needs_belf" != x"yes"; then
    # this is probably gcc 2.8.0, egcs 1.0 or newer; no need for -belf
    CFLAGS="$SAVE_CFLAGS"
  fi
  ;;

ifdef([AC_PROVIDE_AC_LIBTOOL_WIN32_DLL],
[*-*-cygwin* | *-*-mingw*)
  AC_CHECK_TOOL(DLLTOOL, dlltool, false)
  AC_CHECK_TOOL(AS, as, false)
  AC_CHECK_TOOL(OBJDUMP, objdump, false)
  ;;
])
esac
])

# AC_LIBTOOL_DLOPEN - enable checks for dlopen support
AC_DEFUN(AC_LIBTOOL_DLOPEN, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])])

# AC_LIBTOOL_WIN32_DLL - declare package support for building win32 dll's
AC_DEFUN(AC_LIBTOOL_WIN32_DLL, [AC_BEFORE([$0], [AC_LIBTOOL_SETUP])])

# AC_ENABLE_SHARED - implement the --enable-shared flag
# Usage: AC_ENABLE_SHARED[(DEFAULT)]
#   Where DEFAULT is either `yes' or `no'.  If omitted, it defaults to
#   `yes'.
AC_DEFUN(AC_ENABLE_SHARED, [dnl
define([AC_ENABLE_SHARED_DEFAULT], ifelse($1, no, no, yes))dnl
AC_ARG_ENABLE(shared,
changequote(<<, >>)dnl
<<  --enable-shared[=PKGS]  build shared libraries [default=>>AC_ENABLE_SHARED_DEFAULT],
changequote([, ])dnl
[p=${PACKAGE-default}
case "$enableval" in
yes) enable_shared=yes ;;
no) enable_shared=no ;;
*)
  enable_shared=no
  # Look at the argument we got.  We use all the common list separators.
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:,"
  for pkg in $enableval; do
    if test "X$pkg" = "X$p"; then
      enable_shared=yes
    fi
  done
  IFS="$ac_save_ifs"
  ;;
esac],
enable_shared=AC_ENABLE_SHARED_DEFAULT)dnl
])

# AC_DISABLE_SHARED - set the default shared flag to --disable-shared
AC_DEFUN(AC_DISABLE_SHARED, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
AC_ENABLE_SHARED(no)])

# AC_ENABLE_STATIC - implement the --enable-static flag
# Usage: AC_ENABLE_STATIC[(DEFAULT)]
#   Where DEFAULT is either `yes' or `no'.  If omitted, it defaults to
#   `yes'.
AC_DEFUN(AC_ENABLE_STATIC, [dnl
define([AC_ENABLE_STATIC_DEFAULT], ifelse($1, no, no, yes))dnl
AC_ARG_ENABLE(static,
changequote(<<, >>)dnl
<<  --enable-static[=PKGS]  build static libraries [default=>>AC_ENABLE_STATIC_DEFAULT],
changequote([, ])dnl
[p=${PACKAGE-default}
case "$enableval" in
yes) enable_static=yes ;;
no) enable_static=no ;;
*)
  enable_static=no
  # Look at the argument we got.  We use all the common list separators.
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:,"
  for pkg in $enableval; do
    if test "X$pkg" = "X$p"; then
      enable_static=yes
    fi
  done
  IFS="$ac_save_ifs"
  ;;
esac],
enable_static=AC_ENABLE_STATIC_DEFAULT)dnl
])

# AC_DISABLE_STATIC - set the default static flag to --disable-static
AC_DEFUN(AC_DISABLE_STATIC, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
AC_ENABLE_STATIC(no)])


# AC_ENABLE_FAST_INSTALL - implement the --enable-fast-install flag
# Usage: AC_ENABLE_FAST_INSTALL[(DEFAULT)]
#   Where DEFAULT is either `yes' or `no'.  If omitted, it defaults to
#   `yes'.
AC_DEFUN(AC_ENABLE_FAST_INSTALL, [dnl
define([AC_ENABLE_FAST_INSTALL_DEFAULT], ifelse($1, no, no, yes))dnl
AC_ARG_ENABLE(fast-install,
changequote(<<, >>)dnl
<<  --enable-fast-install[=PKGS]  optimize for fast installation [default=>>AC_ENABLE_FAST_INSTALL_DEFAULT],
changequote([, ])dnl
[p=${PACKAGE-default}
case "$enableval" in
yes) enable_fast_install=yes ;;
no) enable_fast_install=no ;;
*)
  enable_fast_install=no
  # Look at the argument we got.  We use all the common list separators.
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:,"
  for pkg in $enableval; do
    if test "X$pkg" = "X$p"; then
      enable_fast_install=yes
    fi
  done
  IFS="$ac_save_ifs"
  ;;
esac],
enable_fast_install=AC_ENABLE_FAST_INSTALL_DEFAULT)dnl
])

# AC_ENABLE_FAST_INSTALL - set the default to --disable-fast-install
AC_DEFUN(AC_DISABLE_FAST_INSTALL, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
AC_ENABLE_FAST_INSTALL(no)])

# AC_PROG_LD - find the path to the GNU or non-GNU linker
AC_DEFUN(AC_PROG_LD,
[AC_ARG_WITH(gnu-ld,
[  --with-gnu-ld           assume the C compiler uses GNU ld [default=no]],
test "$withval" = no || with_gnu_ld=yes, with_gnu_ld=no)
AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_CANONICAL_HOST])dnl
AC_REQUIRE([AC_CANONICAL_BUILD])dnl
ac_prog=ld
if test "$ac_cv_prog_gcc" = yes; then
  # Check if gcc -print-prog-name=ld gives a path.
  AC_MSG_CHECKING([for ld used by GCC])
  ac_prog=`($CC -print-prog-name=ld) 2>&5`
  case "$ac_prog" in
    # Accept absolute paths.
changequote(,)dnl
    [\\/]* | [A-Za-z]:[\\/]*)
      re_direlt='/[^/][^/]*/\.\./'
changequote([,])dnl
      # Canonicalize the path of ld
      ac_prog=`echo $ac_prog| sed 's%\\\\%/%g'`
      while echo $ac_prog | grep "$re_direlt" > /dev/null 2>&1; do
	ac_prog=`echo $ac_prog| sed "s%$re_direlt%/%"`
      done
      test -z "$LD" && LD="$ac_prog"
      ;;
  "")
    # If it fails, then pretend we aren't using GCC.
    ac_prog=ld
    ;;
  *)
    # If it is relative, then search for the first ld in PATH.
    with_gnu_ld=unknown
    ;;
  esac
elif test "$with_gnu_ld" = yes; then
  AC_MSG_CHECKING([for GNU ld])
else
  AC_MSG_CHECKING([for non-GNU ld])
fi
AC_CACHE_VAL(ac_cv_path_LD,
[if test -z "$LD"; then
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}${PATH_SEPARATOR-:}"
  for ac_dir in $PATH; do
    test -z "$ac_dir" && ac_dir=.
    if test -f "$ac_dir/$ac_prog" || test -f "$ac_dir/$ac_prog$ac_exeext"; then
      ac_cv_path_LD="$ac_dir/$ac_prog"
      # Check to see if the program is GNU ld.  I'd rather use --version,
      # but apparently some GNU ld's only accept -v.
      # Break only if it was the GNU/non-GNU ld that we prefer.
      if "$ac_cv_path_LD" -v 2>&1 < /dev/null | egrep '(GNU|with BFD)' > /dev/null; then
	test "$with_gnu_ld" != no && break
      else
	test "$with_gnu_ld" != yes && break
      fi
    fi
  done
  IFS="$ac_save_ifs"
else
  ac_cv_path_LD="$LD" # Let the user override the test with a path.
fi])
LD="$ac_cv_path_LD"
if test -n "$LD"; then
  AC_MSG_RESULT($LD)
else
  AC_MSG_RESULT(no)
fi
test -z "$LD" && AC_MSG_ERROR([no acceptable ld found in \$PATH])
AC_SUBST(LD)
AC_PROG_LD_GNU
])

AC_DEFUN(AC_PROG_LD_GNU,
[AC_CACHE_CHECK([if the linker ($LD) is GNU ld], ac_cv_prog_gnu_ld,
[# I'd rather use --version here, but apparently some GNU ld's only accept -v.
if $LD -v 2>&1 </dev/null | egrep '(GNU|with BFD)' 1>&5; then
  ac_cv_prog_gnu_ld=yes
else
  ac_cv_prog_gnu_ld=no
fi])
])

# AC_PROG_NM - find the path to a BSD-compatible name lister
AC_DEFUN(AC_PROG_NM,
[AC_MSG_CHECKING([for BSD-compatible nm])
AC_CACHE_VAL(ac_cv_path_NM,
[if test -n "$NM"; then
  # Let the user override the test.
  ac_cv_path_NM="$NM"
else
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}${PATH_SEPARATOR-:}"
  for ac_dir in $PATH /usr/ccs/bin /usr/ucb /bin; do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/nm || test -f $ac_dir/nm$ac_exeext ; then
      # Check to see if the nm accepts a BSD-compat flag.
      # Adding the `sed 1q' prevents false positives on HP-UX, which says:
      #   nm: unknown option "B" ignored
      if ($ac_dir/nm -B /dev/null 2>&1 | sed '1q'; exit 0) | egrep /dev/null >/dev/null; then
	ac_cv_path_NM="$ac_dir/nm -B"
	break
      elif ($ac_dir/nm -p /dev/null 2>&1 | sed '1q'; exit 0) | egrep /dev/null >/dev/null; then
	ac_cv_path_NM="$ac_dir/nm -p"
	break
      else
	ac_cv_path_NM=${ac_cv_path_NM="$ac_dir/nm"} # keep the first match, but
	continue # so that we can try to find one that supports BSD flags
      fi
    fi
  done
  IFS="$ac_save_ifs"
  test -z "$ac_cv_path_NM" && ac_cv_path_NM=nm
fi])
NM="$ac_cv_path_NM"
AC_MSG_RESULT([$NM])
AC_SUBST(NM)
])

# AC_CHECK_LIBM - check for math library
AC_DEFUN(AC_CHECK_LIBM,
[AC_REQUIRE([AC_CANONICAL_HOST])dnl
LIBM=
case "$host" in
*-*-beos* | *-*-cygwin*)
  # These system don't have libm
  ;;
*-ncr-sysv4.3*)
  AC_CHECK_LIB(mw, _mwvalidcheckl, LIBM="-lmw")
  AC_CHECK_LIB(m, main, LIBM="$LIBM -lm")
  ;;
*)
  AC_CHECK_LIB(m, main, LIBM="-lm")
  ;;
esac
])

# AC_LIBLTDL_CONVENIENCE[(dir)] - sets LIBLTDL to the link flags for
# the libltdl convenience library, adds --enable-ltdl-convenience to
# the configure arguments.  Note that LIBLTDL is not AC_SUBSTed, nor
# is AC_CONFIG_SUBDIRS called.  If DIR is not provided, it is assumed
# to be `${top_builddir}/libltdl'.  Make sure you start DIR with
# '${top_builddir}/' (note the single quotes!) if your package is not
# flat, and, if you're not using automake, define top_builddir as
# appropriate in the Makefiles.
AC_DEFUN(AC_LIBLTDL_CONVENIENCE, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
  case "$enable_ltdl_convenience" in
  no) AC_MSG_ERROR([this package needs a convenience libltdl]) ;;
  "") enable_ltdl_convenience=yes
      ac_configure_args="$ac_configure_args --enable-ltdl-convenience" ;;
  esac
  LIBLTDL=ifelse($#,1,$1,['${top_builddir}/libltdl'])/libltdlc.la
  INCLTDL=ifelse($#,1,-I$1,['-I${top_builddir}/libltdl'])
])

# AC_LIBLTDL_INSTALLABLE[(dir)] - sets LIBLTDL to the link flags for
# the libltdl installable library, and adds --enable-ltdl-install to
# the configure arguments.  Note that LIBLTDL is not AC_SUBSTed, nor
# is AC_CONFIG_SUBDIRS called.  If DIR is not provided, it is assumed
# to be `${top_builddir}/libltdl'.  Make sure you start DIR with
# '${top_builddir}/' (note the single quotes!) if your package is not
# flat, and, if you're not using automake, define top_builddir as
# appropriate in the Makefiles.
# In the future, this macro may have to be called after AC_PROG_LIBTOOL.
AC_DEFUN(AC_LIBLTDL_INSTALLABLE, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
  AC_CHECK_LIB(ltdl, main,
  [test x"$enable_ltdl_install" != xyes && enable_ltdl_install=no],
  [if test x"$enable_ltdl_install" = xno; then
     AC_MSG_WARN([libltdl not installed, but installation disabled])
   else
     enable_ltdl_install=yes
   fi
  ])
  if test x"$enable_ltdl_install" = x"yes"; then
    ac_configure_args="$ac_configure_args --enable-ltdl-install"
    LIBLTDL=ifelse($#,1,$1,['${top_builddir}/libltdl'])/libltdl.la
    INCLTDL=ifelse($#,1,-I$1,['-I${top_builddir}/libltdl'])
  else
    ac_configure_args="$ac_configure_args --enable-ltdl-install=no"
    LIBLTDL="-lltdl"
    INCLTDL=
  fi
])

dnl old names
AC_DEFUN(AM_PROG_LIBTOOL, [indir([AC_PROG_LIBTOOL])])dnl
AC_DEFUN(AM_ENABLE_SHARED, [indir([AC_ENABLE_SHARED], $@)])dnl
AC_DEFUN(AM_ENABLE_STATIC, [indir([AC_ENABLE_STATIC], $@)])dnl
AC_DEFUN(AM_DISABLE_SHARED, [indir([AC_DISABLE_SHARED], $@)])dnl
AC_DEFUN(AM_DISABLE_STATIC, [indir([AC_DISABLE_STATIC], $@)])dnl
AC_DEFUN(AM_PROG_LD, [indir([AC_PROG_LD])])dnl
AC_DEFUN(AM_PROG_NM, [indir([AC_PROG_NM])])dnl

dnl This is just to silence aclocal about the macro not being used
ifelse([AC_DISABLE_FAST_INSTALL])dnl


# serial 1

dnl finds information needed for compilation of shared library style python
dnl extensions.  AM_PATH_PYTHON should be called before hand.
AC_DEFUN(AM_INIT_PYEXEC_MOD,
  [AC_REQUIRE([AM_PATH_PYTHON])
  AC_MSG_CHECKING([for python headers])
  AC_CACHE_VAL(am_cv_python_includes,
    [changequote(,)dnl
    am_cv_python_includes="`$PYTHON -c '
import sys
includepy = \"%s/include/python%s\" % (sys.prefix, sys.version[:3])
if sys.version[0] > \"1\" or sys.version[2] > \"4\":
  libpl = \"%s/include/python%s\" % (sys.exec_prefix, sys.version[:3])
else:
  libpl = \"%s/lib/python%s/config\" % (sys.exec_prefix, sys.version[:3])
print \"-I%s -I%s\" % (includepy, libpl)'`"
    changequote([, ])])
  PYTHON_INCLUDES="$am_cv_python_includes"
  AC_MSG_RESULT(found)
  AC_SUBST(PYTHON_INCLUDES)

  AC_MSG_CHECKING([definitions from Python makefile])
  AC_CACHE_VAL(am_cv_python_makefile,
    [changequote(,)dnl
    py_makefile="`$PYTHON -c '
import sys
print \"%s/lib/python%s/config/Makefile\"%(sys.exec_prefix, sys.version[:3])'`"
    if test ! -f "$py_makefile"; then
      AC_MSG_ERROR([*** Couldn't find the python config makefile.  Maybe you are
*** missing the development portion of the python installation])
    fi
    eval `sed -n \
-e "s/^CC=[ 	]*\(.*\)/am_cv_python_CC='\1'/p" \
-e "s/^OPT=[ 	]*\(.*\)/am_cv_python_OPT='\1'/p" \
-e "s/^CCSHARED=[ 	]*\(.*\)/am_cv_python_CCSHARED='\1'/p" \
-e "s/^LDSHARED=[ 	]*\(.*\)/am_cv_python_LDSHARED='\1'/p" \
-e "s/^SO=[ 	]*\(.*\)/am_cv_python_SO='\1'/p" \
    $py_makefile`
    am_cv_python_makefile=found
    changequote([, ])])
  AC_MSG_RESULT(done)
  CC="$am_cv_python_CC"
  OPT="$am_cv_python_OPT"
  SO="$am_cv_python_SO"
  PYTHON_CFLAGS="$am_cv_python_CCSHARED \$(OPT)"
  PYTHON_LINK="$am_cv_python_LDSHARED -o \[$]@"

  AC_SUBST(CC)dnl
  AC_SUBST(OPT)dnl
  AC_SUBST(SO)dnl
  AC_SUBST(PYTHON_CFLAGS)dnl
  AC_SUBST(PYTHON_LINK)])



# serial 1

AC_DEFUN(AM_PATH_PYTHON,
  [AC_CHECK_PROGS(PYTHON, python python1.5 python1.4 python1.3,no)
  if test "$PYTHON" != no; then
    AC_MSG_CHECKING([where .py files should go])
changequote(, )dnl
    pythondir=`$PYTHON -c '
import sys
if sys.version[0] > "1" or sys.version[2] > "4":
  print "%s/lib/python%s/site-packages" % (sys.prefix, sys.version[:3])
else:
  print "%s/lib/python%s" % (sys.prefix, sys.version[:3])'`
changequote([, ])dnl
    AC_MSG_RESULT($pythondir)
    AC_MSG_CHECKING([where python extensions should go])
changequote(, )dnl
    pyexecdir=`$PYTHON -c '
import sys
if sys.version[0] > "1" or sys.version[2] > "4":
  print "%s/lib/python%s/site-packages" % (sys.exec_prefix, sys.version[:3])
else:
  print "%s/lib/python%s/sharedmodules" % (sys.exec_prefix, sys.version[:3])'`
changequote([, ])dnl
    AC_MSG_RESULT($pyexecdir)
  else
    # these defaults are version independent ...
    AC_MSG_CHECKING([where .py files should go])
    pythondir='$(prefix)/lib/site-python'
    AC_MSG_RESULT($pythondir)
    AC_MSG_CHECKING([where python extensions should go])
    pyexecdir='$(exec_prefix)/lib/site-python'
    AC_MSG_RESULT($pyexecdir)
  fi
  AC_SUBST(pythondir)
  AC_SUBST(pyexecdir)])


dnl AM_CHECK_PYMOD(MODNAME [,SYMBOL [,ACTION-IF-FOUND [,ACTION-IF-NOT-FOUND]]])
dnl Check if a module containing a given symbol is visible to python.
AC_DEFUN(AM_CHECK_PYMOD,
[AC_REQUIRE([AM_PATH_PYTHON])
py_mod_var=`echo $1['_']$2 | sed 'y%./+-%__p_%'`
AC_MSG_CHECKING(for ifelse([$2],[],,[$2 in ])python module $1)
AC_CACHE_VAL(py_cv_mod_$py_mod_var, [
ifelse([$2],[], [prog="
import sys
try:
	import $1
except ImportError:
	sys.exit(1)
except:
	sys.exit(0)
sys.exit(0)"], [prog="
import $1
$1.$2"])
if $PYTHON -c "$prog" 1>&AC_FD_CC 2>&AC_FD_CC
  then
    eval "py_cv_mod_$py_mod_var=yes"
  else
    eval "py_cv_mod_$py_mod_var=no"
  fi
])
py_val=`eval "echo \`echo '$py_cv_mod_'$py_mod_var\`"`
if test "x$py_val" != xno; then
  AC_MSG_RESULT(yes)
  ifelse([$3], [],, [$3
])dnl
else
  AC_MSG_RESULT(no)
  ifelse([$4], [],, [$4
])dnl
fi
])



# Configure paths for GLIB
# Owen Taylor     97-11-3

dnl AM_PATH_GLIB([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl Test for GLIB, and define GLIB_CFLAGS and GLIB_LIBS, if "gmodule" or 
dnl gthread is specified in MODULES, pass to glib-config
dnl
AC_DEFUN(AM_PATH_GLIB,
[dnl 
dnl Get the cflags and libraries from the glib-config script
dnl
AC_ARG_WITH(glib-prefix,[  --with-glib-prefix=PFX   Prefix where GLIB is installed (optional)],
            glib_config_prefix="$withval", glib_config_prefix="")
AC_ARG_WITH(glib-exec-prefix,[  --with-glib-exec-prefix=PFX Exec prefix where GLIB is installed (optional)],
            glib_config_exec_prefix="$withval", glib_config_exec_prefix="")
AC_ARG_ENABLE(glibtest, [  --disable-glibtest       Do not try to compile and run a test GLIB program],
		    , enable_glibtest=yes)

  if test x$glib_config_exec_prefix != x ; then
     glib_config_args="$glib_config_args --exec-prefix=$glib_config_exec_prefix"
     if test x${GLIB_CONFIG+set} != xset ; then
        GLIB_CONFIG=$glib_config_exec_prefix/bin/glib-config
     fi
  fi
  if test x$glib_config_prefix != x ; then
     glib_config_args="$glib_config_args --prefix=$glib_config_prefix"
     if test x${GLIB_CONFIG+set} != xset ; then
        GLIB_CONFIG=$glib_config_prefix/bin/glib-config
     fi
  fi

  for module in . $4
  do
      case "$module" in
         gmodule) 
             glib_config_args="$glib_config_args gmodule"
         ;;
         gthread) 
             glib_config_args="$glib_config_args gthread"
         ;;
      esac
  done

  AC_PATH_PROG(GLIB_CONFIG, glib-config, no)
  min_glib_version=ifelse([$1], ,0.99.7,$1)
  AC_MSG_CHECKING(for GLIB - version >= $min_glib_version)
  no_glib=""
  if test "$GLIB_CONFIG" = "no" ; then
    no_glib=yes
  else
    GLIB_CFLAGS=`$GLIB_CONFIG $glib_config_args --cflags`
    GLIB_LIBS=`$GLIB_CONFIG $glib_config_args --libs`
    glib_config_major_version=`$GLIB_CONFIG $glib_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    glib_config_minor_version=`$GLIB_CONFIG $glib_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    glib_config_micro_version=`$GLIB_CONFIG $glib_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_glibtest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $GLIB_CFLAGS"
      LIBS="$GLIB_LIBS $LIBS"
dnl
dnl Now check if the installed GLIB is sufficiently new. (Also sanity
dnl checks the results of glib-config to some extent
dnl
      rm -f conf.glibtest
      AC_TRY_RUN([
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

int 
main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.glibtest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = g_strdup("$min_glib_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_glib_version");
     exit(1);
   }

  if ((glib_major_version != $glib_config_major_version) ||
      (glib_minor_version != $glib_config_minor_version) ||
      (glib_micro_version != $glib_config_micro_version))
    {
      printf("\n*** 'glib-config --version' returned %d.%d.%d, but GLIB (%d.%d.%d)\n", 
             $glib_config_major_version, $glib_config_minor_version, $glib_config_micro_version,
             glib_major_version, glib_minor_version, glib_micro_version);
      printf ("*** was found! If glib-config was correct, then it is best\n");
      printf ("*** to remove the old version of GLIB. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If glib-config was wrong, set the environment variable GLIB_CONFIG\n");
      printf("*** to point to the correct copy of glib-config, and remove the file config.cache\n");
      printf("*** before re-running configure\n");
    } 
  else if ((glib_major_version != GLIB_MAJOR_VERSION) ||
	   (glib_minor_version != GLIB_MINOR_VERSION) ||
           (glib_micro_version != GLIB_MICRO_VERSION))
    {
      printf("*** GLIB header files (version %d.%d.%d) do not match\n",
	     GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION, GLIB_MICRO_VERSION);
      printf("*** library (version %d.%d.%d)\n",
	     glib_major_version, glib_minor_version, glib_micro_version);
    }
  else
    {
      if ((glib_major_version > major) ||
        ((glib_major_version == major) && (glib_minor_version > minor)) ||
        ((glib_major_version == major) && (glib_minor_version == minor) && (glib_micro_version >= micro)))
      {
        return 0;
       }
     else
      {
        printf("\n*** An old version of GLIB (%d.%d.%d) was found.\n",
               glib_major_version, glib_minor_version, glib_micro_version);
        printf("*** You need a version of GLIB newer than %d.%d.%d. The latest version of\n",
	       major, minor, micro);
        printf("*** GLIB is always available from ftp://ftp.gtk.org.\n");
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the glib-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of GLIB, but you can also set the GLIB_CONFIG environment to point to the\n");
        printf("*** correct copy of glib-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
      }
    }
  return 1;
}
],, no_glib=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_glib" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$GLIB_CONFIG" = "no" ; then
       echo "*** The glib-config script installed by GLIB could not be found"
       echo "*** If GLIB was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the GLIB_CONFIG environment variable to the"
       echo "*** full path to glib-config."
     else
       if test -f conf.glibtest ; then
        :
       else
          echo "*** Could not run GLIB test program, checking why..."
          CFLAGS="$CFLAGS $GLIB_CFLAGS"
          LIBS="$LIBS $GLIB_LIBS"
          AC_TRY_LINK([
#include <glib.h>
#include <stdio.h>
],      [ return ((glib_major_version) || (glib_minor_version) || (glib_micro_version)); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding GLIB or finding the wrong"
          echo "*** version of GLIB. If it is not finding GLIB, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"
          echo "***"
          echo "*** If you have a RedHat 5.0 system, you should remove the GTK package that"
          echo "*** came with the system with the command"
          echo "***"
          echo "***    rpm --erase --nodeps gtk gtk-devel" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means GLIB was incorrectly installed"
          echo "*** or that you have moved GLIB since it was installed. In the latter case, you"
          echo "*** may want to edit the glib-config script: $GLIB_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     GLIB_CFLAGS=""
     GLIB_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(GLIB_CFLAGS)
  AC_SUBST(GLIB_LIBS)
  rm -f conf.glibtest
])

dnl AM_PATH_ORBIT([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl Test for ORBit, and define ORBIT_CFLAGS and ORBIT_LIBS
dnl
AC_DEFUN(AM_PATH_ORBIT,
[dnl 
dnl Get the cflags and libraries from the orbit-config script
dnl
AC_ARG_WITH(orbit-prefix,[  --with-orbit-prefix=PFX   Prefix where ORBIT is installed (optional)],
            orbit_config_prefix="$withval", orbit_config_prefix="")
AC_ARG_WITH(orbit-exec-prefix,[  --with-orbit-exec-prefix=PFX Exec prefix where ORBIT is installed (optional)],
            orbit_config_exec_prefix="$withval", orbit_config_exec_prefix="")
AC_ARG_ENABLE(orbittest, [  --disable-orbittest       Do not try to compile and run a test ORBIT program],
		    , enable_orbittest=yes)

  for module in . $4
  do
      case "$module" in
         server) 
             orbit_config_args="$orbit_config_args server"
	 ;;
         client) 
             orbit_config_args="$orbit_config_args client"
         ;;
      esac
  done
  if test x$orbit_config_args = x ; then
	orbit_config_args=server
  fi

  if test x$orbit_config_exec_prefix != x ; then
     orbit_config_args="$orbit_config_args --exec-prefix=$orbit_config_exec_prefix"
     if test x${ORBIT_CONFIG+set} != xset ; then
        ORBIT_CONFIG=$orbit_config_exec_prefix/bin/orbit-config
     fi
  fi
  if test x$orbit_config_prefix != x ; then
     orbit_config_args="$orbit_config_args --prefix=$orbit_config_prefix"
     if test x${ORBIT_CONFIG+set} != xset ; then
        ORBIT_CONFIG=$orbit_config_prefix/bin/orbit-config
     fi
  fi

  AC_PATH_PROG(ORBIT_CONFIG, orbit-config, no)
  min_orbit_version=ifelse([$1], , 0.5.1, $1)
  AC_MSG_CHECKING(for ORBit - version >= $min_orbit_version)
  no_orbit=""
  if test "$ORBIT_CONFIG" = "no" ; then
    no_orbit=yes
  else
    ORBIT_CFLAGS=`$ORBIT_CONFIG $orbit_config_args --cflags`
    ORBIT_LIBS=`$ORBIT_CONFIG $orbit_config_args --libs`
    orbit_config_major_version=`$ORBIT_CONFIG $orbit_config_args --version | \
	   sed -e 's,[[^0-9.]],,g' -e 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    orbit_config_minor_version=`$ORBIT_CONFIG $orbit_config_args --version | \
	   sed -e 's,[[^0-9.]],,g' -e 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    orbit_config_micro_version=`$ORBIT_CONFIG $orbit_config_args --version | \
	   sed -e 's,[[^0-9.]],,g' -e 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_orbittest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $ORBIT_CFLAGS"
      LIBS="$ORBIT_LIBS $LIBS"
dnl
dnl Now check if the installed ORBIT is sufficiently new. (Also sanity
dnl checks the results of orbit-config to some extent
dnl
      rm -f conf.orbittest
      AC_TRY_RUN([
#include <orb/orbit.h>
#include <stdio.h>
#include <stdlib.h>

int 
main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.orbittest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = g_strdup("$min_orbit_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_orbit_version");
     exit(1);
   }

  if ((orbit_major_version != $orbit_config_major_version) ||
      (orbit_minor_version != $orbit_config_minor_version) ||
      (orbit_micro_version != $orbit_config_micro_version))
    {
      printf("\n*** 'orbit-config --version' returned %d.%d.%d, but ORBit (%d.%d.%d)\n", 
             $orbit_config_major_version, $orbit_config_minor_version, $orbit_config_micro_version,
             orbit_major_version, orbit_minor_version, orbit_micro_version);
      printf ("*** was found! If orbit-config was correct, then it is best\n");
      printf ("*** to remove the old version of ORBit. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If orbit-config was wrong, set the environment variable ORBIT_CONFIG\n");
      printf("*** to point to the correct copy of orbit-config, and remove the file config.cache\n");
      printf("*** before re-running configure\n");
    } 
#if defined (ORBIT_MAJOR_VERSION) && defined (ORBIT_MINOR_VERSION) && defined (ORBIT_MICRO_VERSION)
  else if ((orbit_major_version != ORBIT_MAJOR_VERSION) ||
	   (orbit_minor_version != ORBIT_MINOR_VERSION) ||
           (orbit_micro_version != ORBIT_MICRO_VERSION))
    {
      printf("*** ORBit header files (version %d.%d.%d) do not match\n",
	     ORBIT_MAJOR_VERSION, ORBIT_MINOR_VERSION, ORBIT_MICRO_VERSION);
      printf("*** library (version %d.%d.%d)\n",
	     orbit_major_version, orbit_minor_version, orbit_micro_version);
    }
#endif /* defined (ORBIT_MAJOR_VERSION) ... */
  else
    {
      if ((orbit_major_version > major) ||
        ((orbit_major_version == major) && (orbit_minor_version > minor)) ||
        ((orbit_major_version == major) && (orbit_minor_version == minor) && (orbit_micro_version >= micro)))
      {
        return 0;
       }
     else
      {
        printf("\n*** An old version of ORBit (%d.%d.%d) was found.\n",
               orbit_major_version, orbit_minor_version, orbit_micro_version);
        printf("*** You need a version of ORBit newer than %d.%d.%d. The latest version of\n",
	       major, minor, micro);
        printf("*** ORBit is always available from ftp://ftp.orbit.org.\n");
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the orbit-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of ORBit, but you can also set the ORBIT_CONFIG environment to point to the\n");
        printf("*** correct copy of orbit-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
      }
    }
  return 1;
}
],, no_orbit=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_orbit" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$ORBIT_CONFIG" = "no" ; then
       echo "*** The orbit-config script installed by ORBIT could not be found"
       echo "*** If ORBit was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the ORBIT_CONFIG environment variable to the"
       echo "*** full path to orbit-config."
     else
       if test -f conf.orbittest ; then
        :
       else
          echo "*** Could not run ORBIT test program, checking why..."
          CFLAGS="$CFLAGS $ORBIT_CFLAGS"
          LIBS="$LIBS $ORBIT_LIBS"
          AC_TRY_LINK([
#include <orb/orbit.h>
#include <stdio.h>
],      [ return ((orbit_major_version) || (orbit_minor_version) || (orbit_micro_version)); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding ORBit or finding the wrong"
          echo "*** version of ORBit. If it is not finding ORBit, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"
          echo "***"
          echo "*** If you have a RedHat 5.0 system, you should remove the ORBit package that"
          echo "*** came with the system with the command"
          echo "***"
          echo "***    rpm --erase --nodeps orbit orbit-devel" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means ORBIT was incorrectly installed"
          echo "*** or that you have moved ORBit since it was installed. In the latter case, you"
          echo "*** may want to edit the orbit-config script: $ORBIT_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     ORBIT_CFLAGS=""
     ORBIT_LIBS=""
     ifelse([$3], , :, [$3])
  fi

  AC_PATH_PROG(ORBIT_IDL, orbit-idl, ifelse([$3], , :, [$3]))
  AC_SUBST(ORBIT_CFLAGS)
  AC_SUBST(ORBIT_LIBS)
  AC_SUBST(ORBIT_IDL)
  rm -f conf.orbittest
])

# Configure paths for LIBIDL

dnl AM_PATH_LIBIDL([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl Test for LIBIDL, and define LIBIDL_CFLAGS and LIBIDL_LIBS
dnl
AC_DEFUN(AM_PATH_LIBIDL,
[dnl 
dnl Get the cflags and libraries from the libIDL-config script
dnl
AC_ARG_WITH(libIDL-prefix,[  --with-libIDL-prefix=PFX   Prefix where libIDL is installed (optional)],
            libIDL_config_prefix="$withval", libIDL_config_prefix="")
AC_ARG_WITH(libIDL-exec-prefix,[  --with-libIDL-exec-prefix=PFX Exec prefix where libIDL is installed (optional)],
            libIDL_config_exec_prefix="$withval", libIDL_config_exec_prefix="")
AC_ARG_ENABLE(libIDLtest, [  --disable-libIDLtest       Do not try to compile and run a test libIDL program],
		    , enable_libIDLtest=yes)

  if test x$libIDL_config_exec_prefix != x ; then
     libIDL_config_args="$libIDL_config_args --exec-prefix=$libIDL_config_exec_prefix"
     if test x${LIBIDL_CONFIG+set} != xset ; then
        LIBIDL_CONFIG=$libIDL_config_exec_prefix/bin/libIDL-config
     fi
  fi
  if test x$libIDL_config_prefix != x ; then
     libIDL_config_args="$libIDL_config_args --prefix=$libIDL_config_prefix"
     if test x${LIBIDL_CONFIG+set} != xset ; then
        LIBIDL_CONFIG=$libIDL_config_prefix/bin/libIDL-config
     fi
  fi

  AM_PATH_GLIB(1.2.0)

  AC_PATH_PROG(LIBIDL_CONFIG, libIDL-config, no)
  min_libIDL_version=ifelse([$1], ,0.6.0,$1)
  AC_MSG_CHECKING(for libIDL - version >= $min_libIDL_version)
  no_libIDL=""
  if test "$LIBIDL_CONFIG" = "no" ; then
    no_libIDL=yes
  else
    LIBIDL_CFLAGS=`$LIBIDL_CONFIG $libIDL_config_args --cflags`
    LIBIDL_LIBS=`$LIBIDL_CONFIG $libIDL_config_args --libs`
    libIDL_config_major_version=`$LIBIDL_CONFIG $libIDL_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    libIDL_config_minor_version=`$LIBIDL_CONFIG $libIDL_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    libIDL_config_micro_version=`$LIBIDL_CONFIG $libIDL_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_libIDLtest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $LIBIDL_CFLAGS"
      LIBS="$LIBIDL_LIBS $LIBS"
dnl
dnl Now check if the installed LIBIDL is sufficiently new.
dnl
      rm -f conf.libIDLtest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <libIDL/IDL.h>

int 
main ()
{
  int major, minor, micro;
  int libIDL_major_version;
  int libIDL_minor_version;
  int libIDL_micro_version;
  char *tmp_version;

  system ("touch conf.libIDLtest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = g_strdup ("$min_libIDL_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_libIDL_version");
     exit(1);
  }
  tmp_version = g_strdup (IDL_get_libver_string ());
  if (sscanf(tmp_version, "%d.%d.%d",
	     &libIDL_major_version,
	     &libIDL_minor_version,
	     &libIDL_micro_version) != 3) {
     printf("%s, bad version string\n", tmp_version);
     exit(1);
  }

  if ((libIDL_major_version != $libIDL_config_major_version) ||
      (libIDL_minor_version != $libIDL_config_minor_version) ||
      (libIDL_micro_version != $libIDL_config_micro_version))
    {
      printf("\n*** 'libIDL-config --version' returned %d.%d.%d, but libIDL (%d.%d.%d)\n", 
             $libIDL_config_major_version, $libIDL_config_minor_version, $libIDL_config_micro_version,
             libIDL_major_version, libIDL_minor_version, libIDL_micro_version);
      printf ("*** was found! If libIDL-config was correct, then it is best\n");
      printf ("*** to remove the old version of LIBIDL. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If libIDL-config was wrong, set the environment variable LIBIDL_CONFIG\n");
      printf("*** to point to the correct copy of libIDL-config, and remove the file config.cache\n");
      printf("*** before re-running configure\n");
    } 
  else if ((libIDL_major_version != LIBIDL_MAJOR_VERSION) ||
	   (libIDL_minor_version != LIBIDL_MINOR_VERSION) ||
           (libIDL_micro_version != LIBIDL_MICRO_VERSION))
    {
      printf("\n*** libIDL header files (version %d.%d.%d) do not match\n",
	     LIBIDL_MAJOR_VERSION, LIBIDL_MINOR_VERSION, LIBIDL_MICRO_VERSION);
      printf("*** library (version %d.%d.%d)\n",
	     libIDL_major_version, libIDL_minor_version, libIDL_micro_version);
    }
  else
    {
      if ((libIDL_major_version > major) ||
        ((libIDL_major_version == major) && (libIDL_minor_version > minor)) ||
        ((libIDL_major_version == major) && (libIDL_minor_version == minor) && (libIDL_micro_version >= micro)))
      {
        return 0;
       }
     else
      {
        printf("\n*** An old version of libIDL (%d.%d.%d) was found.\n",
               libIDL_major_version, libIDL_minor_version, libIDL_micro_version);
        printf("*** You need at least libIDL version %d.%d.%d.\n",
               major, minor, micro);
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the libIDL-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of libIDL, but you can also set the LIBIDL_CONFIG environment to point to the\n");
        printf("*** correct copy of libIDL-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
      }
    }
  return 1;
}
],, no_libIDL=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_libIDL" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$LIBIDL_CONFIG" = "no" ; then
       echo "*** The libIDL-config script installed by libIDL could not be found"
       echo "*** If libIDL was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the LIBIDL_CONFIG environment variable to the"
       echo "*** full path to libIDL-config."
     else
       if test -f conf.libIDLtest ; then
        :
       else
          echo "*** Could not run libIDL test program, checking why..."
          CFLAGS="$CFLAGS $LIBIDL_CFLAGS"
          LIBS="$LIBS $LIBIDL_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
#include <stdlib.h>
#include <libIDL/IDL.h>
],      [ return IDL_get_libver_string ? 1 : 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding libIDL or finding the wrong"
          echo "*** version of LIBIDL. If it is not finding libIDL, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means libIDL was incorrectly installed"
          echo "*** or that you have moved libIDL since it was installed. In the latter case, you"
          echo "*** may want to edit the libIDL-config script: $LIBIDL_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     LIBIDL_CFLAGS=""
     LIBIDL_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(LIBIDL_CFLAGS)
  AC_SUBST(LIBIDL_LIBS)
  rm -f conf.libIDLtest
])

