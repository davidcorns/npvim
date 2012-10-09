#include <debug.h>
#include <stdio.h>
#include <stdarg.h>

namespace debug {
	
FILE *g_debugLogFile = fopen ("debug.log","a");

void log(char* msg, ...) {
	va_list args;
	va_start(args, msg);
	vfprintf(g_debugLogFile, msg, args);
	va_end(msg);
	
	fprintf(g_debugLogFile, "\n");
	fflush(g_debugLogFile);
}

}	//namespace debug;
