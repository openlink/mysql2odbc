/*
 *  libfakesql.h 
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

#ifndef _fake_sql_h
#define _fake_sql_h

#define PROTOCOL_VERSION	10
#define MYSQL_SERVER_VERSION	"3.23.49"
#define MYSQL_SERVER_SUFFIX	"iodbc"
#define FRM_VER			6
#define MYSQL_VERSION_ID	32349
#define MYSQL_PORT		3306

#ifndef MYSQL_UNIX_ADDR
#define MYSQL_UNIX_ADDR		"/tmp/mysql.sock"
#endif

#ifdef WIN32
#define MYSQL_NAMEDPIPE		"MySQL"
#define MYSQL_SERVICENAME	"MySql"
#endif

#ifndef MYSQL_CHARSET
#define MYSQL_CHARSET		"latin1"
#endif

///////////////////////////////////////////////////////////////////////////////

#define NAME_LEN		64		/* Field/table name length */
#define HOSTNAME_LENGTH		60
#define USERNAME_LENGTH		16
#define SERVER_VERSION_LENGTH	60

#define LOCAL_HOST		"localhost"
#define LOCAL_HOST_NAMEDPIPE	"."

#define NOT_NULL_FLAG		1	/* Field can't be NULL */
#define PRI_KEY_FLAG		2	/* Field is part of a primary key */
#define UNIQUE_KEY_FLAG 	4	/* Field is part of a unique key */
#define MULTIPLE_KEY_FLAG	8	/* Field is part of a key */
#define BLOB_FLAG		16	/* Field is a blob */
#define UNSIGNED_FLAG		32	/* Field is unsigned */
#define ZEROFILL_FLAG		64	/* Field is zerofill */
#define BINARY_FLAG		128
/* The following are only sent to new clients */
#define ENUM_FLAG		256	/* field is an enum */
#define AUTO_INCREMENT_FLAG	512	/* field is a autoincrement field */
#define TIMESTAMP_FLAG		1024	/* Field is a timestamp */
#define SET_FLAG		2048	/* field is a set */
#define NUM_FLAG		32768	/* Field is num (for clients) */
#define PART_KEY_FLAG		16384	/* Intern; Part of some key */
#define GROUP_FLAG		32768	/* Intern: Group field */
#define UNIQUE_FLAG		65536	/* Intern: Used by sql_yacc */

#define REFRESH_GRANT		1	/* Refresh grant tables */
#define REFRESH_LOG		2	/* Start on new log file */
#define REFRESH_TABLES		4	/* close all tables */
#define REFRESH_HOSTS		8	/* Flush host cache */
#define REFRESH_STATUS		16	/* Flush status variables */
#define REFRESH_THREADS		32	/* Flush status variables */
#define REFRESH_SLAVE		64	/* Reset master info and restart slave
					   thread */
#define REFRESH_MASTER		128	/* Remove all bin logs in the index
					   and truncate the index */

#define CLIENT_LONG_PASSWORD	1	/* new more secure passwords */
#define CLIENT_FOUND_ROWS	2	/* Found instead of affected rows */
#define CLIENT_LONG_FLAG	4	/* Get all column flags */
#define CLIENT_CONNECT_WITH_DB	8	/* One can specify db on connect */
#define CLIENT_NO_SCHEMA	16	/* Don't allow database.table.column */
#define CLIENT_COMPRESS		32	/* Can use compression protocol */
#define CLIENT_ODBC		64	/* Odbc client */
#define CLIENT_LOCAL_FILES	128	/* Can use LOAD DATA LOCAL */
#define CLIENT_IGNORE_SPACE	256	/* Ignore spaces before '(' */
#define CLIENT_CHANGE_USER	512	/* Support the mysql_change_user() */
#define CLIENT_INTERACTIVE	1024	/* This is an interactive client */
#define CLIENT_SSL		2048	/* Switch to SSL after handshake */
#define CLIENT_IGNORE_SIGPIPE	4096	/* IGNORE sigpipes */
#define CLIENT_TRANSACTIONS	8192	/* Client knows about transactions */

#define MYSQL_ERRMSG_SIZE	200

typedef struct st_net
  {
    struct SSQLPrivate *vio;
    char		last_error[MYSQL_ERRMSG_SIZE];
    unsigned int	last_errno;
    int 		fd;
  } NET;

///////////////////////////////////////////////////////////////////////////////

#define IS_PRI_KEY(n)		((n) & PRI_KEY_FLAG)
#define IS_NOT_NULL(n)		((n) & NOT_NULL_FLAG)
#define IS_BLOB(n)		((n) & BLOB_FLAG)
#define IS_NUM(t)		((t) <= FIELD_TYPE_INT24 || (t) == FIELD_TYPE_YEAR)
#define IS_NUM_FIELD(f)		((f)->flags & NUM_FLAG)
#define INTERNAL_NUM_FIELD(f)	(((f)->type <= FIELD_TYPE_INT24 && ((f)->type != FIELD_TYPE_TIMESTAMP || (f)->length == 14 || (f)->length == 8)) || (f)->type == FIELD_TYPE_YEAR)

