
> `nvme_reset_work`的时候会调用`nvme_alloc_admin_tags`这个函数第一次看到的时候没有理解其作用，只是发现一堆注册代码，后续看target代码时才发现它的功能比较重要，单独分析一下。这个函数主要当admin_q为空时注册了`dev->admin_tagset`里面的一些字段（理论是此时一定是空滴，open的时候有判断，一定要求是空的；当然使用静态queue时是非空的，我没有去分析这种场景）。这些字段里面最重要的是ops，注册为结构blk_mq_ops `nvme_mq_admin_ops`这个结构相当于是request queue的请求处理操作集，后面会看到很多地方需要用它的。然后request queue会通过调用`blk_mq_init_queue`来分配一个request queue给admin_q字段用，这样和块设备层交互的一个request queue就有了。request queue为什么这么重要，看一下LDD3块设备驱动章节就知道了，块设备驱动和内核块设备层交互的方式就是request queue，所有的请求（不管是管理还是IO）都会发送到queue中，内核块设备层调用IO调度之后，再以bio的形式下发到设备的驱动层，也就是nvme驱动的target层了。这样nvme驱动的host和target两层就相当于通过request queue接连起来了，中间的内核块设备层就可以视为透明的，封装了内核的一些通用操作。
>> nvme_mq_admin_ops这个ops注册了6个函数,`init_hctx`和`exit_hctx`都很简单。`init_request`也很简单，只是选择了一下请求的下发的queue队列。timeout复杂一点，在request请求超时时调用，会去做各种异常判断和处理，比如是不是需要reset设备啊，是否需要abort命令啊。剩下的两个函数`nvme_queue_rq`和`nvme_pci_complete_rq`才是请求处理的关键函数，后者很明显就是request请求处理完成后的回调函数做了一些资源释放的操作，再调用`blk_mq_end_request`通知内核的块设备层这个请求完成了；前者就是真正的nvme请求处理的函数了，其实绕了半天走到这里才是真正开始对nvme设备进行指令下发，在这个函数里首先对command结构简单的处理了一下，然后调了`blk_mq_start_request`注意这个接口和`nvme_pci_complete_rq`里面调的`blk_mq_end_request`是前后呼应的。OK，这些事搞完之后，加锁执行了`__nvme_submit_cmd`和`nvme_process_cq`这两个接口，然后就结束了。。。
>> `__nvme_submit_cmd`这个函数干了撒呢，代码不到10行，就干了一个事，把command用memcpy写入`nvmeq->sq_cmds[nvmeq->sq_tail]`，然后写doorbell，告诉nvme设备，来请求了。这个函数干的事其实就是协议里面规定的命令处理流程的提交部分，写请求到请求queue中（这个queue又是怎么选中的呢，就是前面讲的init步骤），再触发一下doorbell。后面的事就交给nvme设备处理了。
>> 前面的提交函数把协议规定的提交流程完成了，那等待nvme返回结果的步骤肯定就是`nvme_process_cq`干的事了哈。这个函数里面就一个循环判断CQ队列是否有新来的completion command，有就处理了，没有就退出了。判断的依据就是phase字段啦，协议里面有规定。封装的函数是`nvme_cqe_valid`，很多地方会调它。特别说明nvme的中断处理函数`nvme_irq_check`中也会调用这个函数去判断是否该中断信号是nvme设备产生的。中断处理函数又是在`queue_request_irq`这个函数里面去使用`pci_request_irq`注册pci设备中断的。在创建或者配置nvme queue的时候会设置。

***

# nvme/target/core.c这个文件实现nvme设备目标端功能，相当于nvme设备命令的具体下发实现了。

## nvmet_req_init 
### nvmet_parse_connect_cmd
### nvmet_parse_io_cmd
### nvmet_parse_fabrics_cmd
### nvmet_parse_discovery_cmd
### nvmet_parse_admin_cmd
