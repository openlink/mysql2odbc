#
#  Makefile.am 
#
#  $Id$
#
#  mysql2odbc - A MySQL to ODBC bridge library
#  
#  Copyright (C) 2003 by OpenLink Software <iodbc@openlinksw.com>
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

AUTOMAKE_OPTIONS	= gnu dist-zip 1.7

MAINTAINERCLIEANFILES	= Makefile.in aclocal.m4 configure

noinst_PROGRAMS	= mtest
lib_LTLIBRARIES = libmysqlclient.la

noinst_HEADERS = libfakesql.h mysql.h

INCLUDES	= @ODBC_CFLAGS@


#
#  Test program
#
mtest_LDADD	= libmysqlclient.la \
		  @ODBC_LIBS@ 
mtest_SOURCES	= mtest.c


#
#  Replacement mysqlclient library
#
libmysqlclient_la_SOURCES  = libfakesql.c
libmysqlclient_la_LDFLAGS  = -version-info @mysqllib_version@ 
libmysqlclient_la_LIBADD   = @ODBC_LIBS@


if MAINTAINER_MODE
#
#  Create a tar file containing the binaries and support files
#
binary-tar:
	-mkdir @build@
	$(MAKE) install-strip prefix=`pwd`/@build@
	tar cvf @build@.tar @build@
	gzip -9vf @build@.tar
	rm -rf @build@

#
#  Create a OpenLink taz file containing the binaries and support files
#
openlink-taz:
	-mkdir @build@
	$(MAKE) install-strip prefix=`pwd`/@build@
	(cd @build@; tar cvf ../@build@.tar .)
	compress -f @build@.tar
	mv @build@.tar.Z @build@.taz
	rm -rf @build@

#
#  Create a source snapshot package
#
snapshot:
	$(MAKE) dist-gzip distdir=$(PACKAGE)-`date +"%Y%m%d"`


#
#  Create an official release package
#
release:
	$(MAKE) distcheck


#
#  Generate ChangeLog
#
changelog:
	cvs2cl --branches --no-wrap --utc --hide-filenames --prune

endif