#define MYSQL_COUNT_ERROR	(~(my_ulonglong) 0)

#define mysql_reload(mysql)	mysql_refresh((mysql),REFRESH_GRANT)

#define HAVE_MYSQL_REAL_CONNECT

#define FIELD_TYPE_CHAR  FIELD_TYPE_TINY

enum enum_field_types
  {
    FIELD_TYPE_DECIMAL,
    FIELD_TYPE_TINY,
    FIELD_TYPE_SHORT,
    FIELD_TYPE_LONG,
    FIELD_TYPE_FLOAT,
    FIELD_TYPE_DOUBLE,
    FIELD_TYPE_NULL,
    FIELD_TYPE_TIMESTAMP,
    FIELD_TYPE_LONGLONG,
    FIELD_TYPE_INT24,
    FIELD_TYPE_DATE,
    FIELD_TYPE_TIME,
    FIELD_TYPE_DATETIME,
    FIELD_TYPE_YEAR,
    FIELD_TYPE_NEWDATE,
    FIELD_TYPE_ENUM = 247,
    FIELD_TYPE_SET,
    FIELD_TYPE_TINY_BLOB,
    FIELD_TYPE_MEDIUM_BLOB,
    FIELD_TYPE_LONG_BLOB,
    FIELD_TYPE_BLOB,
    FIELD_TYPE_VAR_STRING,
    FIELD_TYPE_STRING
  };

typedef struct
  {
    char *			name;	/* Name of column */
    char *			table;	/* Table of column if column was a field */
    char *			def;	/* Default value (set by mysql_list_fields) */
    enum enum_field_types	type;	/* Type of field. Se mysql_com.h for types */
    unsigned int		length;	/* Width of column */
    unsigned int		max_length;	/* Max width of selected set */
    unsigned int		flags;		/* Div flags */
    unsigned int		decimals;	/* Number of decimals in field */
  } MYSQL_FIELD;

typedef char my_bool;

typedef unsigned long my_ulonglong;

//@ typedef int my_socket;

typedef char **MYSQL_ROW;

typedef unsigned int MYSQL_FIELD_OFFSET;

typedef struct st_mysql_rows
  {
    struct st_mysql_rows *	next;
    MYSQL_ROW			data;
  } MYSQL_ROWS;

typedef MYSQL_ROWS *MYSQL_ROW_OFFSET;

typedef struct st_mysql_data
  {
    my_ulonglong		rows;
    unsigned int		fields;
    MYSQL_ROWS *		data;
//  MEM_ROOT			alloc;
  } MYSQL_DATA;

struct st_mysql_options
  {
    unsigned int		connect_timeout;
    unsigned int		client_flag;
    my_bool			compress;
    my_bool			named_pipe;
    unsigned int		port;
    char *			host;
    char *			init_command;
    char *			user;
    char *			password;
    char *			unix_socket;
    char *			db;
    char *			my_cnf_file;
    char *			my_cnf_group;
    char *			charset_dir;
    char *			charset_name;
    my_bool			use_ssl;
    char *			ssl_key;
    char *			ssl_cert;
    char *			ssl_ca;
    char *			ssl_capath;
  };

enum mysql_option
  {
    MYSQL_OPT_CONNECT_TIMEOUT,
    MYSQL_OPT_COMPRESS,
    MYSQL_OPT_NAMED_PIPE,
    MYSQL_INIT_COMMAND,
    MYSQL_READ_DEFAULT_FILE,
    MYSQL_READ_DEFAULT_GROUP,
    MYSQL_SET_CHARSET_DIR,
    MYSQL_SET_CHARSET_NAME,
    MYSQL_OPT_LOCAL_INFILE
  };

enum mysql_status
  {
    MYSQL_STATUS_READY,
    MYSQL_STATUS_GET_RESULT,
    MYSQL_STATUS_USE_RESULT
  };

