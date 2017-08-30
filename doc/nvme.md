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
- `nvme_probe`函数分析：首先，申请`nvme_dev`数据结构内存，作为nvme设备驱动信息的内存载体；给`dev->queues`申请内存，这个queue是若干对SQ&CQ的指针数组,数组大小即SQ&CQ的对数，其取决于cpu个数，即相当于这里的分配方式一个cpu一对SQ&CQ，这是协议规范里面的经典样式；再使用`nvme_dev_map`去保留mem和io映射空间，remap寄存器地址（这里映射的是NVME_REG_DBS + 4096 这么大范围，注意后面又会有几次remap，会先unmap重新map，范围大小会变化；映射使得`dev->bar`代表的就是nvme协议规定的register address详见协议3.1节），方便后面的操作可以直接进行内存操作；再初始化了两个work queue，`nvme_reset_work`和`nvme_remove_dead_ctrl_work`；再使用`nvme_setup_prp_pools`创建DMA内存池，方便后面的DMA操作；接着完成nvme设备的初始化`nvme_init_ctrl`；再使用`nvme_change_ctrl_state`修改nvme设备为`NVME_CTRL_RESETTING`；最后，调度执行workqueue的reset work `nvme_reset_work`。
- _上一个流程中的`nvme_init_ctrl`主要完成的工作：_
  ## 1、注册nvme设备的ctrl ops为`nvme_pci_ctrl_ops`，这个ops包含了读写寄存器的操作接口；使用内核驱动框架提供的`device_create_with_groups`将本nvme设备注册加入`nvme_class`类中（这个class是在core.c的init中创建的），这样在内核设备模型中就加入了新设备，可以在sysfs中访问了。其具体实现和框架设计可以参考LDD3。 
  ## 2、将该设备加入`nvme_ctrl_list`中，用锁`dev_list_lock`保护。注意`nvme_ctrl_list`在nvme字符设备驱动的open中`nvme_dev_open`会使用，使用的方式就是遍历链表找到对应的ctrl结构，然后将其挂入kernel传入的file pointer中，使用file 的private data字段存储设备驱动内存结构，这也是kernel driver的典型方式。 
  ## 3、设置了latency tolerance controls，具体有什么用，暂时我也没理解到。

