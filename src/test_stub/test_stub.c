// test_stub.cpp : 定义控制台应用程序的入口点。
//
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

#include "stub002/d_stub.h"
#include "stub003/single_stub.h"


void a(void)
{
	printf("i am a\n");
}

void b(void)
{
	printf("i am b\n");
}

int a_plus(int a, int b)
{
	return a+b;
}

int b_plus(int a, int b)
{
	return a-b;
}

/* test single stub */
#if 0
int main()
{
	int ret;
	int n1 = 5, n2 = 10;

	a();

	single_setStub(a, b);

	a();

	single_clearStub(a);

	a();

	ret = a_plus(n1, n2);
	printf("before stub ret is %d \n", ret);

	single_setStub(a_plus, b_plus);
	ret = a_plus(n1, n2);
	printf("after stub ret is %d \n", ret);

	single_clearStub(a_plus);
	ret = a_plus(n1, n2);
	printf("clear stub ret is %d \n", ret);
 

	printf("end");
	system("pause");

	return 0;
}
#endif
/* end test single stub */


int main()
{
	int n1 = 5, n2 = 10, ret;
	stubInfo si, si_plus;

	a();
	ret = a_plus(n1, n2);
	printf("ret is %d\n", ret );
	
	setStub(a, b, &si);
	setStub(a_plus, b_plus, &si_plus);
	printf("\n");
	
	a();
	ret = a_plus(n1, n2);
	ret = a_plus(n1, n2);
	printf("ret is %d\n", ret );

	cleanStub(&si);
	cleanStub(&si_plus);
	printf("\n");

	a();
	ret = a_plus(n1, n2);
	printf("ret is %d\n", ret );

	printf("\ntest end\n");
	system("pause");

	return 0;
}



#if 0
int main()
{
	stubInfo si;
	

	//setStub ((void *)t0, (void *)t1, &si);

	setStub_02(t0, t1);

	t0();

	return 0;
}

#endif