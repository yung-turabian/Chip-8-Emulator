/* Copyright 2025 Punchline Software

*/

#ifndef PUNCHLINE_LOG_H
#define PUNCHLINE_LOG_H

#include <fcntl.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define STDOUT_FILENO 1
#define STDERR_FILENO 2

enum LogType {
		LINFO,
		LTRACE,
		LDEBUG,
		LERROR,
		LFATAL
};

#define FATAL_COLOR "\x1b[31m"
#define TRACE_COLOR "\x1b[32m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_BLUE "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN "\x1b[36m"
#define ANSI_WHITE "\x1b[37m"
#define ANSI_RESET "\x1b[0m" // ANSI reset needed to be called after color change

// Settings
static int log_fd = STDOUT_FILENO;
static bool show_date = false;

static inline void LogInit( const char *log_file_name ) {
#if defined(DEBUG)
		log_fd = STDOUT_FILENO;
#else
		char buf[512] = {'\0'};
		strncpy( buf, log_file_name, strlen(log_file_name) );
		strncat( buf, ".log", 4 );
		log_fd = open( log_file_name, O_WRONLY, 644 );
#endif
}

static inline void Log( enum LogType typ, const char *fmt, ... ) {
		char buf[512] = {'\0'};
		char color[16];
		char label[6];
		switch ( typ ) {
				case LTRACE:
						strcpy( color, TRACE_COLOR );
						strncpy( label, "TRACE", 6 );
				break;

				default:
						strcpy( color, ANSI_WHITE );
				break;
		}
		snprintf( buf, 
						  strlen(color) + strlen(label) + (sizeof(char) * 6) + 
							strlen(fmt) + strlen(ANSI_RESET), 
							"%s[TRACE]%s: %s\n", TRACE_COLOR, ANSI_RESET, fmt ); 
		va_list vargs;
		va_start( vargs, fmt );
		vdprintf( log_fd, buf, vargs );
		va_end( vargs );
}

#endif /* PUNCHLINE_LOG_H */
