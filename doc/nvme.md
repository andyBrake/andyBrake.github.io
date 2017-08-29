# [nvme驱动分析](http://blog.csdn.net/zhuzongpeng/article/details/76136164)
# [nvme协议讲解](http://mp.weixin.qq.com/s?__biz=MzIwNTUxNDgwNg==&mid=2247484399&idx=1&sn=9a7612489ba72d94e17304c7476d271f&chksm=972ef2b6a0597ba07c4dd45996602b9d891cc195478e3643b6bd32ff57c0b3f1c2f7ee98b69c&scene=21#wechat_redirect)

***
# nvme 协议学习笔记：
- nvme协议规定了nvme的命令格式，command占用64Byte，前4个Byte为command Dword0，为通用格式。命令分为Admin Command 10个，IO Command 3个（read，write，flush）；Admin/IO command大小为64B，对应的Completion大小为16B。
- NVMe 规范定义的 class code 是 0x010802
- nvme 设备只使用PCIE HEADER（共64Byte）中的BAR0，BAR1两个寄存器，合并为一个64bit的寄存器使用。该寄存器内的base address中用64-14个bit描述nvme设备的寄存器基地址（即地址一定是一个64K对齐的位置）。这些寄存器在协议的3.1节定义。
- nvme协议规定使用queue进行命令交互，分为admin queue和IO queue两种，每种queue都分为submission 和completion两种queue。其中admin queue全系统只能有一个；IO queue一般是一个core一个（最多可以有64K个），如果一个core分配多个IO submission queue（但是只对应一个completion queue）则一般用于qos任务分级处理；Admin SQ/CQ的队列深度是2~4K；而IO SQ/CQ的队列深度是2~64K

***
# nvme kernel driver阅读笔记:
- nvme/host/core.c init中创建了workqueue `nvme_wq`和nvme class `nvme_class`，以及使用`__register_chrdev`注册了一个字符设备（会动态分配主设备号），这个字符设备没有write和read的ops，只有open,close和ioctrl，用于处理nvme设备的命令admin，io，reset，scan。这个字符设备的创建则是在probe流程中，调用device_create_with_groups创建。总得来说，这个core模块加载只占了系统的设备号资源以及完成一些内存结构的创建，而并没有去真正配置nvme设备，占中断等资源。
- nvme/host/pci.c 这个文件是nvme设备驱动的pci逻辑层代码，会去调用core.c中定义的接口。该模块加载只做一件事，调用`pci_register_driver`将`nvme_driver`结构挂入pci总线。注册的时候需要传入`nvme_id_table`这个接口内就会根据厂商的id和设备id（前面提到的0x010802）编码作为驱动和设备进行匹配的判断标准。  
- 注册上去的接口最重要的是probe接口，它在设备被扫描到时调用，相当于上电，热插拔等情况设备被发现时才会去执行。nvme驱动的probe函数是`nvme_probe`函数，在nvme设备被侦测到的时候由kernel去调用，调用的判断原则就是device id需要吻合
- `nvme_probe`函数分析：首先，申请`nvme_dev`数据结构内存，作为nvme设备驱动信息的内存载体；给`dev->queues`申请内存，这个queue是若干对SQ&CQ的指针数组,数组大小即SQ&CQ的对数，其取决于cpu个数，即相当于这里的分配方式一个cpu一对SQ&CQ，这是协议规范里面的经典样式；再使用`nvme_dev_map`去保留mem和io映射空间，remap寄存器地址，方便后面的操作可以直接进行内存操作；再初始化了两个work queue，`nvme_reset_work`和`nvme_remove_dead_ctrl_work`；再使用`nvme_setup_prp_pools`创建DMA内存池，方便后面的DMA操作；

- admin和io命令最后调用到__nvme_submit_user_cmd 
- device，pci_dev, nvme_dev三个数据结构的继承组合关系 
- nvme设备驱动probe中会将设备挂入`nvme_ctrl_list`全局链表中，这个链表在open这个nvme的字符设备时会用 
- [work queue讲解](http://www.cnblogs.com/wwang/archive/2010/10/27/1862202.html) 
- probe的最后一步会调用nvme_reset_work函数完成nvme设备的配置操作 
- `nvme_reset_work`中会依次使能pci设备，配置admin queue等基本属性，调用`nvme_init_identify`读取identify信息（协议5.15节详解该命令）存入内存ctrl结构（这个函数还会配置power state和directive属性），再配置io queue，最后调用nvme_start_ctrl启动nvme设备work queue后台nvme_wq中的若干任务。
- 提交nvme command使用函数`nvme_submit_sync_cmd` 或 `nvme_submit_user_cmd`或`nvme_user_cmd`（处理从用户态ioctl下发的请求），下发执行依赖了block块设备层`blk_execute_rq`等接口
- nvme设备提交IO请求使用函数`nvme_submit_io`
- `nvme_submit_user_cmd`实际实现为`__nvme_submit_user_cmd`,在nvme设备驱动中有request_queue结构，用于和块设备层进行交互。交互的载体为bio结构。首先使用`blk_mq_alloc_request`分配出一个request结构；带user buffer的情况下，会使用块设备层若干接口完成映射，未深入分析；再使用`blk_execute_rq`执行本次request；最后释放相关资源，完成。块设备驱动接口可以参考LDD3中的块设备章节。

