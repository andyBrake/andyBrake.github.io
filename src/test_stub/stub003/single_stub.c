#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

#include "single_stub.h"

BYTE g_apiBackup[FLATJMPCODE_LENGTH+FLATJMPCMD_LENGTH];

BOOL single_setStub(LPVOID ApiFun,LPVOID HookFun)
{
    BOOL    IsSuccess = FALSE;
    DWORD   TempProtectVar;              //��ʱ�������Ա���
    MEMORY_BASIC_INFORMATION MemInfo;    //�ڴ��ҳ������Ϣ
    
    VirtualQuery(ApiFun,&MemInfo,sizeof(MEMORY_BASIC_INFORMATION));
    
    if(VirtualProtect(MemInfo.BaseAddress,MemInfo.RegionSize, PAGE_EXECUTE_READWRITE,&MemInfo.Protect))  //�Ķ�ҳ��Ϊ��д
    {
        memcpy((void*)g_apiBackup,(const void*)ApiFun, sizeof(g_apiBackup));

        *(BYTE*)ApiFun = FLATJMPCMD;                                 //����API���ں��������ǰ��ע��jmp xxx
        *(DWORD*)((BYTE*)ApiFun + FLATJMPCMD_LENGTH) = (DWORD)HookFun -
            (DWORD)ApiFun - FLATJMPCODE_LENGTH;
        
        VirtualProtect(MemInfo.BaseAddress,MemInfo.RegionSize,
            MemInfo.Protect,&TempProtectVar);                        //�Ļ�ԭ����
        
        IsSuccess = TRUE;
    }
    
    return IsSuccess;
}

BOOL single_clearStub(LPVOID ApiFun)
{
    BOOL    IsSuccess = FALSE;
    DWORD   TempProtectVar;              //��ʱ�������Ա���
    MEMORY_BASIC_INFORMATION MemInfo;    //�ڴ��ҳ������Ϣ
    
    VirtualQuery(ApiFun,&MemInfo,sizeof(MEMORY_BASIC_INFORMATION));
    
    if(VirtualProtect(MemInfo.BaseAddress,MemInfo.RegionSize, PAGE_EXECUTE_READWRITE,&MemInfo.Protect))                            //�Ķ�ҳ��Ϊ��д
    {
        memcpy((void*)ApiFun, (const void*)g_apiBackup, sizeof(g_apiBackup));  //�ָ������
        
        VirtualProtect(MemInfo.BaseAddress,MemInfo.RegionSize,
            MemInfo.Protect,&TempProtectVar);                        //�Ļ�ԭ����
        
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
	page =(DWORD)a & ~(pagesize-1);      //a��������ҳͷ

	ret = VirtualProtect((LPVOID)page, pagesize, PAGE_EXECUTE_READWRITE,&tmp);
	printf("ret is %d\n", ret);

	*(char *)pfun = 0xe9;               //�����ת����
	*(DWORD *)((char *)pfun+1) = (DWORD)b-(DWORD)a-5;     //��ת�ĳ��ȣ�ȷ������b������ַ

*/