# $Id: configure.ac 253 2004-08-09 20:08:52Z sam $

AC_INIT(src/main.c)

AC_PREREQ(2.50)
AC_CONFIG_AUX_DIR(autotools)
AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE(pwntcha, 0.0)
AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_STDC_HEADERS

AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

AC_CHECK_FUNCS(getopt_long)

# Use OpenCV?
ac_cv_my_have_opencv="no"
save_CPPFLAGS="${CPPFLAGS}"
AC_PATH_PROG(OPENCV_CONFIG, opencv-config, no)
if test "${OPENCV_CONFIG}" != "no"; then
  CPPFLAGS="${CPPFLAGS} `opencv-config --cflags`"
fi
AC_CHECK_HEADERS(cv.h,
 [ac_cv_my_have_opencv="yes"],
 [ac_cv_my_have_opencv="no"])
CPPFLAGS="${save_CPPFLAGS}"
AM_CONDITIONAL(USE_OPENCV, test "${ac_cv_my_have_opencv}" = "yes")

# Use Imlib2?
ac_cv_my_have_imlib2="no"
save_CPPFLAGS="${CPPFLAGS}"
AC_PATH_PROG(IMLIB2_CONFIG, imlib2-config, no)
if test "${IMLIB2_CONFIG}" != "no"; then
  CPPFLAGS="${CPPFLAGS} `imlib2-config --cflags` -DX_DISPLAY_MISSING=1"
fi
AC_CHECK_HEADERS(Imlib2.h,
 [ac_cv_my_have_imlib2="yes"],
 [ac_cv_my_have_imlib2="no"])
CPPFLAGS="${save_CPPFLAGS}"
AM_CONDITIONAL(USE_IMLIB2, test "${ac_cv_my_have_imlib2}" = "yes")

if test "${ac_cv_my_have_imlib2}" = "no" -a "${ac_cv_my_have_opencv}" = "no"; then
  AC_MSG_ERROR([[cannot find Imlib2 or OpenCV, please install one of them]])
fi

AC_OUTPUT([
  Makefile
  src/Makefile
])

