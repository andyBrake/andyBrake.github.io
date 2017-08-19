# [nvme](./doc/nvme.md)

***

# LLD3收获：
- [memory barrier](http://www.wowotech.net/kernel_synchronization/memory-barrier.html)  其作用是保证barrier前后的代码执行先后顺序，因为有些硬件设备内部有缓存机制，在多核运行场景下编译器优化后可能导致指令的执行顺序乱序，从而出现极难定位的问题。通过barrier可以防止这种情况发生，但是会对性能有影响。使用时需要选择合适的barrier函数，比如只考虑读，只考虑写或者都需要考虑。
- all tasklet code must be atomic, but faster（run in the next time tick）; workqueue allowed to sleep.
- 中断号的注册最好是在probe阶段注册，不要在moudle加载，否则会提前占用中断号资源，而设备很可能不会被使用。
- tasklet 只会被执行一次，即使在执行前被显示地调用了多次，且是中断上下文不能使用睡眠；tasklet执行在发起本tasklet的相同cpu上，且一定是第一个被调度执行的函数（只有中断可能比它早执行）；tasklet有两种优先级；
- workqueue在进程上下文中，可以睡眠，但是不能直接将内核内存拷贝到用户态内存
