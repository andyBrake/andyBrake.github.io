[nvme驱动分析](http://blog.csdn.net/zhuzongpeng/article/details/76136164)

***
# nvme kernel driver阅读笔记:
- nvme/host/core.c init中创建了workqueue和nvme class，以及注册了一个字符设备，这个字符设备没有write和read的ops，只有ioctrl，用于处理nvme设备的命令admin，io，reset，scan。这个字符设备的创建则是在probe流程中，调用device_create_with_groups创建
- admin和io命令最后调用到__nvme_submit_user_cmd 
- nvme驱动中真正执行是nvme_probe函数，在nvme设备被侦测到的时候由kernel去调用，调用的判断原则就是>device id需要吻合
- device，pci_dev, nvme_dev三个数据结构的继承组合关系 
- nvme设备驱动probe中会将设备挂入nvme_ctrl_list全局链表中，这个链表在open这个nvme的字符设备时会用 
- [work queue讲解](http://www.cnblogs.com/wwang/archive/2010/10/27/1862202.html) 
- probe的最后一步会调用nvme_reset_work函数完成nvme设备的配置操作 
