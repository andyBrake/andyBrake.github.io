#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#include "log.h"


#define LOG_FILE "./test.log"
static FILE *pLogFile = NULL;


void log_int()
{
    pLogFile = fopen(LOG_FILE, "w+");
    fprintf(pLogFile, "\n=================LOG FILE Start=============\n");
}

void logRecord(char *strFunc, int line, char *fmt, ...)
{
    char tmp_buf[1024];
    char tag_buf[1024];
    unsigned int sz = 0;
    va_list         args;

    va_start(args, fmt);

    sz = vsprintf(tmp_buf, fmt, args);
    if ('\n' == tmp_buf[sz - 1])
    {
        tmp_buf[--sz] = '\0';
    }
    sz += sprintf(tag_buf, " [%s, %u][ %d ]", strFunc, line, getpid());

    strcat(tmp_buf, tag_buf);

    va_end(args);

    printf("\n%s\n", tmp_buf);

    if (NULL == pLogFile)
    {
        printf("\n\nOPEN log ERROR!!!\n\n");
        return;
    }
    fprintf(pLogFile, "%s\n", tmp_buf);

    return;
}

void log_close()
{
    if (pLogFile)
    {
        fprintf(pLogFile, "\n=================LOG FILE CLOSE NORMAL=============\n");
        fclose(pLogFile);
    }
    return;
}


int main()
{
    int i =10;

    log_int();

    Debug_Log(cMajor, cLogInfo, ("Hello, %u\n", i));

    log_close();

    return 0;
}