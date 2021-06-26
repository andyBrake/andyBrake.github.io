#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>

#else
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#endif

#include "d_stub.h"

#define JUMPCODE_MAX 0x05 //0x05
#define JUMPCODE_CMD 0xE9
#define JUMPCODE_RET 0xC3

#define DBG (0)

static int checkJumpCode(void *funcAddr)
{
    int i;
    int ret = 0;
    for (i = 0; i < JUMPCODE_MAX - 1; i++)
    {
        if (JUMPCODE_RET == ((unsigned char *)funcAddr)[i])
        {
            ret = -1;
            break;
        }
    }
    if (0 != ret)
    {
        printf("Warning: This function cannot be setStub!\n");
    }
    return ret;
}

static void setJumpCode(void *codeAddr, char jumpCode[JUMPCODE_MAX])
{
#ifdef _WIN32
    DWORD dwOldProtect;
    (void)VirtualProtect(codeAddr, JUMPCODE_MAX, PAGE_EXECUTE_READWRITE, &dwOldProtect);
    memcpy(codeAddr, jumpCode, JUMPCODE_MAX);
    (void)VirtualProtect(codeAddr, JUMPCODE_MAX, dwOldProtect, &dwOldProtect);
#else
    codeAddr = (void *)((long)codeAddr & 0xffffffff); //workaround for 64-bit machine
    int pageSize = sysconf(_SC_PAGE_SIZE);
    void *startAddr = (void *)codeAddr;
    void *endAddr = (void *)((unsigned long)startAddr + JUMPCODE_MAX - 1);
    void *pageAddr = (void *)((unsigned long)startAddr & (~(pageSize - 1)));
    int pageNum = ((unsigned long)endAddr - (unsigned long)pageAddr) / pageSize + 2;
    int ret = 0;

#if DBG
    printf("set mpotect, start addr 0x%p page size %u Byte, page number %u\n",
           pageAddr, pageSize, pageNum);
#endif
    ret = mprotect(pageAddr, pageNum * pageSize, PROT_READ | PROT_WRITE | PROT_EXEC);
    if (ret != 0)
    {
        printf("mprotect ret %d, failed\n", ret);
    }

    memcpy(codeAddr, jumpCode, JUMPCODE_MAX);
#if DBG
    printf("cpy done\n");
#endif

#endif
}

/*****************************************************
 @Description:
   Set dynamic stub.
 @Parameter:
   funcAddr : the function which want to be replaced.
   stubAddr : stub function.
   si       : stub information.
 *****************************************************/
void setStub(void *funcAddr, void *stubAddr, stubInfo *si)
{
    char jumpCode[14] = {JUMPCODE_CMD};                                                    //JUMPCODE_MAX
    unsigned long dist = (unsigned long)stubAddr - (unsigned long)funcAddr - JUMPCODE_MAX; // get the offset

    assert(NULL != funcAddr);
    assert(NULL != stubAddr);
    assert(NULL != si);

    memset(si, 0, sizeof(stubInfo));

    (void)checkJumpCode(funcAddr);

    memcpy((void *)&jumpCode[1], (void *)&dist, sizeof(void *));
    memcpy((void *)&si->info[0], (void *)funcAddr, JUMPCODE_MAX);
    memcpy((void *)&si->info[JUMPCODE_MAX], (void *)&funcAddr, sizeof(void *));
#if DBG
    printf("start to set jump code\n");
#endif
    jumpCode[0] = JUMPCODE_CMD;

    memcpy(&jumpCode[6], (void *)&dist, sizeof(void *));

    setJumpCode(funcAddr, jumpCode);
}

/******************************************************
 @Description:
   Clearn stub.
 @Parameter:
   si       : stub information be generated by setStub.
 *****************************************************/
void cleanStub(stubInfo *si)
{
    char jumpCode[JUMPCODE_MAX];
    void *funcAddr;

    assert(NULL != si);

    memcpy((void *)&jumpCode, (void *)&si->info[0], JUMPCODE_MAX);
    memcpy((void *)&funcAddr, (void *)&si->info[JUMPCODE_MAX], sizeof(void *));

    if (NULL == funcAddr)
    {
        return;
    }
    setJumpCode(funcAddr, jumpCode);
}
