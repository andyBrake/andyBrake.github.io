



#define FLATJMPCODE_LENGTH 5            //x86 平坦内存模式下，绝对跳转指令长度
#define FLATJMPCMD_LENGTH  1            //机械码0xe9长度
#define FLATJMPCMD         0xe9         //相应汇编的jmp指令

BOOL single_setStub(LPVOID ApiFun,LPVOID HookFun);
BOOL single_clearStub(LPVOID ApiFun);