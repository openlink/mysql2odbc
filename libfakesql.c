/*
 *  libfakesql.c 
 *
 *  $Id$
 *
 *  mysql2odbc - A MySQL to ODBC bridge library
 *  
 *  Copyright (C) 2003-2018 OpenLink Software <iodbc@openlinksw.com>
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

#ifdef WIN32
# include <windows.h>
# include <config-win.h>
#else
# define STDCALL
#endif
#include <sql.h>
#include <sqlext.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <mysql.h>

#ifndef SQLLEN
# define SQLLEN SQLINTEGER
#endif

#ifdef WIN32
# define snprintf _snprintf
#endif

#define DBOF(X)			((TSQLPrivate *)((X)->net.vio))

#define UNIMPLEMENTED_VOID
#define UNIMPLEMENTED_OK	return (0);
#define UNIMPLEMENTED_FAIL	return (-1);
#define UNIMPLEMENTED(X)	return (X)0

#define safe_free(x)		{ if (x) free(x); }
#define safe_dup(x)		((x) ? strdup (x) : NULL)

#define TRACE(T)

/* from errmsg.h */
#define CR_UNKNOWN_ERROR	2000
#define CR_OUT_OF_MEMORY	2008
#define CR_SERVER_LOST		2013

#define CR_ODBC_ERROR		9999

typedef struct SSQLPrivate TSQLPrivate;

struct SSQLPrivate
  {
    SQLHENV	hEnv;
    SQLHDBC	hDbc;
    SQLHSTMT	hStmt;
    int		bConnected;
    int		bHaveData;
    int		bPrepared;
  };

/* Prototypes */
static int
	_alloc_db (MYSQL *mysql);
static void
	_free_db (MYSQL *mysql);
static int
	_connect_db (MYSQL *mysql, const char *connStr);
static void
	_set_error (MYSQL *mysql, unsigned int err);
static void
	_fetch_db_errors (MYSQL *mysql, const char *where, int save);
static TSQLPrivate *
	_db (MYSQL *mysql);
static int
	_trap_sqlerror (MYSQL *mysql, SQLRETURN rc, const char *where);
static MYSQL_FIELD *
	_alloc_fields (MYSQL *mysql, unsigned int field_count);
static void
	_free_fields (MYSQL *mysql);
static MYSQL_RES *
	_alloc_res (MYSQL *mysql);
static void
	_free_res (MYSQL_RES *res);
static int
	_append_row (MYSQL_DATA *data, MYSQL_ROWS **pp);
static void
	_free_data (MYSQL_DATA *data);
static MYSQL *
	_impl_init (MYSQL *mysql);
static void
	_impl_close (MYSQL *mysql);
static MYSQL *
	_impl_real_connect (MYSQL *mysql, const char *host, const char *user,
	    const char *passwd, const char *db, unsigned int port,
	    const char *unix_socket, unsigned int clientflag);
static MYSQL *
	_impl_connect (MYSQL *mysql, const char *host, const char *user,
	    const char *passwd);
static int
	_impl_query (MYSQL *mysql, const char *query, long len);
static MYSQL_RES *
	_impl_use_result (MYSQL *mysql);
static MYSQL_RES *
	_impl_store_result (MYSQL *mysql);
static MYSQL_ROW
	_impl_fetch_row (MYSQL_RES *res);


static int
_alloc_db (MYSQL *mysql)
{
  TSQLPrivate *pDB;

  pDB = (TSQLPrivate *) calloc (1, sizeof (TSQLPrivate));
  if (pDB == NULL)
    {
      _set_error (mysql, CR_OUT_OF_MEMORY);
      return -1;
    }

  pDB->hEnv = SQL_NULL_HENV;
  pDB->hDbc = SQL_NULL_HDBC;
  pDB->hStmt = SQL_NULL_HSTMT;

  _set_error (mysql, 0);

#if 0
  DBOF(mysql) = pDB;
#else
  mysql->net.vio = pDB;
#endif

  return 0;
}


static void
_free_db (MYSQL *mysql)
{
  TSQLPrivate *pDB;

  pDB = DBOF(mysql);
  if (pDB)
    {
      if (pDB->hStmt != SQL_NULL_HSTMT)
	SQLFreeStmt (pDB->hStmt, SQL_DROP);
      if (pDB->bConnected)
	SQLDisconnect (pDB->hDbc);
      if (pDB->hDbc != SQL_NULL_HDBC)
	SQLFreeConnect (pDB->hDbc);
      if (pDB->hEnv != SQL_NULL_HENV)
	SQLFreeEnv (pDB->hEnv);

      pDB->hEnv = SQL_NULL_HENV;
      pDB->hDbc = SQL_NULL_HDBC;
      pDB->hStmt = SQL_NULL_HSTMT;
      pDB->bConnected = 0;
      free (pDB);
#if 0
      DBOF(mysql) = NULL;
#else
      mysql->net.vio = NULL;
#endif
    }
}


