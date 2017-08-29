# [nvme驱动分析](http://blog.csdn.net/zhuzongpeng/article/details/76136164)
# [nvme协议讲解](http://mp.weixin.qq.com/s?__biz=MzIwNTUxNDgwNg==&mid=2247484399&idx=1&sn=9a7612489ba72d94e17304c7476d271f&chksm=972ef2b6a0597ba07c4dd45996602b9d891cc195478e3643b6bd32ff57c0b3f1c2f7ee98b69c&scene=21#wechat_redirect)

***
# nvme 协议学习笔记：
- nvme协议规定了nvme的命令格式，command占用64Byte，前4个Byte为command Dword0，为通用格式。命令分为Admin Command 10个，IO Command 3个（read，write，flush）；Admin/IO command大小为64B，对应的Completion大小为16B。
- NVMe 规范定义的 class code 是 0x010802
- nvme 设备只使用PCIE HEADER中的BAR0，BAR1两个寄存器，合并为一个64bit的寄存器使用。
- nvme协议规定使用queue进行命令交互，分为admin queue和IO queue两种，每种queue都分为submission 和completion两种queue。其中admin queue全系统只能有一个；IO queue一般是一个core一个（最多可以有64K个），如果一个core分配多个IO submission queue（但是只对应一个completion queue）则一般用于qos任务分级处理；Admin SQ/CQ的队列深度是2~4K；而IO SQ/CQ的队列深度是2~64K

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

