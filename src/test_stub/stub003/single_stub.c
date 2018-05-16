#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

#include "single_stub.h"

BYTE g_apiBackup[FLATJMPCODE_LENGTH+FLATJMPCMD_LENGTH];

BOOL single_setStub(LPVOID ApiFun,LPVOID HookFun)
{
    BOOL    IsSuccess = FALSE;
    DWORD   TempProtectVar;              //暂时保护属性变量
    MEMORY_BASIC_INFORMATION MemInfo;    //内存分页属性信息
    
    VirtualQuery(ApiFun,&MemInfo,sizeof(MEMORY_BASIC_INFORMATION));
    
    if(VirtualProtect(MemInfo.BaseAddress,MemInfo.RegionSize, PAGE_EXECUTE_READWRITE,&MemInfo.Protect))  //改动页面为可写
    {
        memcpy((void*)g_apiBackup,(const void*)ApiFun, sizeof(g_apiBackup));

        *(BYTE*)ApiFun = FLATJMPCMD;                                 //拦截API，在函数代码段前面注入jmp xxx
        *(DWORD*)((BYTE*)ApiFun + FLATJMPCMD_LENGTH) = (DWORD)HookFun -
            (DWORD)ApiFun - FLATJMPCODE_LENGTH;
        
        VirtualProtect(MemInfo.BaseAddress,MemInfo.RegionSize,
            MemInfo.Protect,&TempProtectVar);                        //改回原属性
        
        IsSuccess = TRUE;
    }
    
    return IsSuccess;
}

BOOL single_clearStub(LPVOID ApiFun)
{
    BOOL    IsSuccess = FALSE;
    DWORD   TempProtectVar;              //暂时保护属性变量
    MEMORY_BASIC_INFORMATION MemInfo;    //内存分页属性信息
    
    VirtualQuery(ApiFun,&MemInfo,sizeof(MEMORY_BASIC_INFORMATION));
    
    if(VirtualProtect(MemInfo.BaseAddress,MemInfo.RegionSize, PAGE_EXECUTE_READWRITE,&MemInfo.Protect))                            //改动页面为可写
    {
        memcpy((void*)ApiFun, (const void*)g_apiBackup, sizeof(g_apiBackup));  //恢复代码段
        
        VirtualProtect(MemInfo.BaseAddress,MemInfo.RegionSize,
            MemInfo.Protect,&TempProtectVar);                        //改回原属性
        
        IsSuccess = TRUE;
    }
    
    return IsSuccess;
}

/*
this is a demo code from others, that show the basic theory how to sync-modify a function behavior: add a asm code 0xE9, which can jmp to stub function

	void *pfun = (void *)a;
	DWORD page,tmp,pagesize;
	SYSTEM_INFO sysInfo;
	int ret;

	a();
	
	GetSystemInfo(&sysInfo);
	pagesize = sysInfo.dwPageSize;
	page =(DWORD)a & ~(pagesize-1);      //a函数所在页头

	ret = VirtualProtect((LPVOID)page, pagesize, PAGE_EXECUTE_READWRITE,&tmp);
	printf("ret is %d\n", ret);

	*(char *)pfun = 0xe9;               //汇编跳转命令
	*(DWORD *)((char *)pfun+1) = (DWORD)b-(DWORD)a-5;     //跳转的长度，确保跳到b函数地址

*/