static TSQLPrivate *
_db (MYSQL *mysql)
{
  TSQLPrivate *pDB;

  if (mysql == NULL)
    return NULL;

  if ((pDB = DBOF(mysql)) == NULL)
    {
      _set_error (mysql, CR_OUT_OF_MEMORY);
      return NULL;
    }

  if (!pDB->bConnected || pDB->hStmt == SQL_NULL_HSTMT)
    {
      _set_error (mysql, CR_SERVER_LOST);
      return NULL;
    }

  _set_error (mysql, 0);

  return pDB;
}


static int
_connect_db (MYSQL *mysql, const char *connStr)
{
  TSQLPrivate *pDB;
  SQLSMALLINT buflen;
  SQLCHAR buf[257];
  SQLRETURN ret;

  pDB = DBOF(mysql);

  ret = SQLAllocEnv (&pDB->hEnv);
  if (_trap_sqlerror (mysql, ret, "SQLAllocEnv"))
    return -1;

  ret = SQLAllocConnect (pDB->hEnv, &pDB->hDbc);
  if (_trap_sqlerror (mysql, ret, "SQLAllocConnect"))
    return -1;

#if 0
  /*
   *  Set the application name 
   */
  ret = SQLSetConnectOption (pDB->hDbc, SQL_APPLICATION_NAME,
      (UDWORD) "fake_mysql_client");
#endif

  /*
   *  Either use the connect string provided on the command line or
   *  ask for one. If an empty string or a ? is given, show a nice
   *  list of options
   */
  ret = SQLDriverConnect (pDB->hDbc, 0, (SQLCHAR *) connStr, SQL_NTS, buf,
	  sizeof (buf), &buflen, SQL_DRIVER_NOPROMPT);
  if (_trap_sqlerror (mysql, ret, "SQLAllocConnect"))
    return -1;

  pDB->bConnected = 1;

  ret = SQLAllocStmt (pDB->hDbc, &pDB->hStmt);
  if (_trap_sqlerror (mysql, ret, "SQLAllocConnect"))
    return -1;

  return 0;
}


static void
_set_error (MYSQL *mysql, unsigned int err)
{
  mysql->net.last_errno = err;

  switch (err)
    {
    case CR_OUT_OF_MEMORY:
      strcpy (mysql->net.last_error, "MySQL client run out of memory");
      break;

    case CR_UNKNOWN_ERROR:
      strcpy (mysql->net.last_error, "Unknown MySQL error");
      break;

    case CR_SERVER_LOST:
      strcpy (mysql->net.last_error, "MySQL server has gone away");
      break;

    default:
      mysql->net.last_error[0] = 0;
    }
}


/*
 *  Show all the error information that is available
 */
static void
_fetch_db_errors (MYSQL *mysql, const char *where, int save)
{
  TSQLPrivate *pDB;
  SQLCHAR buf[512];
  SQLCHAR sqlstate[15];
  SQLRETURN ret;
  char *copy = NULL;

  pDB = DBOF(mysql);

  /* Get statement errors */
  if (pDB->hStmt)
    {
      for (;;)
	{
	  ret = SQLError (pDB->hEnv, pDB->hDbc, pDB->hStmt,
	      sqlstate, NULL, buf, sizeof(buf), NULL);
	  if (ret != SQL_SUCCESS)
	    break;
	  if (save && copy == NULL)
	    copy = strdup ((const char *)buf);
#ifdef DEBUG
	  fprintf (stderr, "%s, SQLSTATE=%s\n", buf, sqlstate);
#endif
	}
    }

  /* Get connection errors */
  if (pDB->hDbc)
    {
      for (;;)
	{
	  ret = SQLError (pDB->hEnv, pDB->hDbc, SQL_NULL_HSTMT,
	      sqlstate, NULL, buf, sizeof(buf), NULL);
	  if (ret != SQL_SUCCESS)
	    break;
	  if (save && copy == NULL)
	    copy = strdup ((const char *)buf);
#ifdef DEBUG
	  fprintf (stderr, "%s, SQLSTATE=%s\n", buf, sqlstate);
#endif
	}
    }

  /* Get environment errors */
  if (pDB->hEnv)
    {
      for (;;)
	{
	  ret = SQLError (pDB->hEnv, SQL_NULL_HDBC, SQL_NULL_HSTMT,
	      sqlstate, NULL, buf, sizeof(buf), NULL);
	  if (ret != SQL_SUCCESS)
	    break;
	  if (save && copy == NULL)
	    copy = strdup ((const char *)buf);
#ifdef DEBUG
	  fprintf (stderr, "%s, SQLSTATE=%s\n", buf, sqlstate);
#endif
	}
    }

  if (copy)
    {
      char *cp, *dp;
      int i;

      /* Remove [vendor info][driver info] */
      cp = copy;
      for (i = 0; i < 2; i++)
	{
	  if (*cp != '[' || (dp = strchr (cp, ']')) == NULL)
	    break;
	  cp = dp + 1;
	}
      if (cp > copy)
	{
	  if (*cp == ' ')
	    cp++;
	  if (cp[0] && cp[1])
	    strcpy (copy, cp);
	}

      /* Remove trailing \n */
      if ((cp = strchr (copy, '\n')) != NULL)
	*cp = 0;

      strncpy (mysql->net.last_error, copy, MYSQL_ERRMSG_SIZE);
      mysql->net.last_error[MYSQL_ERRMSG_SIZE - 1] = 0;
      free (copy);
    }
}


