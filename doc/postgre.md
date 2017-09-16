[POSTGRE](../../postgre) 开源项目内存管理源代码分析笔记

# 相关代码
 1. 头文件 palloc.h, memutils.h, memdebug.h
 2. c代码在路径/src/backend/utils/mmgr 下面
 3. c代码目录下有一个readme文件详细介绍了mmgr子模块的设计思想和关键数据结构，全局变量

# 重点数据结构
 * 数据结构`MemoryContextMethods`有一个全局变量`AllocSetMethods`，作为了allocset的默认方法集。后面重点会分析其实现函数。

 * 数据结构`MemoryContext`的一个全局变量`CurrentMemoryContext`，使用了一个特殊修饰[PGDLLIMPORT](http://blog.csdn.net/clever101/article/details/5421782)，这个全局变量在readme里面讲解的很清楚了。注意`MemoryContext`这个结构一定是在`AllocSetContext`的头部的，因为系统中有直接强转指针的操作，而没有使用container那种操作。

 * `MemoryContext`还有一个全局变量`TopMemoryContext`表示系统全局memory context的根，是在整个系统初始化的时候创建出来的(后续创建的内存context都会是他的子孙，这种树状管理结构能够保证不会发生内存泄露。因为一个memory context可以track管理他的子孙memory context,当一个memory在free或者reset的时候，会递归地去释放他的子孙memory context管理的内存);同时创建了`ErrorContext`用作异常处理的。另外还有若干个相同类型的全局变量，不过都是和postgre其他子模块业务相关的，这里不做讨论，包括`PostmasterContext, CacheMemoryContext, MessageContext, TopTransactionContext, CurTransactionContext`.

```
所有带Data结尾的数据结构，去掉Data即是这个结构的指针别名

typedef struct AllocSetContext
{
	MemoryContextData header;	//这个结构一定是放头部的，代码中会强转这两种类型
	/* Info about storage allocated in this context: */
	AllocBlock	blocks;	//这个context下malloc出来的内存块链表，只有第一个block中还有可以使用的内存，其他的都被拆分好放在freelist中了
	AllocChunk	freelist[ALLOCSET_NUM_FREELISTS];		//空闲单元内存的chunk结构指针数组，每一个slot中存的一种大小内存的第一个chunk控制结构，chunk自身又会由链表形式串起来。aset字段就会用于指向下一个chunk控制结构。
	//下面4个参数是context的配置值
	Size		initBlockSize;	/* initial block size */
	Size		maxBlockSize;	/* maximum block size */
	Size		nextBlockSize;	/* next block size to allocate */
	Size		allocChunkLimit;	/* effective chunk size limit */

  AllocBlock	keeper;			/* if not NULL, keep this block over resets */
} AllocSetContext;
```

 * `AllocBlockData` 在AllocSetContext结构中以单向list形式存在，存储这个context已经malloc分配出来的内存。每一次新malloc出来的内存，头部一定会插入这个结构，用来管理这一片新内存。

 * `AllocChunkData` 内存使用的时候，是以chunk为最小单位从block中分出去给业务模块使用，所以每一块即将被使用的内存头部都会插入一个这个数据结构。对于已经分出去使用的chunk，其内部的aset指针会指向AllocSetContext，而没有分出去的呢，则可能指向NULL或者下一个空闲的chunk结构（参见freelist数组）。比如如下图所示，表示一个block使用后的内存布局情况。

`|AllocBlockData  |  AllocChunkData | user memory    8k     | AllocChunkData  | user memory   4k |`

***

# 典型流程
 1. postgre server启动的时候，main函数内会调用`MemoryContextInit`，完成`TopMemoryContext`的申请，这样系统中后续所有memory context的根就创建出来了，另外还创建了另外一个根`ErrorContext`。首先，使用`AllocSetContextCreate`，其内部会调用`MemoryContextAlloc`创建一个特殊的分区，其parent为NULL，即创建出`TopMemoryContext`。

 2. 业务流程可以使用`MemoryContextAlloc`或它的扩展接口去申请内存，`palloc`功能和它类似，只是不需要传入memory context，因为`palloc`使用了全局变量`CurrentMemoryContext`，即默认使用当前的memory context。我们这里假设业务流程直接就用这个memory context了，不再创建新的memory context，简化流程分析。不过实际应用场景，最好是一个业务模块申请一个新的memory context作为top的子孙，这样方便业务间内存管理隔离，分配和释放，检测更灵活一些。

 3. 业务第一次申请内存的时候memory context（set）里面block和freelist都是空的（假设创建context时minContextSize足够小就会这样，否则block在创建memory context的时候就会malloc申请一个出来），会触发第一次内存分配，并且malloc出来的内存是大于用户要求size的，然后设置好set中block。这样后面的判断就会发现block非空了。

 4. 业务第二次申请内存的时候，memory context(set)的block非空，freelist为空，假设此时申请内存的大小在block中不足供应，则此时会触发freelist的设置：把block中剩下没用的内存按照规则放入freelist的slot中。然后，走入了上一步第一次分配内存的流程，会去malloc一个新的block，并插入到set里block的链表头，之前那个老的block现在就处于链表的第二个位置了，但是老block上剩余的空间已经由freelist标识出来了，那么此时老block也不可以再拿来直接分配空间用了，要使用它上面的内存只能由chunk去管理了。

 5. 业务第三次来申请内存，假设本次申请的内存size比较小，可以从freelist中取，并把这个chunk置为已用，然后返回这个chunk的user memory地址即可。具体怎么修改chunk的使用状态？这个在设计上比较绕，实现细节是把使用的freelist指向这个chunk的下一个chunk，即aset指针保存的值，然后把使用的这个chunk的aset指向memory context这个set，这样就标记这个chunk已经被占用了。__aset这个字段的语义是非常复杂多变了哦，有时用来串链，有时用来当bool型值一样作用是非判断。理解代码的时候，理解好这个字段很重要。__

 5. realloc,free这两个操作只有p***类接口提供了，业务根据需要调用。__TODO__

 6. 最后，在一片memory context释放完成或者reset的时候，会触发用户注册的callback调用，可以完成一些用户工作。

# `AllocSetMethods`方法集分析

 * `alloc`方法: 这个函数有300多行，太长了。。。内部实际可以拆分成几个子函数，有优化空间。

 > `AllocSetAlloc` 入参是`MemoryContext`，然后强转成了包含他的父类对象`AllocSet`，后面的所有操作都是基于`AllocSet`结构的。<br>
 <br>
 > 如果需要分配的size大于了`allocChunkLimit`，则需要重新分配一个新的block，使用的是库函数malloc。注意这里申请的大小是用户申请的size圆整后，再加上了一个`AllocBlockData`和`AllocChunkData`的size；block中freeptr指向block的endptr，而这个endptr指向的是整个block的尾部地址的下一个位置，假设block的地址范围是0~9，长度为10，那么endptr就是10，指向的是block后面的那个位置，也就是说这个block的free空间没有了。申请的block结构再挂入到set的block链表头，组成了一个单链表结构。同时，chunkData结构中的chunk size字段也会设置为用户申请内存大小（圆整后的）。最后，返回这片新申请内存的user memory位置。<br>
申请的block内存排布如下

 ```
  
    -------------------
    |  AllocBlockData |    freeptr指向user mem结束后位置
    -------------------
    |  AllocChunkData |    chunksize描述后面uesr mem的大小
    -------------------
    |  user memory    |    <----  申请返回的指针指向的user memory这个区域头
    -------------------

 ```

  > 如果需要分配的size小于等于了`allocChunkLimit`（8 byte），则先尝试从freelist数组中去找到一片大小合适的内存使用。freelist数组元素是chunkdata结构指针，组织形式是按2的幂递增的，index 0存在chunkdata管理的内存大小是8byte，依次是16，32...整个数组有11个slot，所以最大单元是8k byte内存。选择内存块的算法参见`AllocSetFreeIndex`。选中freelist数组的index之后，再需要判断对应slot上是否为空。如果非空，则说明当前单元大小的内存有空闲的，则返回本chunk所在的内存，本次分配就此结束，没有产生新的内存资源分配。这个chunk被使用的标志就是其aset字段设为了其父对象AllocSet的指针，这个设计有点奇葩。<br>
    如果为空，则chunk_size先设置为8*power(2, index),根据前面讲到的index规则很显然此时 chunk_size 即为需要给用户分配的内存，他是大于等于用户申请值的，且为2的整次幂；然后，如果需要分配的空间比block链表头中block上剩余的空间还要大的话，则会进行一次block内存整理操作并重新去malloc一个block。malloc的大小是set的字段`nextBlockSize`指定（这个值是变化的，每次成功分配就放大2倍）；最后，第三步，就从block－>freeptr位置拆chunk出来用了，和前面使用chunk的方式一样，把chunksize设置好，chunk中的aset需要设置为Allocset表示被用了，然后再把block－>freeptr向后移动相应大小的位置。返回这个chunk中的user memory位置，大功告成。

 ***
    <mark>Tips</mark><br>
    block 中的内存整理在aset.c文件的line776 ～ 830. 就是把block中还没用的内存看能不能拆成freelist数组中对应的大小块，并串到其上的chunk链表中去。这样处理的后果是，set里面block链表中只有头block的内存可以直接去拆一个非2次幂的大小chunk出来用，其他的block上内存都是不能再直接使用的了，只能由chunk管理从freelist中分出来使用了。
 ***

  * `free_p` : `AllocSetFree`用户free内存的时候传入的是user memory内存指针，所以函数处理首先会偏移一个chunk大小，取到这个内存的chunk管理结构，然后再继续后面的处理。alloc分配时有两种情况，即申请内存是否大于`allocChunkLimit`，两种情况下内存的来源是有很大不同的，所以分配时针对释放内存的大小，也会分别处理。

  > 第一种情况是内存大小大于了`allocChunkLimit`，那么很显然这个内存是通过malloc出来的一个完整block，所以释放的时候直接从set的block链表中把这个block找到，并摘链再free即可。这种情况下，其实内存操作还有额外性能损耗，相比直接malloc和free来说，多了链表遍历的操作。

  > 第二种情况的处理更简单，因为小内存一定是从block中拆出去的一块，所以把这块内存按照大小在freelist上找到对应的slot位置，然后把它挂入对应chunk list头部位置即可。这样后面如果要分配就可以复用这块内存了，这样的处理对于小块内存来说，没有产生malloc和free操作，性能会有很大提高。

  ***
     <mark>Tips</mark><br>
分析完分配和释放流程之后，可以看到针对小于`allocChunkLimit`大小的内存申请释放，这套机制是起到了很好的性能优化效果的。而大于的话，则性能会更差。所以根据业务场景需要，可以好好考虑如何设置一个合理的`allocChunkLimit`。

  ***

  * `realloc` : `AllocSetRealloc`
  * `init` : `AllocSetInit`实际什么都没有做，是个空函数。
  * `reset` : `AllocSetReset`
  * `delete_context` : `AllocSetDelete`
  * `get_chunk_space` : `AllocSetGetChunkSpace`
  * `is_empty` : `AllocSetIsEmpty`
  * `stats` : `AllocSetStats`
  * `check` : `AllocSetCheck`

***

# 代码的一些细节
   1. MemoryContextCreate 中分配内存时如果`TopMemoryContext`如果为空，则直接调用库函数malloc，否则就会调用`MemoryContextAlloc`。这是一个设计上的约束，全局只有一个顶层memorycontext，所有进程申请的memorycontext都会作为他的子孙。

   2. MemoryContextCreate 内在申请完内存之后，会立即调用注册的init方法完成初始化，再插入memorycontext的关系树中。

   3. `MemoryContextAlloc` 调用的注册方法`AllocSetMethods`中的alloc

   4. block和chunk结构中的aset字段需要好好理解，理解各自作用，就可以理解整套设计了。
