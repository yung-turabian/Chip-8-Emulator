/* Copyright 2025 Punchline Software

*/

#ifndef PUNCHLINE_LOG_H
#define PUNCHLINE_LOG_H

enum LogType {
		LINFO,
		LTRACE,
		LDEBUG,
		LERROR,
		LFATAL
};

void LogInit( const char *log_file_name );
void Log( enum LogType typ, const char *fmt, ... );
void LogFree( void );

#endif /* PUNCHLINE_LOG_H */

// Define this once, top-level
#ifdef YUNGLOG_IMPLEMENTATION

#include <fcntl.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define ERROR_COLOR "\x1b[31m"
#define TRACE_COLOR "\x1b[32m"
#define DEBUG_YELLOW "\x1b[33m"
#define FATAL_COLOR "\x1b[35m"
#define INFO_COLOR "\x1b[36m"
#define ANSI_WHITE "\x1b[37m"
#define ANSI_RESET "\x1b[0m" // ANSI reset needed to be called after color change
#define ANSI_COLOR_SIZE 9
#define ANSI_RESET_SIZE 8

// Settings
static int log_fd = -1;
static bool show_date = false;
static time_t curr_time;

void LogFree( void ) {
		if ( log_fd != -1 ) {
				close( log_fd );
		}
}

void LogInit( const char *log_file_name ) {
		if ( log_file_name != NULL ) {
				char buf[512] = {'\0'};
				strncpy( buf, log_file_name, strlen(log_file_name) );
				strncat( buf, ".log", 5 );
				log_fd = open( buf, O_CREAT | O_WRONLY, 0644 );
				if ( log_fd == -1 ) {
						fprintf( stderr, "Failed to open a file descriptor for logger.\n" );
						exit( 1 );
				}

				/*if ( dup2(log_fd, STDERR_FILENO) == -1 ) {
						fprintf( stderr, "Failed to redirect file descriptor.\n" );
						exit( 1 );
				}*/

				curr_time = time( NULL );

				dprintf( log_fd, "(%d) yung logger began on %s\n", log_fd, 
								 ctime(&curr_time) );
		} else {
				log_fd = STDOUT_FILENO;
		}
		//curr_time = time( NULL );
		//printf( "%s", ctime(&curr_time) );


		int i = atexit( LogFree );
		if ( i != 0 ) {
				fprintf( stderr, "Cannot set exit function\n" );
				exit( 1 );
		}
}

void Log( enum LogType typ, const char *fmt, ... ) {
		if ( log_fd == -1 ) { return; }

		char buf[512] = {'\0'};
		char label[6];
		char color[16] = {'\0'};
		char reset[ANSI_RESET_SIZE] = {'\0'};
		if ( log_fd == STDOUT_FILENO ) {
				switch ( typ ) {
						case LTRACE:
								strcpy( color, TRACE_COLOR );
						break;
						case LINFO:
								strcpy( color, INFO_COLOR );
						break;
						default:
								strcpy( color, ANSI_WHITE );
						break;
				}

				strncpy( reset, ANSI_RESET, ANSI_RESET_SIZE );
		}

		switch ( typ ) {
				case LTRACE:
						strncpy( label, "TRACE", 6 );
				break;
				case LINFO:
						strncpy( label, "INFO", 5 );
				break;
				case LDEBUG:
						strncpy( label, "DEBUG", 6 );
				break;
				default:
						strncpy( label, "?", 2 );
				break;
		}



		snprintf( buf, 
						  ( strlen(color) + strlen(reset) + strlen(label) + 
								strlen(fmt) + 6 ) * sizeof(char), 
							"%s[%s]%s: %s\n", color, label, reset, fmt ); 
		va_list vargs;
		va_start( vargs, fmt );
		vdprintf( log_fd, buf, vargs );
		va_end( vargs );
}

#endif