static int
_trap_sqlerror (MYSQL *mysql, SQLRETURN rc, const char *where)
{
#ifdef DEBUG
  fprintf (stderr, "%s: ret=%d\n", where, rc);
#endif
  switch (rc)
    {
    case SQL_SUCCESS:
    case SQL_NO_DATA_FOUND:
      break;

    case SQL_SUCCESS_WITH_INFO:
      _fetch_db_errors (mysql, where, 0);
      break;

    case SQL_INVALID_HANDLE:
      _set_error (mysql, CR_SERVER_LOST);
      DBOF(mysql)->bConnected = 0;
      return -1;

    case SQL_ERROR:
      _set_error (mysql, CR_ODBC_ERROR);
      _fetch_db_errors (mysql, where, 1);
      return -1;

    default:
      _set_error (mysql, CR_UNKNOWN_ERROR);
      return -1;
    }

  return 0;
}


static MYSQL_FIELD *
_alloc_fields (MYSQL *mysql, unsigned int field_count)
{
  MYSQL_FIELD *f;

/*_free_fields (mysql);*/
  if (field_count)
    {
      f = (MYSQL_FIELD *) calloc (field_count, sizeof (MYSQL_FIELD));
      if (f == NULL)
	{
	  _set_error (mysql, CR_OUT_OF_MEMORY);
	  return NULL;
	}
    }
  else
    f = NULL;

  mysql->fields = f;
  mysql->field_count = field_count;

  return f;
}


static void
_free_fields (MYSQL *mysql)
{
  unsigned int i;
  MYSQL_FIELD *f;

  if ((f = mysql->fields) != NULL)
    {
      for (i = 0; i < mysql->field_count; i++, f++)
	{
	  safe_free (f->name);
	  safe_free (f->table);
	  safe_free (f->def);
	}
      free (mysql->fields);
      mysql->fields = NULL;
      mysql->field_count = 0;
    }
}


static MYSQL_RES *
_alloc_res (MYSQL *mysql)
{
  MYSQL_RES *res;
  MYSQL_FIELD *f;
  unsigned int j;

  if (mysql->fields == NULL)
    return NULL;

  if ((res = (MYSQL_RES *) calloc (1, sizeof (MYSQL_RES))) == NULL)
    goto failed;

  res->row_count = 0;
  res->current_field = 0;
  res->field_count = mysql->field_count;
  res->fields = mysql->fields;
  res->handle = mysql;
  res->eof = 0;

  /* Put indicators here */
  res->lengths = (unsigned long *) calloc (res->field_count, sizeof (SQLLEN));

  /* These hold allocated fields */
  res->row = (MYSQL_ROW) calloc (res->field_count, sizeof (char *));

  for (f = res->fields, j = 0; j < res->field_count; j++, f++)
    {
      /* Allocation amount */
      f->max_length = f->length + 32;
      if ((res->row[j] = malloc (res->fields[j].max_length)) == NULL)
	goto failed;
    }

  return res;

failed:
  _set_error (mysql, CR_OUT_OF_MEMORY);
  _free_res (res);

  return NULL;
}


static void
_free_res (MYSQL_RES *res)
{
  unsigned int j;

  if (res)
    {
      if (res->lengths)
	free (res->lengths);
      if (res->row)
	{
	  for (j = 0; j < res->field_count; j++)
	    {
	      if (res->row[j])
		free (res->row[j]);
	    }
	  free (res->row);
	}
      if (res->data)
	_free_data (res->data);
      else if (res->current_row)
	free (res->current_row);
      free (res);
    }
}


static int
_append_row (MYSQL_DATA *data, MYSQL_ROWS **pp)
{
  MYSQL_ROWS *rows;

  rows = (MYSQL_ROWS *) calloc (1,
      sizeof (MYSQL_ROWS)
      + data->fields * sizeof (char *));
  if (rows == NULL)
    return -1;

  rows->next = NULL;
  rows->data = (char **) (rows + 1);

  if (*pp)
    (*pp)->next = rows;
  else
    data->data = rows;
  *pp = rows;

  data->rows++;

  return 0;
}


