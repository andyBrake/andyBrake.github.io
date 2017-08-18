# [nvme](./doc/nvme.md)

***

# LLD3收获：
- [memory barrier](http://www.wowotech.net/kernel_synchronization/memory-barrier.html)  其作用是保证barrier前后的代码执行先后顺序，因为有些硬件设备内部有缓存机制，在多核运行场景下编译器优化后可能导致指令的执行顺序乱序，从而出现极难定位的问题。通过barrier可以防止这种情况发生，但是会对性能有影响。使用时需要选择合适的barrier函数，比如只考虑读，只考虑写或者都需要考虑。
- all tasklet code must be atomic, but faster（run in the next time tick）; workqueue allowed to sleep.
