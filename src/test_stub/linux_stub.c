
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
 
 
#define JMP_OFFSET_LEN  5   //JMP指令的长度
 
//JMP相对跳转
void install_stub(void* src_func, void* dst_func)
{
    int pagesize = sysconf(_SC_PAGESIZE);                                          // 系统页大小
    unsigned long srcpage = (unsigned long)((unsigned long)src_func & 0xFFFFF000); // 计算原函数地址所在的页 的首地址
    mprotect((void*)srcpage, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC);        // 使用mprotect函数使该页的内存可读可写可执行
    unsigned char jmpcmd[JMP_OFFSET_LEN] = {0};
    unsigned int offset = (unsigned long)dst_func - (unsigned long)src_func - JMP_OFFSET_LEN;
    jmpcmd[0] = 0xE9;                                                              // JMP指令
    memcpy(&jmpcmd[1], &offset, sizeof(offset));                                   // 偏移
    memcpy(src_func, jmpcmd, JMP_OFFSET_LEN);                                      // 将原函数的地址替换为JMP指令 跳转到目的函数
}
 
//JMP绝对跳转
void install_stub2(void* src_func, void* dst_func)
{
    int pagesize = sysconf(_SC_PAGESIZE);                                          // 系统页大小
    unsigned long srcpage = (unsigned long)((unsigned long)src_func & 0xFFFFF000); // 计算原函数地址所在的页 的首地址
    mprotect((void*)srcpage, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC);        // 使用mprotect函数使该页的内存可读可写可执行
    unsigned char jmpcmd[14] = {0};                                                // JMP远跳只支持32位程序 64位程序地址占8个字节 寻址有问题
    jmpcmd[0] = 0xFF;                                                              // 当JMP指令为 FF 25 00 00 00 00时，会取下面的8个字节作为跳转地址
    jmpcmd[1] = 0x25;                                                              // 因此可以使用14个字节作为指令 (FF 25 00 00 00 00) + dstaddr
    jmpcmd[2] = 0x00;                                                              
    jmpcmd[3] = 0x00;
    jmpcmd[4] = 0x00;
    jmpcmd[5] = 0x00;
    unsigned long dstaddr = (unsigned long)dst_func;
    memcpy(&jmpcmd[6], &dstaddr, sizeof(dstaddr));                                 
    memcpy(src_func, jmpcmd, sizeof(jmpcmd));   
}
 
void test_funcA()
{
    printf("call funcA\n");
}
 
void test_funcB()
{
    printf("call funcB\n");
}
 
int main()
{
    install_stub2((void*)test_funcA, (void*)test_funcB);  // 添加动态桩 用B替换A
    test_funcA();                                        // 执行A函数
    return 0;

}