static void
_free_data (MYSQL_DATA *data)
{
  unsigned int j;
  MYSQL_ROWS *r, *r2;

  if (data)
    {
      if (data->data)
	{
	  for (r = data->data; r; r = r2)
	    {
	      if (r->data)
		{
		  for (j = 0; j < data->fields; j++)
		    {
		      if (r->data[j])
			free (r->data[j]); /* char * */
		    }
		}
	      r2 = r->next;
	      free (r); /* MYSQL_ROWS */
	    }
	}
      free (data); /* MYSQL_DATA */
    }
}


/******************************************************************************/


/* Very, very dummy */
unsigned long max_allowed_packet = 0;
unsigned long net_buffer_length = 0;


static MYSQL *
_impl_init (MYSQL *mysql)
{
  if (mysql == NULL)
    {
      if ((mysql = malloc (sizeof (MYSQL))) == NULL)
	return NULL;
      mysql->free_me = 1;
    }
  memset (mysql, 0, sizeof (MYSQL));

  return mysql;
}


static void 
_impl_close (MYSQL *mysql)
{
  if (mysql)
    {
      safe_free (mysql->host);
      safe_free (mysql->user);
      safe_free (mysql->passwd);
      safe_free (mysql->unix_socket);
      safe_free (mysql->server_version);
      safe_free (mysql->host_info);
      safe_free (mysql->info);
      safe_free (mysql->db);
      _free_db (mysql);
      _free_fields (mysql);
      if (mysql->free_me)
	free (mysql);
    }
}


static MYSQL *
_impl_real_connect (
    MYSQL *mysql,
    const char *host,
    const char *user,
    const char *passwd,
    const char *db,
    unsigned int port,
    const char *unix_socket,
    unsigned int clientflag)
{
  int save_free;
  char dsn[512];

  /* this is all to fool the client */
  mysql->host = safe_dup (host);
  mysql->user = safe_dup (user);
  mysql->passwd = safe_dup (passwd);
  mysql->db = safe_dup (db);
  mysql->unix_socket = safe_dup (unix_socket);
  mysql->port = port;
  mysql->client_flag = clientflag;
  mysql->server_version = strdup (MYSQL_SERVER_VERSION "-" MYSQL_SERVER_SUFFIX);
  mysql->host_info = strdup ("localhost via TCP/IP");
  mysql->protocol_version = PROTOCOL_VERSION;
  mysql->thread_id = 100;
  mysql->server_capabilities = 0;
  mysql->affected_rows = (my_ulonglong) -1;
  mysql->status = MYSQL_STATUS_READY;

  snprintf (dsn, sizeof (dsn), "DSN=%s;UID=%s;PWD=%s",
      db && db[0] ? db : "mysql",
      user ? user : "",
      passwd ? passwd : "");

  if (_alloc_db (mysql))
    goto failed;

  if (_connect_db (mysql, dsn))
    goto failed;

  return mysql;

failed:
  save_free = mysql->free_me;
  mysql->free_me = 0;
  _impl_close (mysql);
  mysql->free_me = save_free;

  return NULL;
}


static MYSQL *
_impl_connect (
    MYSQL *mysql,
    const char *host,
    const char *user,
    const char *passwd)
{
  MYSQL *res;

  mysql = _impl_init (mysql);
  res = _impl_real_connect (mysql, host, user, passwd, NULL, 0, NULL, 0);
  if (res == NULL)
    _impl_close (mysql);

  return res;
}


