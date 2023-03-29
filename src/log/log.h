#include <stdio.h>

#if  1 // enable log file
extern void logRecord(char *strFunc, int iLine, char *fmt, ...);

#define LOG(cFormat, arg...) \
    logRecord((char *)__FUNCTION__, __LINE__, cFormat, ##arg)

#define _DEBUG_PRINTF               LOG
#else
#define _DEBUG_PRINTF               printf
#endif

#define Debug_Log(Category, Level, Message) do  \
{                                               \
	if (1)                     \
	{                                           \
		_DEBUG_PRINTF Message ;                 \
	}                                           \
}while(0)