typedef struct st_mysql
  {
    NET				net;
//  gptr			connector_fd;
    char *			host;
    char *			user;
    char *			passwd;
    char *			unix_socket;
    char *			server_version;
    char *			host_info;
    char *			info;
    char *			db;
    unsigned int		port;
    unsigned int		client_flag;
    unsigned int		server_capabilities;
    unsigned int		protocol_version;
    unsigned int		field_count;
    unsigned int 		server_status;
    unsigned long		thread_id;
    my_ulonglong		affected_rows;
    my_ulonglong		insert_id;/* id if insert on table w NEXTNR */
    my_ulonglong		extra_info;		/* Used by mysqlshow */
    unsigned long		packet_length;
    enum mysql_status		status;
    MYSQL_FIELD	*		fields;
//  MEM_ROOT			field_alloc;
    my_bool			free_me;
    my_bool			reconnect; /* set to 1 if automatic reconnect */
//  struct st_mysql_options	options;
//  char			scramble_buff[9];
//  struct charset_info_st *	charset;
    unsigned int		server_language;
  } MYSQL;

typedef struct st_mysql_res
  {
    my_ulonglong		row_count;
    unsigned int		field_count;
    unsigned int		current_field;
    MYSQL_FIELD	*		fields;
    MYSQL_DATA *		data;
    MYSQL_ROWS *		data_cursor;
//  MEM_ROOT			field_alloc;
    MYSQL_ROW			row;		/* If unbuffered read */
    MYSQL_ROW			current_row;	/* buffer to current row */
    unsigned long *		lengths;	/* column lengths of current row */
    MYSQL *			handle;		/* for unbuffered reads */
    my_bool			eof;		/* Used my mysql_fetch_row */
  } MYSQL_RES;


/* Functions to get information from the MYSQL and MYSQL_RES structures */
/* Should definitely be used if one uses shared libraries */

