#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <cstring>

#ifndef LOGNAME
#define LOGNAME "logfile.html"
#endif

int logprint(char param);
int logprint(int param);
int logprint(char* param);
int logprint(double param);

FILE* logOpen(const char*);

FILE* LOGFILEPTR = logOpen(LOGNAME);

#define $ fprintf(LOGFILEPTR, "FILE: %s, Function: %s, line: %d\n", __FILE__, __func__, __LINE__);

#define $$(param) param; logprint(#param); logprint(param); logprint('\n'); \

