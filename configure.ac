AC_PREREQ(2.59)
AC_REVISION($Id$)
#
#  mysql2odbc - A MySQL to ODBC bridge library
#  
#  Copyright (C) 2003-2013 OpenLink Software <iodbc@openlinksw.com>
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Library General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Library General Public License for more details.
#
#  You should have received a copy of the GNU Library General Public
#  License along with this library; if not, write to the Free
#  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#


##########################################################################
##									##
##  Initialization							##
##									##
##########################################################################
AC_INIT([MySQL to ODBC Bridge],[1.0],[iodbc@openlinksw.com],[mysql2odbc])

AC_CONFIG_SRCDIR([libfakesql.c])
AC_CONFIG_AUX_DIR([admin])
AC_CONFIG_MACRO_DIR([admin])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([mysql2odbc], [1.0])
AC_CANONICAL_HOST
AM_MAINTAINER_MODE


##########################################################################
##									##
##  Check for C compiler						##
#									##
##########################################################################
AC_PROG_CC
AC_PROG_CC_C_O

AC_PROG_CPP
AC_LANG([C])

AC_C_CONST


##########################################################################
##									##
##  Check for standard programs						##
##									##
##########################################################################

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_LIBTOOL_DLOPEN
AM_PROG_LIBTOOL


##########################################################################
##									##
##  Check for standard header files					##
##									##
##########################################################################

AC_HEADER_STDC
AC_CHECK_HEADERS([memory.h string.h])


##########################################################################
##									##
##  Check for specific library functions				##
##									##
##########################################################################
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset strchr strdup])

AC_CHECK_LIB([dl], [dlopen])
AC_CHECK_LIB([z], [gzopen])


##########################################################################
##									##
##  Version numbers							##
##									##
##########################################################################

#
#  Put separator between options
#
AC_ARG_WITH([],[
MySQL version:],[])

AC_MSG_CHECKING(for MySQL version)
AC_ARG_WITH(mysql-version,
[  --with-mysql-version=3  build for MySQL 3.x (default)
  --with-mysql-version=4  build for MySQL 4.x],
[
    case "${withval}" in
      3)	myversion=3 ;;
      4)	myversion=4 ;;
      *)	AC_MSG_ERROR(bad value ${withval} for --with-mysql-version) ;;
    esac
],
[
    myversion=3	  # set default to MySQL 3.x mode 
])

if test "x$myversion" = "x3"
then
	mysqllib_version="10:0:0"
	AC_MSG_RESULT(MySQL 3.x compatible mode)
else
	mysqllib_version="12:0:0"
	AC_MSG_RESULT(MySQL 4.x compatible mode)
fi
AC_SUBST(mysqllib_version)




##########################################################################
##									##
##  Checkout ODBC situation						##
##									##
##########################################################################

#
#  Put separator between options
#
AC_ARG_WITH([],[
ODBC configuration:],[])

#
#  Initialize variables
#
found_odbc=no
ODBC_CFLAGS=
ODBC_LIBS=


#
#  Check for iODBC support
#
if test "$found_odbc" = "no"
then
AC_ARG_WITH(iodbc,
[  --with-iodbc(=DIR)      Select iODBC support. 
                              DIR is the iODBC base install directory 
                              (default=/usr/local)],
[
    if test "$withval" = "yes"
    then
	withval=/usr/local
    fi

    AC_PATH_PROG(IODBC_CONFIG, iodbc-config, no, [$PATH:$withval/bin])

    AC_MSG_CHECKING([for iODBC support])
    if test -x "$IODBC_CONFIG"
    then
	found_odbc=yes
	ODBC_CFLAGS=`$IODBC_CONFIG --cflags`
	ODBC_LIBS=`$IODBC_CONFIG --libs`
	AC_MSG_RESULT(yes)
    elif test -f "$withval/lib/libiodbc.a"
    then
	found_odbc=yes
	ODBC_CFLAGS="-I$withval/include"
	ODBC_LIBS="-L$withval/lib -liodbc"
	AC_MSG_RESULT(yes)
    else
	AC_MSG_RESULT(no)
    fi
])
fi


