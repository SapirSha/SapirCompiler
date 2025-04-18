#include "ConsoleOut.h"
#include "stdio.h"

void console_out(char* msg) {
	fprintf(stdout, "%s", msg);
}