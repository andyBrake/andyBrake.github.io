# volatile 学习总结

volatile 目的是告诉编译器，这个变量是易变的，就是可能被其他线程，中断或者是硬件设备修改。编译器在编译volatile修饰的变量时不要进行优化，读操作每次都应该从memroy中原始位置获取出这个变量的值，而不能使用cpu的片内cache或者寄存器中缓存的值；写的时候，编译器不能优化掉中间写过程。

## 读的例子

```
int g_flag = 0;

void func ()
{
    while (g_flag == 0)
    {
         //do something
    }
}

or 

void func2 ()
{
    if (0 == g_flag)
    {
        //do something 
    }

    if (1 == g_flag)
    {
        //do something 
    }

    if (2 == g_flag)
    {
        //do something 
    }
}

void func3 ()
{
    if (1 == g_flag)
    {
         //do something
    }
}
```

上面两个函数中，第一个函数一直读`g_flag`判断，编译器可能优化的结果是第一次从memory中读取到寄存器中，后面的操作都是从寄存器中取值判断，而`g_flag`的修改在另外一个线程或者中断中发生了，那么func 中的while判断一直为真，死循环等待了。同样的，第二个函数func2中，第一次读取`g_flag`用于判断，后面的两个判断可能被编译器优化为从寄存器中读取值，而不是从memory中读取，导致后面的逻辑出错。而func3则没有问题，因为只有一次读取操作，这个函数内后续没有再读的操作，编译器没有优化的可能。

---

## 写的例子
```
#define  CONF_REG *(uint32_t *)(0x00AAC0)   /* 假设这是配置寄存器地址 */

void initConfReg ()
{
    CONF_REG = 0x0;
    CONF_REG = 0x1;
    CONF_REG = 0x2;
    CONF_REG = 0x3;
}
```
上面的操作，编译器可能优化掉`CONF_REG`前面的三次赋值，只留下最后一次赋值0x3的操作，这对于硬件配置而言会导致配置流程失败。


# 结构体加volatile
如果一个全局变量是一个struct 或者class，加上volatile的修饰后，其内部所有成员相当于都加了volatile修饰。如果一个全局变量加上volatile修饰，可能导致编译的时候产生大量类型不匹配的告警，需要谨慎对待，判断是否真的需要对整个结构都加上volatile修饰。
很多时候，全局结构都是有锁，或者其他互斥机制保护的，当操作全局结构时，是不可能有另外一个线程修改这个全局变量的。所以，如果一个函数中，在锁内操作全局结构，那么是不需要再额外加volatile修饰的。但是如果在锁外，或者跨了锁保护操作则有可能需要处理
比如下面这种情况：

```
bool g_flag;

void func ()
{
	LOCK();
	
	if (g_flag)
	{
		//do something
	}
	
	UNLOCK();
	
	//do something without hold lock
	
	LOCK()
	
	if (g_flag)
	{}
	
	UNLOCK();
	
	
}
```

函数内两次操作g_flag跨了锁保护，可能第二次读取的时候，其值已经被修改，同时编译器可能又优化了这段代码，使用了寄存器中缓存的g_flag值。那么总体的逻辑就是错的。这种情况就需要对g_flag加volatile修饰，或者修改func的实现逻辑，保证在一个锁内处理完。
另外，如果LOCK和UNLOCK操作会触发寄存器的变化，从而使得编译器无法优化掉g_flag的读取流程，那么这个逻辑也不会有问题。所以在实际的代码中，并不是所有的全局变量都加了volatile修饰，但是硬件寄存器相关的变量都是增加了volatile修饰的。在linux kernel
中，90%以上的volatile都是在driver相关的代码中使用的，而其他内核模块都没有使用volatile。

# volatile修饰指针
volatile struct A * pA；  这个表示指针指向的变量，也就是一个结构体A是易变的
struct A * volatile pA；  这个表示指针本身是一个易变的，而不是说结构体A易变。其原理和const类似。

