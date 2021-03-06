#ifndef __D_STUB_H__
#define __D_STUB_H__

#ifdef __cplusplus
extern "C" {
#endif

#define STUBINFO_MAX    0x9

#pragma pack(1)

typedef struct stubInfo {
	char info[STUBINFO_MAX];
} stubInfo;

#pragma pack()

/*****************************************************
 @Description:
   set dynamic stub.
 @Parameter:
   funcAddr : the function which want to be replaced.
   stubAddr : stub function.
   si       : stub information.
 *****************************************************/
extern void setStub (void * funcAddr, void * stubAddr, stubInfo * si);

/******************************************************
 @Description:
   clearn stub.
 @Parameter:
   si       : stub information be generated by setStub.
 *****************************************************/
extern void cleanStub (stubInfo * si);

#ifdef __cplusplus
}
#endif

#endif /* __D_STUB_H__ */

