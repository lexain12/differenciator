#include <time.h>
#include <stdlib.h>
#include "LogLib.h"

tm Tm = {};

int logprint(char param)
{
    fprintf(LOGFILEPTR, "%c", param);

    return 0;
}

int logprint(int param)
{
    fprintf(LOGFILEPTR, "%d", param);

    return 0;
}

int logprint(char* param)
{
    fprintf(LOGFILEPTR, "%s", param);

    return 0;
}

int logprint(double param)
{
    fprintf(LOGFILEPTR, "%lg", param);

    return 0;
}

FILE* logOpen(const char* logFileName)
{
    FILE* filePtr = fopen(logFileName, "w");
    setvbuf(filePtr, NULL, _IONBF, 0);

    const time_t timer = time(NULL);
    fprintf(filePtr, "---------------%s", strtok(ctime(&timer), "\n"));
    fseek(filePtr, -1, SEEK_CUR);
    fprintf(filePtr, "---------------\n");
    fprintf(filePtr, "<pre>\n");

    return filePtr;
}