#ifdef __cplusplus
extern "C" {
#endif

#if 0
#define load_defaults _fake_load_defaults
#define my_init _fake_my_init
#define my_thread_end _fake_my_thread_end
#define my_thread_init _fake_my_thread_init
#define mysql_affected_rows _fake_mysql_affected_rows
#define mysql_change_user _fake_mysql_change_user
#define mysql_character_set_name _fake_mysql_character_set_name
#define mysql_close _fake_mysql_close
#define mysql_connect _fake_mysql_connect
#define mysql_create_db _fake_mysql_create_db
#define mysql_data_seek _fake_mysql_data_seek
#define mysql_debug _fake_mysql_debug
#define mysql_drop_db _fake_mysql_drop_db
#define mysql_dump_debug_info _fake_mysql_dump_debug_info
#define mysql_eof _fake_mysql_eof
#define mysql_errno _fake_mysql_errno
#define mysql_error _fake_mysql_error
#define mysql_escape_string _fake_mysql_escape_string
#define mysql_fetch_field _fake_mysql_fetch_field
#define mysql_fetch_field_direct _fake_mysql_fetch_field_direct
#define mysql_fetch_fields _fake_mysql_fetch_fields
#define mysql_fetch_lengths _fake_mysql_fetch_lengths
#define mysql_fetch_row _fake_mysql_fetch_row
#define mysql_field_count _fake_mysql_field_count
#define mysql_field_seek _fake_mysql_field_seek
#define mysql_field_tell _fake_mysql_field_tell
#define mysql_free_result _fake_mysql_free_result
#define mysql_get_client_info _fake_mysql_get_client_info
#define mysql_get_host_info _fake_mysql_get_host_info
#define mysql_get_proto_info _fake_mysql_get_proto_info
#define mysql_get_server_info _fake_mysql_get_server_info
#define mysql_info _fake_mysql_info
#define mysql_init _fake_mysql_init
#define mysql_insert_id _fake_mysql_insert_id
#define mysql_kill _fake_mysql_kill
#define mysql_list_dbs _fake_mysql_list_dbs
#define mysql_list_fields _fake_mysql_list_fields
#define mysql_list_processes _fake_mysql_list_processes
#define mysql_list_tables _fake_mysql_list_tables
#define mysql_num_fields _fake_mysql_num_fields
#define mysql_num_rows _fake_mysql_num_rows
#define mysql_odbc_escape_string _fake_mysql_odbc_escape_string
#define mysql_ping _fake_mysql_ping
#define mysql_query _fake_mysql_query
#define mysql_read_query_result _fake_mysql_read_query_result
#define mysql_real_connect _fake_mysql_real_connect
#define mysql_real_escape_string _fake_mysql_real_escape_string
#define mysql_real_query _fake_mysql_real_query
#define mysql_refresh _fake_mysql_refresh
#define mysql_row_seek _fake_mysql_row_seek
#define mysql_row_tell _fake_mysql_row_tell
#define mysql_select_db _fake_mysql_select_db
#define mysql_send_query _fake_mysql_send_query
#define mysql_shutdown _fake_mysql_shutdown
#define mysql_stat _fake_mysql_stat
#define mysql_store_result _fake_mysql_store_result
#define mysql_thread_id _fake_mysql_thread_id
#define mysql_thread_safe _fake_mysql_thread_safe
#define mysql_use_result _fake_mysql_use_result
#endif

extern unsigned long max_allowed_packet;
extern unsigned long net_buffer_length;

my_ulonglong mysql_num_rows (MYSQL_RES * res);

unsigned int mysql_num_fields (MYSQL_RES * res);

my_bool mysql_eof (MYSQL_RES * res);

MYSQL_FIELD *mysql_fetch_field_direct (MYSQL_RES * res,
    unsigned int fieldnr);

MYSQL_FIELD *mysql_fetch_fields (MYSQL_RES * res);

MYSQL_ROWS *mysql_row_tell (MYSQL_RES * res);

unsigned int mysql_field_tell (MYSQL_RES * res);

unsigned int mysql_field_count (MYSQL * mysql);

my_ulonglong mysql_affected_rows (MYSQL * mysql);

my_ulonglong mysql_insert_id (MYSQL * mysql);

unsigned int mysql_errno (MYSQL * mysql);

char *mysql_error (MYSQL * mysql);

char *mysql_info (MYSQL * mysql);

unsigned long mysql_thread_id (MYSQL * mysql);

const char *mysql_character_set_name (MYSQL * mysql);

MYSQL *mysql_init (MYSQL * mysql);

MYSQL *mysql_connect (MYSQL * mysql, const char *host,
    const char *user, const char *passwd);

my_bool mysql_change_user (MYSQL * mysql, const char *user,
    const char *passwd, const char *db);

MYSQL *mysql_real_connect (MYSQL * mysql, const char *host,
    const char *user, const char *passwd, const char *db,
    unsigned int port, const char *unix_socket, unsigned int clientflag);

void mysql_close (MYSQL * sock);

int mysql_select_db (MYSQL * mysql, const char *db);

int mysql_query (MYSQL * mysql, const char *q);

int mysql_send_query (MYSQL * mysql, const char *q,
    unsigned int length);

int mysql_read_query_result (MYSQL * mysql);

int mysql_real_query (MYSQL * mysql, const char *q,
    unsigned int length);

int mysql_create_db (MYSQL * mysql, const char *DB);

int mysql_drop_db (MYSQL * mysql, const char *DB);

int mysql_shutdown (MYSQL * mysql);

int mysql_dump_debug_info (MYSQL * mysql);

int mysql_refresh (MYSQL * mysql, unsigned int refresh_options);

int mysql_kill (MYSQL * mysql, unsigned long pid);

int mysql_ping (MYSQL * mysql);

char *mysql_stat (MYSQL * mysql);

char *mysql_get_server_info (MYSQL * mysql);

char *mysql_get_client_info (void);

char *mysql_get_host_info (MYSQL * mysql);

unsigned int mysql_get_proto_info (MYSQL * mysql);

MYSQL_RES *mysql_list_dbs (MYSQL * mysql, const char *wild);

MYSQL_RES *mysql_list_tables (MYSQL * mysql, const char *wild);

MYSQL_RES *mysql_list_fields (MYSQL * mysql, const char *table,
    const char *wild);

MYSQL_RES *mysql_list_processes (MYSQL * mysql);

MYSQL_RES *mysql_store_result (MYSQL * mysql);

MYSQL_RES *mysql_use_result (MYSQL * mysql);

int mysql_options (MYSQL * mysql, enum mysql_option option,
    const char *arg);

void mysql_free_result (MYSQL_RES * result);

void mysql_data_seek (MYSQL_RES * result, my_ulonglong offset);

MYSQL_ROW_OFFSET mysql_row_seek (MYSQL_RES * result,
    MYSQL_ROW_OFFSET);

MYSQL_FIELD_OFFSET mysql_field_seek (MYSQL_RES * result,
    MYSQL_FIELD_OFFSET offset);

MYSQL_ROW mysql_fetch_row (MYSQL_RES * result);

unsigned long *mysql_fetch_lengths (MYSQL_RES * result);

MYSQL_FIELD *mysql_fetch_field (MYSQL_RES * result);

unsigned long mysql_escape_string (char *to, const char *from,
    unsigned long from_length);

unsigned long mysql_real_escape_string (MYSQL * mysql,
    char *to, const char *from, unsigned long length);

void mysql_debug (const char *debug);

char *mysql_odbc_escape_string (MYSQL * mysql,
    char *to, unsigned long to_length, const char *from,
    unsigned long from_length, void *param,
    char *(*extend_buffer) (void *, char *to, unsigned long *length));

void myodbc_remove_escape (MYSQL * mysql, char *name);

unsigned int mysql_thread_safe (void);

void my_init (void);

void load_defaults (const char *conf_file, const char **groups, int *argc,
    char ***argv);

my_bool my_thread_init (void);

void my_thread_end (void);

//@
char *get_tty_password (char *opt_message);
#ifdef __cplusplus
};
#endif

#endif