static int
_impl_query (
    MYSQL *mysql,
    const char *query,
    long len)
{
  TSQLPrivate *pDB;
  SQLSMALLINT col, numCols;
  SQLLEN numRows;
  SQLRETURN ret;
  MYSQL_FIELD *f;

  if ((pDB = _db (mysql)) == NULL)
    return -1;

  /* Close previous stmt */
  if (pDB->bPrepared)
    {
      SQLFreeStmt (pDB->hStmt, SQL_CLOSE);
      pDB->bPrepared = 0;
    }

  pDB->bHaveData = FALSE;

  /* Prepare & execute new one */
  ret = SQLExecDirect (pDB->hStmt, (SQLCHAR *) query, (SQLINTEGER) len);
  if (_trap_sqlerror (mysql, ret, "SQLExecDirect"))
    return -1;

  pDB->bPrepared = 1;
  pDB->bHaveData = (ret != SQL_NO_DATA);

  /* Allocate column descriptors */
  numCols = 0;
  if (pDB->bHaveData)
    {
      ret = SQLNumResultCols (pDB->hStmt, &numCols);
      if (_trap_sqlerror (mysql, ret, "SQLNumResultCols"))
	return -1;
    }

  f = _alloc_fields (mysql, (unsigned int) numCols);

  /* Query field properties */
  for (col = 1; col <= numCols; col++, f++)
    {
      SQLLEN lValue;
      SQLSMALLINT retLen;
      SQLCHAR value[128];

      /* TODO */
      f->type = FIELD_TYPE_STRING;

      /* field.table */
      value[0] = 0;
      ret = SQLColAttribute (pDB->hStmt, col, SQL_DESC_TABLE_NAME,
	  value, (SQLSMALLINT) sizeof (value), &retLen, &lValue);
      if (_trap_sqlerror (mysql, ret, "SQLColAttribute"))
	return -1;
      f->table = strdup ((const char *)value);

      /* field.name */
      value[0] = 0;
      ret = SQLColAttribute (pDB->hStmt, col, SQL_DESC_LABEL,
	  value, (SQLSMALLINT) sizeof (value), &retLen, &lValue);
      if (_trap_sqlerror (mysql, ret, "SQLColAttribute"))
	return -1;
      f->name = strdup ((const char *)value);

      /* field.length */
      lValue = 0;
      ret = SQLColAttribute (pDB->hStmt, col, SQL_DESC_DISPLAY_SIZE,
	  value, (SQLSMALLINT) sizeof (value), &retLen, &lValue);
      if (_trap_sqlerror (mysql, ret, "SQLColAttribute"))
	return -1;
      if (lValue < 0) /* blobs give -1(?) */
	lValue = 65500;
      f->length = (unsigned int) lValue;

      /* TODO set field.db in MySQL4 emulation mode */
    }

  /* Retrieve affected_rows */
  ret = SQLRowCount (pDB->hStmt, &numRows);
  if (_trap_sqlerror (mysql, ret, "SQLRowCount"))
    mysql->affected_rows = (my_ulonglong) -1;
  else
    mysql->affected_rows = (my_ulonglong) numRows;

  return 0;
}


static MYSQL_RES *
_impl_use_result (MYSQL *mysql)
{
  TSQLPrivate *pDB;
  MYSQL_RES *res;
  unsigned int j;
  SQLRETURN ret;

  if ((pDB = _db (mysql)) == NULL)
    return NULL;

  /* note: this could also fail if there are no fields (eg. after INSERT) */
  if ((res = _alloc_res (mysql)) == NULL)
    return NULL;

  /* This array of fields is returned to the caller -
   *  if sqlind == SQL_NULL_DATA, then corresponding value = NULL
   */
  res->current_row = (MYSQL_ROW) calloc (res->field_count, sizeof (char *));
  if (res->current_row == NULL)
    goto failed;

  /* Bind the result set */
  SQLFreeStmt (pDB->hStmt, SQL_UNBIND);
  for (j = 0; j < res->field_count; j++)
    {
      ret = SQLBindCol (
	  pDB->hStmt,
	  (SQLUSMALLINT) (j + 1),
	  SQL_C_CHAR,
	  res->row[j],
	  (SQLINTEGER) res->fields[j].max_length,
	  (SQLLEN *) &res->lengths[j]);

      if (_trap_sqlerror (mysql, ret, "SQLBindCol"))
	{
	  _free_res (res);
	  return NULL;
	}
    }

  return res;

failed:
  _set_error (mysql, CR_OUT_OF_MEMORY);
  _free_res (res);
  return NULL;
}


static MYSQL_RES *
_impl_store_result (MYSQL *mysql)
{
  TSQLPrivate *pDB;
  MYSQL_RES *res;
  unsigned int j;
  SQLRETURN ret;
  SQLLEN *ind;
  MYSQL_ROWS *rp = NULL;

  if ((pDB = _db (mysql)) == NULL)
    return NULL;

  /* note: this could also fail if there are no fields (eg. after INSERT) */
  if ((res = _alloc_res (mysql)) == NULL)
    return NULL;

  /* Bind the result set */
  SQLFreeStmt (pDB->hStmt, SQL_UNBIND);
  for (j = 0; j < res->field_count; j++)
    {
      ret = SQLBindCol (
	  pDB->hStmt,
	  (SQLUSMALLINT) (j + 1),
	  SQL_C_CHAR,
	  res->row[j],
	  res->fields[j].max_length,
	  (SQLLEN *) &res->lengths[j]);

      if (_trap_sqlerror (mysql, ret, "SQLBindCol"))
	{
	  _free_res (res);
	  return NULL;
	}
    }

  res->data = (MYSQL_DATA *) calloc (1, sizeof (MYSQL_DATA));
  if (res->data == NULL)
    goto failed;
  res->data->fields = mysql->field_count;

  /* Now fetch all the records */
  ind = (SQLLEN *) res->lengths;
  for (;;)
    {
      ret = SQLFetch (pDB->hStmt);
      if (_trap_sqlerror (res->handle, ret, "SQLFetch"))
	return NULL;
      if (ret == SQL_NO_DATA_FOUND)
	break;

      if (_append_row (res->data, &rp) == -1)
	{
	  /* I don't 'goto failed' here, because maybe we've already
	   * collected a lot of info...
	   */
	  _set_error (mysql, CR_OUT_OF_MEMORY);
	  break;
	}

      for (j = 0; j < res->field_count; j++)
	{
	  if (ind[j] != SQL_NULL_DATA)
	    rp->data[j] = strdup (res->row[j]);
	}

#if 0
      /* TODO should we limit the max # of rows somehow? */
      if (res->data->rows > 1000)
	break;
#endif
    }

  res->data_cursor = res->data->data;

  return res;

failed:
  _set_error (mysql, CR_OUT_OF_MEMORY);
  _free_res (res);
  return NULL;
}


