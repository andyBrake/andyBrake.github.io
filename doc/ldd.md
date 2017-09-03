- [memory barrier](http://www.wowotech.net/kernel_synchronization/memory-barrier.html)  其作用是保证barrier前后的代码执行先后顺序，因为有些硬件设备内部有缓存机制，在多核运行场景下编译器优化后可能导致指令的执行顺序乱序，从而出现极难定位的问题。通过barrier可以防止这种情况发生，但是会对性能有影响。使用时需要选择合适的barrier函数，比如只考虑读，只考虑写或者都需要考虑。
- all tasklet code must be atomic, but faster（run in the next time tick）; workqueue allowed to sleep.
- 中断号的注册最好是在probe阶段注册，不要在moudle加载，否则会提前占用中断号资源，而设备很可能不会被使用。
- tasklet 只会被执行一次，即使在执行前被显示地调用了多次，且是中断上下文不能使用睡眠；tasklet执行在发起本tasklet的相同cpu上，且一定是第一个被调度执行的函数（只有中断可能比它早执行）；tasklet有两种优先级；
- workqueue在进程上下文中，可以睡眠，但是不能直接将内核内存拷贝到用户态内存
- kobject 必须对应一个kobj_type，其会提供release方法。[实例](http://www.cnblogs.com/wwang/archive/2010/12/16/1902721.html)
- 如果kobject加入了一个kset，那么其kobj_type一般会置空，会使用kset里面的ktype字段。
- subsystem最细内核已经移除，只是对kset的包装;
- binary attributes一般只有在firmware upload时使用，需要使用特殊结构bin_attribute，支持二进制读写，普通的attributes只支持字符串形式读写；具体可以参考[这里](http://www.wowotech.net/device_model/dm_sysfs.html)
- 热插拔事件由kobject_add 和 kobject_del触发产生;kset结构内的kset_uevent_ops结构用于处理这类event；kset_uevent_ops中的filter如果0则不会产生event处理，相当于忽略本次事件；
- bus_type表示linux内核中的总线抽象模型，device类型表示内核设备模型抽象，具体的设备类型又是这个结构的子类，比如pci_dev,usb_device;设备驱动的抽象结构是device_driver,具体的驱动类型需要封转这个抽象结构;class是对底层具体硬件的再次抽象，基于功能的抽象，比如scsi磁盘，ata磁盘都抽象为一个class，但是他们具体怎么做class不关心，class在/sys/class中显示；属于某种class的设备描述结构为class_device，其关联上class和device;
- 设备都被会对应到ram虚拟文件系统sysfs中，其具体作用和设计可以参考[这里](https://www.ibm.com/developerworks/cn/linux/l-cn-sysfs/index.html)
- request, request_queue, bio:
  1. request_queue 非常复杂，除了记录未完成的request之外，还包含很多配置信息，比如对齐值，最大值，sector size等。
  2. request 表示的是一个在磁盘上连续的请求，io schduler内部会合并request，但不会合并读写请求，也不会合并任何违法限制的请求（比如超过最大size）。
  3. bio 是virtual memory subsystem or system call发起的一组block请求，相当于是kernel和block层交互的数据结构。block层会决定这个请求是合并给现有的request还是新建一个request。`the bio structure contains everything that a block driver needs to carry out the request without reference to the user-space process that caused that request to be initiated`.
  4. bio内的核心结构是struct bio_vec，其作用和组织形式类似SGL；bio是单链表构造；
- 对于无需排队优化的块设备，不需要使用queue，而需要提供一个make_request_fn接口；其可以提供直接数据传送功能，或者重定向请求功能。bio请求完成时，也不能调用queue相关的接口，而使用bio_endio接口。