- `nvme_change_ctrl_state`功能很简单，根据当前nvme ctrl中的状态和设置的新状态，决定是否修改内存状态。在init流程中，这里会触发修改，将状态改为`NVME_CTRL_RESETTING`。
- _`nvme_reset_work`中的处理需要结合nvme协议才能了解这些步骤的原因，具体的操作流程如下：_
  ## 1、首先是disable了nvme设备，然后才能后续的配置，配置完了最后才会再启动nvme设备，这是协议规定的，可以参考协议中讲解reset的章节。
  ## 2、再调用`nvme_pci_enable`完成DMA MASK设置、pci总线中断分配、读取并配置queue depth、stride等参数。
  ## 3、调用`nvme_pci_configure_admin_queue`去配置admin queue:
	* 首先用`nvme_remap_bar`这个函数完成了bar的映射，这次映射的size是NVME_REG_DBS + 一个queue的大小，这个queue就是admin queue.
	
	* 再读取NVME_REG_VS寄存器，完成设置dev的subsystem值和写NVME_REG_CSTS寄存器；调用`nvme_disable_ctrl`关闭nvme设备，这样后面才能开始配置admin queue；

	* 调用`nvme_alloc_queue`申请queue内存（注意这里的queue含义不是nvme协议的queue，而是一个封装了nvme queue的数据结构，里面包含了sq，cq队列以及一些其他控制结构），这个函数是申请queue的通用函数，这里用于申请admin queue，后续申请IO queue也是用它，这个函数内部申请nvme的queue时，由于需要保证物理地址连续，会使用`dma_zalloc_coherent`去申请内存；

     * 申请完admin queue之后，就需要将admin queue的相关信息写入协议规定的寄存器中NVME_REG_AQA、NVME_REG_ASQ、NVME_REG_ACQ这三个寄存器；
     
     * 使能nvme设备`nvme_enable_ctrl`，具体操作就是写特定寄存器特定值，再等待一定时间。

     * 最后，使用`queue_request_irq`完成中断资源的申请，nvme协议推荐使用的是MSI中断方式。

  ## 4、调用`nvme_init_queue`初始化 queue：初始化sq_tail,cq_head,cq_phase,doorbell等字段
  ## 5、调用`nvme_alloc_admin_tags`完成dev中blk_mq_tag_set结构的设置，这部分跟块设备驱动层有关，内部细节暂未分析
  ## 6、调用`nvme_init_identify`，使用`nvme_admin_identify = 0x06`这个opcode下发给nvme设备，读取出了identify信息，并更新到了内存结构了；设置nvme nqn信息`nvme_init_subnqn`，这个代码注释写的是“Generate a "fake" NQN per Figure 254 in NVMe 1.3 + ECN 001”应该1.3版本协议规定的内容；设置quirks，具体quirk是撒，还不太清楚；设置block的request queue的属性值，函数`nvme_set_queue_limits`，内部调用了若干块设备层的接口，详细注解可以查询LDD3块设备驱动章节；最后配置了nvme设备的APST (Autonomous Power State Transition)和directive。

  ## 7、根据OACS（Optional Admin Command Support）字段进行对应的设置操作，主要有NVME_CTRL_OACS_SEC_SUPP和NVME_CTRL_OACS_DBBUF_SUPP。
  
  ## 8、使用`nvme_setup_host_mem`在主机端分配DMA内存，用于设备的DMA操作，并通过nvme_admin_set_features 0x09这个命令设置给nvme设备。
  ## 9、`nvme_setup_io_queues`设置io queue：
	* 1、首先会获取系统cpu个数，然后通过set feature命令去设置queue个数为cpu个数，然后根据命令的返回值去决定最终的queue个数；
	* 2、第二步计算出queue的深度；
	* 3、第三步，根据queue个数计算出bar size，再调用`nvme_remap_bar`去完成映射。注意，理论上这次映射是范围最大的映射了，因为包含了基础寄存器和所有的queue；
	* 4、然后，释放pci的中断资源再重新申请一次中断资源，因为考虑重入，重启等场景，这里是需要先释放再申请的；
	* 5、最后，调用`nvme_create_io_queues`去完成io queue的资源分配，具体操作类似前面分配admin queue资源，区别在于创建io queue是通过`nvme_admin_create_cq`这个command下发给nvme来实现的，而admin queue是通过写基础寄存器来实现的。
  
  ## 10、对所有namespace调用`nvme_start_queues`,`nvme_wait_freeze`，`nvme_dev_add`，`nvme_unfreeze`完成块设备驱动层的操作，内部流程暂未分析。
  ## 11、nvme设备的状态修改为`NVME_CTRL_LIVE`，之前应该是处于`NVME_CTRL_RESETTING`状态。
  ## 12、最后，调用`nvme_start_ctrl`启动nvme设备，其内部实现就是开始调度执行各种work queue：
    * `nvme_keep_alive_work`后台执行`nvme_admin_keep_alive`command；

    * `nvme_scan_work`这个work是在`nvme_init_ctrl`的时候注册给`ctrl->scan_work`的。其功能是扫描并从`nvme_ns_attr_group`中移除无效的namespace。
    
    * `nvme_async_event_work`这个work是在`nvme_init_ctrl`的时候注册给`ctrl->async_event_work`的。其功能是处理event事件，调用的是`ctrl->ops->submit_async_event`这个接口，也是init的时候注册的。

    * 启动各种work queue后，最后还有一步对所有namespace调用`blk_mq_unquiesce_queue`的操作。
    
- nvme的admin command的opcode列表在协议第五章，Figure 41中，共10个必选命令。
- nvme的IO command的opcode列表在协议第六章，Figure 188中，只有3个必选命令。
- admin和io命令最后调用到__nvme_submit_user_cmd 
- device，pci_dev, nvme_dev三个数据结构的继承组合关系 
- nvme设备驱动probe中会将设备挂入`nvme_ctrl_list`全局链表中，这个链表在open这个nvme的字符设备时会用 
- [work queue讲解](http://www.cnblogs.com/wwang/archive/2010/10/27/1862202.html) 
- probe的最后一步会调用nvme_reset_work函数完成nvme设备的配置操作 
- `nvme_reset_work`中会依次使能pci设备，配置admin queue等基本属性，调用`nvme_init_identify`读取identify信息（协议5.15节详解该命令）存入内存ctrl结构（这个函数还会配置power state和directive属性），再配置io queue，最后调用nvme_start_ctrl启动nvme设备work queue后台nvme_wq中的若干任务。
- 提交nvme command使用函数`nvme_submit_sync_cmd` 或 `nvme_submit_user_cmd`或`nvme_user_cmd`（处理从用户态ioctl下发的请求），下发执行依赖了block块设备层`blk_execute_rq`等接口
- nvme设备提交IO请求使用函数`nvme_submit_io`
- `nvme_submit_user_cmd`实际实现为`__nvme_submit_user_cmd`,在nvme设备驱动中有request_queue结构，用于和块设备层进行交互。交互的载体为bio结构。首先使用`blk_mq_alloc_request`分配出一个request结构；带user buffer的情况下，会使用块设备层若干接口完成映射，未深入分析；再使用`blk_execute_rq`执行本次request；最后释放相关资源，完成。块设备驱动接口可以参考LDD3中的块设备章节。