static MYSQL_ROW
_impl_fetch_row (MYSQL_RES *res)
{
  TSQLPrivate *pDB;
  unsigned int j;
  SQLRETURN ret;
  SQLLEN *ind;

  if (res->data)
    {
      /* if we're here, the result set is from store_result */
      if (!res->data_cursor)
	res->current_row = NULL;
      else
	{
	  res->current_row = res->data_cursor->data;
	  res->data_cursor = res->data_cursor->next;
	  res->row_count++;
	}
      return res->current_row;
    }

  pDB = _db (res->handle);
  if (res->eof || pDB == NULL)
    return NULL;

  ret = SQLFetch (pDB->hStmt);
  if (_trap_sqlerror (res->handle, ret, "SQLFetch"))
    return NULL;

  if (ret == SQL_NO_DATA_FOUND)
    {
      res->eof = 1;
      return NULL;
    }

  ind = (SQLLEN *) res->lengths;
  for (j = 0; j < res->field_count; j++)
    {
      if (ind[j] == SQL_NULL_DATA)
	res->current_row[j] = NULL;
      else
	res->current_row[j] = res->row[j];
    }

  res->row_count++;

  return res->current_row;
}


/******************************************************************************/


MYSQL * STDCALL
mysql_init (MYSQL *mysql)
{
  MYSQL *res;

  TRACE ("mysql_init");
  res = _impl_init (mysql);

  return res;
}


MYSQL * STDCALL
mysql_connect (
    MYSQL *mysql,
    const char *host,
    const char *user,
    const char *passwd)
{
  MYSQL *res;

  TRACE ("mysql_connect");
  res = _impl_connect (mysql, host, user, passwd);

  return res;
}


MYSQL * STDCALL
mysql_real_connect (
    MYSQL *mysql,
    const char *host,
    const char *user,
    const char *passwd,
    const char *db,
    unsigned int port,
    const char *unix_socket,
    unsigned int clientflag)
{
  MYSQL *res;

  TRACE ("mysql_real_connect");
  res = _impl_real_connect (mysql, host, user, passwd, db, port,
      unix_socket, clientflag);

  return res;
}


void STDCALL 
mysql_close (MYSQL *mysql)
{
  TRACE ("mysql_close");
  _impl_close (mysql);
}


unsigned int STDCALL
mysql_errno (MYSQL *mysql)
{
  TRACE ("mysql_errno");
  return mysql->net.last_errno;
}


char * STDCALL
mysql_error (MYSQL *mysql)
{
  TRACE ("mysql_error");
  return mysql->net.last_error;
}


char * STDCALL
mysql_info (MYSQL *mysql)
{
  TRACE ("mysql_info");
  return mysql->info;
}


int STDCALL
mysql_query (MYSQL *mysql, const char *q)
{
  int rc;

  TRACE ("mysql_query");
  rc = _impl_query (mysql, q, SQL_NTS);
  return rc;
}


int STDCALL
mysql_send_query (MYSQL *mysql, const char *q, unsigned int length)
{
  TRACE ("mysql_send_query");
  UNIMPLEMENTED_OK;
}


int STDCALL
mysql_read_query_result (MYSQL *mysql)
{
  TRACE ("mysql_read_query_result");
  UNIMPLEMENTED_OK;
}


int STDCALL
mysql_real_query (MYSQL *mysql, const char *q, unsigned int length)
{
  int rc;

  TRACE ("mysql_real_query");
  rc = _impl_query (mysql, q, (long) length);
  return rc;
}


MYSQL_RES * STDCALL
mysql_use_result (MYSQL *mysql)
{
  MYSQL_RES *res;

  TRACE ("mysql_use_result");
  res = _impl_use_result (mysql);
  return res;
}


MYSQL_RES * STDCALL
mysql_store_result (MYSQL *mysql)
{
  MYSQL_RES *res;

  TRACE ("mysql_store_result");
  res = _impl_store_result (mysql);
  return res;
}


MYSQL_RES * STDCALL
mysql_list_dbs (MYSQL *mysql, const char *wild)
{
  TRACE ("mysql_list_dbs UNIMPLEMENTED");
  UNIMPLEMENTED (MYSQL_RES *);
}


MYSQL_RES * STDCALL
mysql_list_tables (MYSQL *mysql, const char *wild)
{
  TRACE ("mysql_list_tables UNIMPLEMENTED");
  UNIMPLEMENTED (MYSQL_RES *);
}


