



#define FLATJMPCODE_LENGTH 5            //x86 ƽ̹�ڴ�ģʽ�£�������תָ���
#define FLATJMPCMD_LENGTH  1            //��е��0xe9����
#define FLATJMPCMD         0xe9         //��Ӧ����jmpָ��

BOOL single_setStub(LPVOID ApiFun,LPVOID HookFun);
BOOL single_clearStub(LPVOID ApiFun);