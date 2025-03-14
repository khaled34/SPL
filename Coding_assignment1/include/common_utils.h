#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


#define FALSE   0
#define TRUE    1

typedef enum logLevel_e_
{
    LOG_ERR,
    LOG_WARN,
    LOG_DEBUG
} logLevel_e;

void log_and_handle(logLevel_e logLevel, const char* format, ...) 
{
    va_list args;
    va_start(args, format);

#ifndef SYSLOG 
    vprintf(format, args);
#else
    /* TODO: Add different implementation for logging later */    
#endif
    
    va_end(args);
    switch(logLevel)
    {
        case LOG_ERR:
            exit(-EXIT_FAILURE);
            break;
        case LOG_WARN: 
        //Fallthrough
        case LOG_DEBUG:
        //Fallthrough
        default:
            break;
    }
}
#endif 