MYSQL_RES * STDCALL
mysql_list_fields (MYSQL *mysql, const char *table, const char *wild)
{
  TRACE ("mysql_list_fields UNIMPLEMENTED");
  UNIMPLEMENTED (MYSQL_RES *);
}


MYSQL_RES * STDCALL
mysql_list_processes (MYSQL *mysql)
{
  TRACE ("mysql_list_processes UNIMPLEMENTED");
  UNIMPLEMENTED (MYSQL_RES *);
}


void STDCALL
mysql_free_result (MYSQL_RES *res)
{
  TRACE ("mysql_free_result");
  _free_res (res);
}


my_bool STDCALL
mysql_eof (MYSQL_RES *res)
{
  TRACE ("mysql_eof");
  return res->eof;
}


my_ulonglong STDCALL
mysql_num_rows (MYSQL_RES *res)
{
  TRACE ("mysql_row_count");
  return res->row_count; /* This is not correct before read all data */
}


unsigned int STDCALL
mysql_num_fields (MYSQL_RES *res)
{
  TRACE ("mysql_num_fields");
  return res->field_count;
}


MYSQL_FIELD * STDCALL
mysql_fetch_field_direct (MYSQL_RES *res, unsigned int fieldnr)
{
  TRACE ("mysql_fetch_field_direct");
  return &res->fields[fieldnr];
}


MYSQL_FIELD * STDCALL
mysql_fetch_fields (MYSQL_RES *res)
{
  TRACE ("mysql_fetch_fields");
  return res->fields;
}


MYSQL_ROWS * STDCALL
mysql_row_tell (MYSQL_RES *res)
{
  TRACE ("mysql_row_tell");
  return res->data_cursor;
}


unsigned int STDCALL
mysql_field_tell (MYSQL_RES *res)
{
  TRACE ("mysql_field_tell");
  return res->current_field;
}


unsigned int 
mysql_field_count (MYSQL *mysql)
{
  TRACE ("mysql_field_count");
  return mysql->field_count;
}


my_ulonglong STDCALL
mysql_affected_rows (MYSQL *mysql)
{
  TRACE ("mysql_affected_rows");
  return mysql->affected_rows;
}


my_ulonglong STDCALL
mysql_insert_id (MYSQL *mysql)
{
  TRACE ("mysql_insert_id");
  return mysql->insert_id;
}


void STDCALL
mysql_data_seek (MYSQL_RES *res, my_ulonglong offset)
{
  TRACE ("mysql_data_seek UNIMPLEMENTED");
  UNIMPLEMENTED_VOID;
}


MYSQL_ROW_OFFSET STDCALL
mysql_row_seek (MYSQL_RES *res, MYSQL_ROW_OFFSET offset)
{
  MYSQL_ROW_OFFSET old;

  TRACE ("mysql_row_seek");
  old = res->data_cursor;
  res->data_cursor = offset;
  res->current_row = NULL;

  return old;
}


MYSQL_FIELD_OFFSET STDCALL
mysql_field_seek (MYSQL_RES *res, MYSQL_FIELD_OFFSET offset)
{
  MYSQL_FIELD_OFFSET old;

  TRACE ("mysql_field_seek");
  old = res->current_field;
  res->current_field = offset;

  return old;
}


MYSQL_ROW STDCALL
mysql_fetch_row (MYSQL_RES *res)
{
  TRACE ("mysql_fetch_row");
  return _impl_fetch_row (res);
}


unsigned long * STDCALL
mysql_fetch_lengths (MYSQL_RES *res)
{
  unsigned long *lengths;
  unsigned int i;
  MYSQL_ROW column;

  TRACE ("mysql_fetch_lengths");

  if (!(column=res->current_row))               
    return 0;                                   /* Something is wrong */

  if (res->data)
    {   
      lengths=res->lengths;
      for (i = 0 ; i < res->field_count ; column++,i ++)
	{
	  if (!*column)
	    {     
	      lengths[i]=0;                             /* Null */
	      continue;
	    }
	  lengths[i] = (unsigned int) strlen (*column);
	}
    }

  return res->lengths;
}


MYSQL_FIELD * STDCALL
mysql_fetch_field (MYSQL_RES *res)
{
  TRACE ("mysql_fetch_field");

#ifdef DEBUG
  fprintf (stdout, "current_field (%i) field_count (%i) \n", res->current_field, res->field_count);
#endif  

  if (res->current_field >= res->field_count)
    return NULL;
  
  return &res->fields[res->current_field++];
}


my_bool STDCALL
mysql_change_user (
    MYSQL *mysql,
    const char *user,
    const char *passwd,
    const char *db)
{
  TRACE ("mysql_change_user UNIMPLEMENTED");
  UNIMPLEMENTED (my_bool);
}