#
#  Check for OpenLink odbcsdk support
#
if test "$found_odbc" = "no"
then
AC_ARG_WITH(odbcsdk,
[  --with-odbcsdk(=DIR)    Select OpenLink odbcsdk support. 
                              DIR is the odbcsdk base install directory 
                              (default=/usr/local)],
[
    if test "$withval" = "yes"
    then
	withval=/usr/local
    fi

    AC_MSG_CHECKING([for OpenLink odbcsdk support])
    if test -f "$withval/lib/libiodbc.a"
    then
	found_odbc=yes
	ODBC_CFLAGS="-I$withval/include"
	ODBC_LIBS="-L$withval/lib -liodbc"
	AC_MSG_RESULT(yes)
    else
	AC_MSG_RESULT(no)
    fi
])
fi


#
#  Check for UnixODBC support
#
if test "$found_odbc" = "no"
then
AC_ARG_WITH(unixodbc,
[  --with-unixodbc(=DIR)   Select UnixODBC support. 
                              DIR is the UnixODBC base install directory 
                              (default=/usr/local)],
[
    if test "$withval" = "yes"
    then
	withval=/usr/local
    fi

    AC_MSG_CHECKING([for UnixODBC support])
    if test -f "$withval/lib/libodbc.la"
    then
	found_odbc=yes
	ODBC_CFLAGS="-I$withval/include"
	ODBC_LIBS="-L$withval/lib -lodbc"
	AC_MSG_RESULT(yes)
    else
	AC_MSG_RESULT(no)
    fi
])
fi


#
#  Check for custom ODBC support
#
if test "$found_odbc" = "no"
then
AC_ARG_WITH(odbc-include,
[  --with-odbc-inc=DIR     Specify custom ODBC include directory.
                              (default=/usr/local/include)],
[
    if test "$withval" = "yes"
    then
	withval=/usr/local/include
    fi

    AC_MSG_CHECKING([for sql.h])
    if test -f "$withval/sql.h"
    then
	ODBC_CFLAGS="-I$withval"
	AC_MSG_RESULT(yes)
    else
	AC_MSG_RESULT(no)
    fi
])
AC_ARG_WITH(odbc-lib,
[  --with-odbc-lib=DIR    Specify custom ODBC lib directory.
                              (default=/usr/local/lib)],
[
    if test "$withval" = "yes"
    then
	    withval=/usr/local/lib
    fi

    ac_save_LDFLAGS=$LDFLAGS
    test -n "$withval" && LDFLAGS="$LDFLAGS -L$withval"
    AC_CHECK_LIB(iodbc, SQLConnect, 
    [
	ODBC_LIBS="-L$withval -liodbc"; found_odbc=yes
    ],
    [
	AC_CHECK_LIB(odbc, SQLConnect,
	[
	    ODBC_LIBS="-L$withval -lodbc"; found_odbc=yes
	])
    ])
    LDFLAGS=$ac_save_LDFLAGS
])
fi


#
#  Maybe the libraries are in some default directory
#
if test "$found_odbc" = "no"
then
    AC_PATH_PROG(IODBC_CONFIG, iodbc-config, no)

    AC_MSG_CHECKING([for iODBC support])
    if test -x "$IODBC_CONFIG"
    then
	found_odbc=yes
	ODBC_CFLAGS=`$IODBC_CONFIG --cflags`
	ODBC_LIBS=`$IODBC_CONFIG --libs`
	AC_MSG_RESULT(yes)
    fi
fi 


#
#  Maybe the libraries are in some default directory
#
if test "$found_odbc" = "no"
then
    AC_CHECK_LIB(iodbc, SQLConnect, 
    [
	ODBC_LIBS="-liodbc"; found_odbc=yes
    ],
    [
	AC_CHECK_LIB(odbc, SQLConnect,
	[
	    ODBC_LIBS="-lodbc"; found_odbc=yes
	])
    ])
fi


#
#  Do not continue unless we found a valid ODBC implementation
#
if test "$found_odbc" = "no"
then
    AC_MSG_ERROR([could not find ODBC header files or libraries])
fi

#
#  Expand our compile flags
#
AC_SUBST(ODBC_CFLAGS)
AC_SUBST(ODBC_LIBS)


########################################################################## 
##									##
##  Generate Makefiles etc.						##
##									##
##########################################################################
AC_CONFIG_FILES([Makefile])
AC_CONFIG_COMMANDS([default], [])
AC_OUTPUT
