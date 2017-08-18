# [nvme驱动分析](http://blog.csdn.net/zhuzongpeng/article/details/76136164)

***
# nvme kernel driver阅读笔记:
- nvme/host/core.c init中创建了workqueue和nvme class，以及注册了一个字符设备，这个字符设备没有write和read的ops，只有ioctrl，用于处理nvme设备的命令admin，io，reset，scan。这个字符设备的创建则是在probe流程中，调用device_create_with_groups创建
- admin和io命令最后调用到__nvme_submit_user_cmd 
- nvme驱动中真正执行是nvme_probe函数，在nvme设备被侦测到的时候由kernel去调用，调用的判断原则就是>device id需要吻合
- device，pci_dev, nvme_dev三个数据结构的继承组合关系 
- nvme设备驱动probe中会将设备挂入`nvme_ctrl_list`全局链表中，这个链表在open这个nvme的字符设备时会用 
- [work queue讲解](http://www.cnblogs.com/wwang/archive/2010/10/27/1862202.html) 
- probe的最后一步会调用nvme_reset_work函数完成nvme设备的配置操作 
- `nvme_reset_work`中会依次使能pci设备，配置admin queue等基本属性，调用`nvme_init_identify`读取identify信息存入内存ctrl结构（这个函数还会配置power state和directive属性），再配置io queue，最后调用nvme_start_ctrl启动nvme设备work queue后台nvme_wq中的若干任务。
- 提交nvme command使用函数`nvme_submit_sync_cmd` 或 `nvme_submit_user_cmd`（后者处理从用户态ioctl下发的请求），下发执行依赖了block块设备层`blk_execute_rq`等接口


# LLD3收获：
- [memory barrier](http://www.wowotech.net/kernel_synchronization/memory-barrier.html)  其作用是保证barrier前后的代码执行先后顺序，因为有些硬件设备内部有缓存机制，在多核运行场景下编译器优化后可能导致指令的执行顺序乱序，从而出现极难定位的问题。通过barrier可以防止这种情况发生，但是会对性能有影响。使用时需要选择合适的barrier函数，比如只考虑读，只考虑写或者都需要考虑。
- all tasklet code must be atomic, but faster（run in the next time tick）; workqueue allowed to sleep.
