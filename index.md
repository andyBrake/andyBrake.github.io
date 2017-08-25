# [nvme](./doc/nvme.md)

***

# LLD3收获：
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
