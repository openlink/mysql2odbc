/*
 *  mtest.c 
 *
 *  $Id$
 *
 *  Sample program
 *
 *  mysql2odbc - A MySQL to ODBC bridge library
 *  
 *  Copyright (C) 2003-2012 OpenLink Software <iodbc@openlinksw.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

#ifndef WIN32
# include <getopt.h>
#endif

#ifdef FAKE
#  include "libfakesql.h"
#else
#  ifdef WIN32
#    include "config-win.h"
#  endif
#  include <mysql.h>
#endif


int
query (MYSQL *mh, const char *q)
{
  MYSQL_RES *res;
  MYSQL_FIELD *fields;
  MYSQL_ROW row;
  unsigned int num_fields;
  unsigned int i;
  my_ulonglong affected;

  if (mysql_query (mh, q))
    {
      fprintf (stderr, "** %s.\n",
	  mysql_error (mh));
      return -1;
    }
  if ((res = mysql_use_result (mh)) == NULL)
    {
      if (mysql_field_count (mh) == 0)
	{
	  affected = mysql_affected_rows (mh);
	  printf ("(no results, %lu rows affected)\n",
	      (unsigned long) affected);
	  puts ("");
	  return 0;
	}
      else
	{
	  fprintf (stderr, "UseResult failed: %s\n",
	      mysql_error (mh));
	  return -1;
	}
    }
  num_fields = mysql_num_fields (res);
  fields = mysql_fetch_fields (res);
  while ((row = mysql_fetch_row (res)) != NULL)
    {
      //unsigned long *lengths = mysql_fetch_lengths (res);

      for (i = 0; i < num_fields; i++)
	{
	  printf ("%s: ", fields[i].name);
	  if (row[i] == NULL)
	    printf ("NULL\n");
	  else
	    printf ("%s\n", row[i]);
	}
      puts ("");
    }
  if (res)
    mysql_free_result (res);

  return 0;
}


int
main (int argc, char **argv)
{
  char *host = "localhost";
  char *db = "mysql";
  char *user = "root";
  char *pass = "";
  char line[2048];
  MYSQL *mh;

#ifndef WIN32
  int key;
  while ((key = getopt (argc, argv, "h:u:p:d:")) != EOF)
    {
      switch (key)
	{
	case 'h':
	  host = optarg;
	  break;
	case 'd':
	  db = optarg;
	  break;
	case 'u':
	  user = optarg;
	  break;
	case 'p':
	  pass = optarg;
	  break;
	default:
	  fprintf (stderr,
	      "usage: %s [-u user] [-p pass] [-h host] [-d db]\n", argv[0]);
	  return 1;
	}
    }
#endif

  mh = (MYSQL *) calloc (1, sizeof (MYSQL));
  mysql_init (mh);

  if (mysql_real_connect (mh, host, user, pass, db, 0, NULL, 0) == NULL)
    {
      fprintf (stderr, "Connection failed: %s\n", mysql_error (mh));
      free (mh);
      return 1;
    }

  printf ("Connected to MySQL %s on %s\n",
      mysql_get_server_info (mh),
      mysql_get_host_info (mh));

  for (;;)
    {
      fputs (">>", stdout);
      if (fgets (line, sizeof (line), stdin) == NULL)
	break;
      line[strlen (line) - 1] = 0;
      if (line[0] == 0 || line[0] == ';')
	continue;
      if (!strcmp (line, "quit"))
	break;

      /* Strip trailling semicolons */
      if (line[strlen(line)-1]==';')
        line[strlen(line)-1]='\0';

      query (mh, line);
    }

  mysql_close (mh);
  free (mh);

  return 0;
}
