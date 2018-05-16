#ifndef __STUB_H__
#define __STUB_H__

#ifdef __cplusplus
exern "C"
{
#endif

#define CODESIZE 5U

struct func_stub
{
    void *fn;
    unsigned char code_buf[CODESIZE];
};

void stub_set(struct func_stub *pstub, void *fn, void *fn_stub);
void stub_reset(struct func_stub *pstub);
int stub_init(void);

#ifdef __cplusplus
}
#endif

#endif

