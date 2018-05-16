//#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <limits.h>
//#include <sys/mman.h>

#include "stub.h"

static long pagesize = -1;

static inline void *pageof(const void* p)
{ 
	return (void *)((unsigned long)p & ~(pagesize - 1));
}

void stub_set(struct func_stub *pstub, void *fn, void *fn_stub)
{	
    pstub->fn = fn;
    memcpy(pstub->code_buf, fn, CODESIZE);
    
    if (-1 == mprotect(pageof(fn), pagesize * 2, PROT_READ | PROT_WRITE | PROT_EXEC))
    {
        perror("mprotect to w+r+x faild");
        exit(errno);
    }

	*(unsigned char *)fn = (unsigned char)0xE9;
    *(unsigned int *)((unsigned char *)fn + 1) = (unsigned char *)fn_stub - (unsigned char *)fn - CODESIZE;
    
    if (-1 == mprotect(pageof(fn), pagesize * 2, PROT_READ | PROT_EXEC))
    {
        perror("mprotect to r+x failed");
        exit(errno);
    }
    
    return;
}

void stub_reset(struct func_stub *pstub)
{	
    if (NULL == pstub->fn)
    {
		return;
    }
    
    if (-1 == mprotect(pageof(pstub->fn), pagesize * 2, PROT_READ | PROT_WRITE | PROT_EXEC))
    {
        perror("mprotect to w+r+x faild");
        exit(errno);
    }

	memcpy(pstub->fn, pstub->code_buf, CODESIZE);
    
    if (-1 == mprotect(pageof(pstub->fn), pagesize * 2, PROT_READ | PROT_EXEC))
    {
        perror("mprotect to r+x failed");
        exit(errno);
    }

	memset(pstub, 0, sizeof(struct func_stub));
	
    return;
}

int stub_init(void)
{
	int ret;

	pagesize = sysconf(_SC_PAGE_SIZE);

	ret = 0;
	if (pagesize < 0)
	{
		perror("get system _SC_PAGE_SIZE configure failed");
		ret = -1;
	}
	
	return ret;
}

