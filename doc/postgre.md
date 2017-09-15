[POSTGRE](../../postgre) 开源项目内存管理源代码分析笔记

# 相关代码
 1. 头文件 palloc.h, memutils.h, memdebug.h
 2. c代码在路径/src/backend/utils/mmgr 下面
 3. c代码目录下有一个readme文件详细介绍了mmgr子模块的设计思想和关键数据结构，全局变量
 
# 重点数据结构
 * 数据结构`MemoryContextMethods`有一个全局变量`AllocSetMethods`，作为了allocset的默认方法集。
 
 * 数据结构`MemoryContext`的一个全局变量`CurrentMemoryContext`，使用了一个特殊修饰[PGDLLIMPORT](http://blog.csdn.net/clever101/article/details/5421782)，这个全局变量在readme里面讲解的很清楚了。注意`MemoryContext`这个结构一定是在`AllocSetContext`的头部的，因为系统中有直接强转指针的操作，而没有使用container那种操作。
 
 * `MemoryContext`还有一个全局变量`TopMemoryContext`表示系统全局memory context的根，是在整个系统初始化的时候创建出来的(后续创建的内存context都会是他的子孙，这种树状管理结构能够保证不会发生内存泄露。因为一个memory context可以track管理他的子孙memory context,当一个memory在free或者reset的时候，会递归地去释放他的子孙memory context管理的内存);同时创建了`ErrorContext`用作异常处理的。另外还有若干个相同类型的全局变量，不过都是和postgre其他子模块业务相关的，包括`PostmasterContext, CacheMemoryContext, MessageContext, TopTransactionContext, CurTransactionContext`.
 
 * 数据结构`MemoryContext`的一个全局变量`TopMemoryContext`，这是系统顶层的memory context。
 
 * `AllocBlockData`
 
 * `AllocChunkData`
 
***

```
AllocSetContext --
                  |- MemoryContextData : header
                  |- AllocBlock : block 链表结构，串链上所有属于这个set的block
                  |- AllocChunk[] : freelist
                  |- AllocBlock : keeper
                  

```

***

# 代码理解
 1. MemoryContextCreate 中分配内存时如果`TopMemoryContext`如果为空，则直接调用库函数malloc，否则就会调用`MemoryContextAlloc`。这是一个设计上的约束，全局只有一个顶层memorycontext，所有进程申请的memorycontext都会作为他的子孙。
 2. MemoryContextCreate 内在申请完内存之后，会立即调用注册的init方法完成初始化，再插入memorycontext的关系树中。
 3. `MemoryContextAlloc` 调用的注册方法`AllocSetMethods`中的alloc
 4. block,chunk 概念
 
# 典型流程
 1. postgre server启动的时候，main函数内会调用`MemoryContextInit`，完成`TopMemoryContext`的申请，这样系统中后续所有memory context的根就创建出来了，另外还创建了另外一个根`ErrorContext`。
 1. 首先，使用`AllocSetContextCreate`创建一个分区，最顶层的parent为NULL，即创建出`TopMemoryContext`
 2. 业务流程可以使用`MemoryContextAlloc`或它的扩展接口去申请内存，`palloc`功能和它类似，只是不需要传入memory context，因为`palloc`使用了全局变量`CurrentMemoryContext`，即默认使用当前的memory context。
 3. 业务第一次申请内存的时候memory context（set）里面block和freelist都是空的，会触发第一次内存分配，并且malloc出来的内存是大于用户要求size的，然后设置好set中block。这样后面的判断就会发现block非空了。
 4. 业务第二次申请内存的时候，memory context(set)的block非空，freelist为空，此时会触发freelist的设置：把block中剩下没用的内存按照规则放入freelist的slot中。然后，走入了上一步第一次分配内存的流程，会去malloc一个新的block，并插入到set里block的链表头，之前那个老的block现在就处于链表的第二个位置了，但是老block上剩余的空间已经由freelist标识出来了。
 5. 业务第三次来申请内存，假设本次申请的内存size比较小，可以从freelist中取，并把这个chunk置为已用。__这里怎么保证下次不会分到这个freelist的slot的？__
 5. realloc,free这两个操作只有p***类接口提供了，业务根据需要调用。__TODO__
 6. 最后，在一片memory context释放完成或者reset的时候，会触发用户注册的callback调用，可以完成一些用户工作。
 
 # `AllocSetMethods`方法集分析
  * `alloc`方法: 这个函数有300多行，太长了。。。
 > `AllocSetAlloc` 入参是`MemoryContext`，然后强转成了包含他的父类对象`AllocSet`，后面的所有操作都是基于`AllocSet`结构的。
 > 如果需要分配的size大于了`allocChunkLimit`，则需要重新分配一个新的block，使用的是库函数malloc。注意这里申请的大小是用户申请的size圆整后，再加上了一个`AllocBlockData`和`AllocChunkData`的size；block中freeptr指向block的endptr，而这个endptr指向的是整个block的尾部地址的下一个位置，假设block的地址范围是0~9，长度为10，那么endptr就是10，指向的是block后面的那个位置。申请的block结构再挂入到set的block链表中，组成了一个单链表结构。
 > 申请的block内存排布如下
 
 ```
  
    -------------------
    |  AllocBlockData |
    -------------------
    |  AllocChunkData |
    -------------------
    |  user memory    |    <-------  申请返回的指针指向的user memory这个区域头
    -------------------
    
 ```
 
  > 如果需要分配的size小于等于了`allocChunkLimit`（8 byte），则会从freelist数组中去找到一片大小合适的内存使用，这样就不需要重新分配内存了。freelist数组的组织形式是按2的幂递增的，index 0的内存大小是8byte，依次是16，32...整个数组有11个slot，所以最大单元是8k byte内存。选择内存块的算法参见`AllocSetFreeIndex`。选中freelist数组的index之后，再需要判断对应slot上是否为空。<br>
  如果非空，则说明当前单元大小的内存有空闲的，则返回本chunk所在的内存。这个chunk被使用的标志就是其aset字段设为了其父对象AllocSet的指针，这个设计有点奇葩。
  如果为空，则chunk_size先设置为8*index；再判断set里面的block链表中是否有free空间足够的，有则从freelist中抠出来，并修改相关管理字段，这一步逻辑比较复杂。block为空的时候，则会malloc一块新的block，大小则是set的字段`nextBlockSize`指定（这个值是变化的，每次成功分配就放大2倍）。
 
  * `free_p` : `AllocSetFree`
  
  * `realloc` : `AllocSetRealloc`
  * `init` : `AllocSetInit`实际什么都没有做，是个空函数。
  * `reset` : `AllocSetReset`
  * `delete_context` : `AllocSetDelete`
  * `get_chunk_space` : `AllocSetGetChunkSpace`
  * `is_empty` : `AllocSetIsEmpty`
  * `stats` : `AllocSetStats`
  * `check` : `AllocSetCheck`
