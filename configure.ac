AC_INIT([libopusfile], [0.0])

AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE

AM_INIT_AUTOMAKE([1.11 foreign])
LT_INIT

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_MACRO_DIR([m4])

dnl Library versioning for libtool.
dnl Please update these for releases.
dnl CURRENT, REVISION, AGE
dnl - library source changed -> increment REVISION
dnl - interfaces added/removed/changed -> increment CURRENT, REVISION = 0
dnl - interfaces added -> increment AGE
dnl - interfaces removed -> AGE = 0

OP_LT_CURRENT=0
OP_LT_REVISION=0
OP_LT_AGE=0

AC_SUBST(OP_LT_CURRENT)
AC_SUBST(OP_LT_REVISION)
AC_SUBST(OP_LT_AGE)

AC_ARG_ENABLE([assertions],
  AS_HELP_STRING([--enable-assertions], [Enable assertions in code]),,
  enable_assertions=no)

AS_IF([test "x$enable_assertions" = "xyes"], [
  AC_DEFINE([OP_ENABLE_ASSERTIONS], [1], [Enable assertions in code])
])

AC_ARG_ENABLE([http],
  AS_HELP_STRING([--disable-http], [Disable HTTP support]),,
  enable_http=yes)

AS_IF([test "x$enable_http" != "xno"],
  [openssl="openssl"
   AC_DEFINE([OP_ENABLE_HTTP], [1], [Enable HTTP support])
])
AC_SUBST(openssl)

PKG_CHECK_MODULES([DEPS], [ogg >= 1.3 opus >= 1.0.1 ]${openssl})

AC_ARG_ENABLE([fixed-point],
  AS_HELP_STRING([--enable-fixed-point], [Enable fixed-point calculation]),,
  enable_fixed_point=no)
AC_ARG_ENABLE([float],
  AS_HELP_STRING([--disable-float], [Disable floating-point API]),,
  enable_float=yes)

AS_IF([test "x$enable_float" = "xno"],
  [enable_fixed_point=yes
   AC_DEFINE([OP_DISABLE_FLOAT_API], [1], [Disable floating-point API])
  ]
)

AS_IF([test "x$enable_fixed_point" = "xyes"],
  [AC_DEFINE([OP_FIXED_POINT], [1], [Enable fixed-point calculation])],
  [dnl This only has to be tested for if float->fixed conversions are required
   AC_SEARCH_LIBS([lrintf], [m], [
      AC_DEFINE([OP_HAVE_LRINTF], [1], [Enable use of lrintf function])
      lrintf_notice="
  Library for lrintf() ....... : ${ac_cv_search_lrintf}"
   ])
  ]
)
AC_SUBST(ac_cv_search_lrintf)

CC_ATTRIBUTE_VISIBILITY([default], [
  CC_FLAG_VISIBILITY([CFLAGS="${CFLAGS} -fvisibility=hidden"])
])

CC_CHECK_CFLAGS_APPEND([-std=c89 -pedantic -Wall -Wextra -Wno-parentheses -Wno-long-long])

dnl Check for doxygen
AC_ARG_ENABLE([doc],
  AS_HELP_STRING([--disable-doc], [Do not build API documentation]),,
  [enable_doc=yes]
)

if test "x$enable_doc" != "xno"; then
  AC_CHECK_PROG(HAVE_DOXYGEN, doxygen, true, false)
  if test "x$HAVE_DOXYGEN" = "xfalse" -a "x$enable_doc" = "xyes" ; then
    AC_MSG_ERROR([*** Doxygen not found. Cannot build API documentation. ***])
  fi
else
  HAVE_DOXYGEN=false
fi
AM_CONDITIONAL(HAVE_DOXYGEN,$HAVE_DOXYGEN)

AC_OUTPUT([
  Makefile
  opusfile.pc
  opusfile-uninstalled.pc
  doc/Doxyfile
])

AC_MSG_NOTICE([
Summary:

Assertions ................... : ${enable_assertions}
HTTP support ................. : ${enable_http}
Fixed-point .................. : ${enable_fixed_point}
Floating-point API ........... : ${enable_float}${lrintf_notice}
Hidden visibility ............ : ${cc_cv_flag_visibility}
API documentation ............ : ${enable_doc}
])
