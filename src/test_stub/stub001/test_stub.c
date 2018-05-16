#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stub.h"

void f1()
{
    printf("f1\n");
    
    return;
}

void f2()
{
    printf("f2\n");
    
    return;
}

void *_memset(void *s, int ch, size_t n)
{
    printf("-memset\n");
    
    return s;
}

int main()
{
    char ac[10] = {1};
    struct func_stub stub;

	if (-1 == stub_init())
	{
		printf("faild to init stub\n");
		return 0;
	}
	
    stub_set(&stub, (void *)memset, (void *)_memset);

    memset(ac, 0, 10);

	stub_reset(&stub);
	
    memset(ac, 0, 10);
	printf("ac[0] = %hhu\n", ac[0]);
	
	stub_set(&stub, (void *)f1, (void *)f2);

	f1();
	
	stub_reset(&stub);

	f1();
	
	return 0;
}