int STDCALL
mysql_select_db (MYSQL *mysql, const char *db)
{
  TRACE ("mysql_select_db");
   mysql->db = strdup (db);
   return 0;
}


int STDCALL
mysql_create_db (MYSQL *mysql, const char *DB)
{
  TRACE ("mysql_select_db UNIMPLEMENTED");
  return 0;
}


int STDCALL
mysql_drop_db (MYSQL *mysql, const char *DB)
{
  TRACE ("mysql_drop_db UNIMPLEMENTED");
  UNIMPLEMENTED_FAIL;
}


int STDCALL
mysql_shutdown (MYSQL *mysql)
{
  TRACE ("mysql_shutdown UNIMPLEMENTED");
  UNIMPLEMENTED_OK;
}


char * STDCALL
mysql_get_server_info (MYSQL *mysql)
{
  TRACE ("mysql_get_server_info");
  return mysql->server_version;
}


char * STDCALL
mysql_get_client_info (void)
{
  TRACE ("mysql_get_client_info");
  return MYSQL_SERVER_VERSION;
}


char * STDCALL
mysql_get_host_info (MYSQL *mysql)
{
  TRACE ("mysql_get_host_info");
  return mysql->host_info;
}


unsigned int STDCALL
mysql_get_proto_info (MYSQL *mysql)
{
  TRACE ("mysql_get_proto_info");
  return mysql->protocol_version;
}


unsigned long STDCALL
mysql_thread_id (MYSQL *mysql)
{
  TRACE ("mysql_thread_id");
  return (mysql)->thread_id;
}


unsigned int STDCALL
mysql_thread_safe (void)
{
  TRACE ("mysql_thread_safe");
  return 1;
}


const char *
mysql_character_set_name (MYSQL *mysql)
{
  TRACE ("mysql_character_set_name");
  return "latin1";
}


int STDCALL
mysql_dump_debug_info (MYSQL *mysql)
{
  TRACE ("mysql_dump_debug_info UNIMPLEMENTED");
  UNIMPLEMENTED (int);
}


int STDCALL
mysql_refresh (MYSQL *mysql, unsigned int refresh_options)
{
  TRACE ("mysql_refresh UNIMPLEMENTED");
  UNIMPLEMENTED (int);
}


int STDCALL
mysql_kill (MYSQL *mysql, unsigned long pid)
{
  TRACE ("mysql_kill UNIMPLEMENTED");
  UNIMPLEMENTED (int);
}


int STDCALL
mysql_ping (MYSQL *mysql)
{
  TRACE ("mysql_ping UNIMPLEMENTED");
  UNIMPLEMENTED (int);
}


char * STDCALL
mysql_stat (MYSQL *mysql)
{
  TRACE ("mysql_stat UNIMPLEMENTED");
  UNIMPLEMENTED (char *);
}


int STDCALL
mysql_options (MYSQL *mysql, enum mysql_option option, const char *arg)
{
  TRACE ("mysql_options UNIMPLEMENTED");
  UNIMPLEMENTED (int);
}


unsigned long STDCALL
mysql_escape_string (char *to, const char *from, unsigned long from_length)
{
  TRACE ("mysql_escape_string UNIMPLEMENTED");
  UNIMPLEMENTED (unsigned long);
}


unsigned long 
mysql_real_escape_string (MYSQL *mysql,
    char *to, const char *from, unsigned long length)
{
  TRACE ("mysql_real_escape_string UNIMPLEMENTED");
  UNIMPLEMENTED (unsigned long);
}


void STDCALL
mysql_debug (const char *debug)
{
  TRACE ("mysql_debug UNIMPLEMENTED");
  UNIMPLEMENTED_VOID;
}


char * STDCALL
mysql_odbc_escape_string (MYSQL *mysql,
    char *to,
    unsigned long to_length,
    const char *from,
    unsigned long from_length,
    void *param,
    char *(*extend_buffer) (void *, char *to, unsigned long *length))
{
  TRACE ("mysql_odbc_escape_string UNIMPLEMENTED");
  UNIMPLEMENTED (char *);
}


void STDCALL
myodbc_remove_escape (MYSQL *mysql, char *name)
{
  TRACE ("myodbc_remove_escape UNIMPLEMENTED");
  UNIMPLEMENTED_VOID;
}


void
my_init (void)
{
  TRACE ("my_init UNIMPLEMENTED");
  UNIMPLEMENTED_VOID;
}


void
load_defaults (
    const char *conf_file,
    const char **groups,
    int *argc,
    char ***argv)
{
  TRACE ("load_defaults UNIMPLEMENTED");
  UNIMPLEMENTED_VOID;
}


void
print_defaults (const char *conf_file, const char **groups)
{
  TRACE ("print_defaults UNIMPLEMENTED");
  UNIMPLEMENTED_VOID;
}


void
free_defaults (void)
{
  TRACE ("free_defaults UNIMPLEMENTED");
  UNIMPLEMENTED_VOID;
}
