# nvme设备中的块设备驱动细节

> 1、`nvme_reset_work`中会调用`nvme_alloc_admin_tags`这个函数第一次看到的时候没有理解其作用，只是发现一堆注册代码，后续再看代码中涉及的块设备驱动代码时才发现它的功能比较重要，单独分析一下。这个函数主要当admin_q为空时注册了`dev->admin_tagset`里面的一些字段（理论是此时一定是空滴，open的时候有判断，一定要求是空的；当然使用静态queue时是非空的，我没有去分析这种场景）。这些字段里面最重要的是ops，注册为结构blk_mq_ops `nvme_mq_admin_ops`这个结构相当于是request queue的请求处理操作集，后面会看到很多地方需要用它的。然后request queue会通过调用`blk_mq_init_queue`来分配一个request queue给`admin_q`字段用，这样和块设备层交互的一个request queue就有了。request queue为什么这么重要，看一下LDD3块设备驱动章节就知道了，块设备驱动和内核块设备层交互的方式就是request queue，所有的管理请求都会发送到queue中，内核块设备层调用IO调度之后，再下发到设备的驱动层。比如`nvme_user_cmd`中执行配置类命令时使用了`nvme_submit_user_cmd`这个函数并且ns置空，这样请求就是发送给`admin_q`去下发执行了。
>> 1-1、nvme_mq_admin_ops这个ops注册了6个函数,`init_hctx`和`exit_hctx`都很简单。`init_request`也很简单，只是选择了一下请求的下发的queue队列。timeout复杂一点，在request请求超时时调用，会去做各种异常判断和处理，比如是不是需要reset设备啊，是否需要abort命令啊。剩下的两个函数`nvme_queue_rq`和`nvme_pci_complete_rq`才是请求处理的关键函数，后者很明显就是request请求处理完成后的回调函数做了一些资源释放的操作，再调用`blk_mq_end_request`通知内核的块设备层这个请求完成了；前者就是真正的nvme请求处理的函数了，其实绕了半天走到这里才是真正开始对nvme设备进行指令下发，在这个函数里首先对command结构简单的处理了一下，然后调了`blk_mq_start_request`。注意这个接口和`nvme_pci_complete_rq`里面调的`blk_mq_end_request`是前后呼应的。OK，这些事搞完之后，加锁执行了`__nvme_submit_cmd`和`nvme_process_cq`这两个接口，然后就结束了。。。<br>
>> 1-2、`__nvme_submit_cmd`这个函数干了撒呢，代码不到10行，就干了一个事，把command用memcpy写入`nvmeq->sq_cmds[nvmeq->sq_tail]`，然后写doorbell，告诉nvme设备，来请求了。这个函数干的事其实就是协议里面规定的命令处理流程的提交部分，写请求到nvme queue也就是SQ中（这个queue又是怎么选中的呢，就是前面讲的init步骤），再触发一下doorbell。后面的事就交给nvme设备处理了。<br>
>> 1-3、前面的提交函数把协议规定的提交流程完成了，那等待nvme返回结果的步骤肯定就是`nvme_process_cq`干的事了哈。这个函数里面就一个循环判断CQ队列是否有新来的completion command，有就处理了，没有就退出了。判断的依据就是phase字段啦，协议里面有规定。封装的函数是`nvme_cqe_valid`，很多地方会调它。特别说明nvme的中断处理函数`nvme_irq_check`中也会调用这个函数去判断该中断信号是不是nvme设备产生的。中断处理函数又是在`queue_request_irq`这个函数里面去使用`pci_request_irq`注册pci设备中断的。在创建或者配置nvme queue的时候会设置。<br><br>

> 2、`nvme_reset_work`中除了上面讲的操作注册了数据结构`nvme_dev`中的`admin_tagset`字段之外，还注册了另一个`tagset`字段这两者都是blk_mq_tag_set结构，功能类似。注册时候的代码逻辑也类似，先判断一下tagset是否空，空的时候才去设置若干字段。设置的ops字段为`nvme_mq_ops`这个就是tagset的request queue操作集。那到底这两者有撒区别呢，干嘛整了两个blk_mq_tag_set出来。开始我也没看出来，直到看到设置timeout字段时，两个地方用的宏不一样才瞬间明白了，admin_tagset设置timeout用的`ADMIN_TIMEOUT`，而tagset设置timeout用的`NVME_IO_TIMEOUT`，这个字段字面意思就很明显了，前面用于下发nvme的admin command，后者用于下发nvme的io command，是想把控制流和io流的处理分开，这样的设计确实更加独立。这个在`nvme_dev`中的`tagset`字段被`nvme_ctrl`中的指针，也叫`tagset`所引用，这个tagset后面会讲他是给块设备驱动内的gendisk的queue使用的，位于结构`nvme_ns`中的`queue`字段，作用就是io请求的request queue的操作结构啦。在使用的时候，类似提交配置命令，不过提交的request queue换成了ns中的queue而已。具体可以看看`nvme_submit_io`的实现。<br>
>> 2-1、在接着看代码，分析一下io处理的`nvme_mq_ops`咋实现的，一看发现居然很多操作是和admin一样的，完全复用了，比如请求的下发、完成、超时、init request处理都是一个函数。admin使用的init_hctx和exit_hctx又很简单，没撒特别处理。io呢，根本没有注册exit_hctx接口，只注册了init_hctx不过也实现很简单。具体这两个接口有撒作用还没整明白，不过应该影响很小，暂不深究。<br>
>> 2-2、io tagset相对admin tagset多了两个接口，map_queues和poll。看过LDD3后，再回头看这两个实现就没什么难度了，第一个函数就是用于DMA内存映射的，其实撒也没干，就是解析一下上下文再传给`blk_mq_pci_map_queues`，这种操作其实也是通用做法，LDD3中有提到过为什么非要具体的设备驱动程序来打这一波酱油，因为数据结构的解析依赖于具体的设备驱动自定义数据结构，这里就是strcut nvme_dev啦；至于poll，就更简单了，LDD3中专门讲了轮询操作的实现，这个函数就是nvme支持轮询io的内核态接口，逻辑其实很简单，就是用前面提到过的`nvme_cqe_valid`,再配上一个while循环，搞定。<br>
>> 2-3、最新代码里面使用的块设备层接口和LDD3上所讲的完全不一样了，应该是最新版本又改动过这部分代码。猜测现在是用`blk_mq_tag_set`这个结构作为queue操作信息的载体,生成request queue是`ns->queue = blk_mq_init_queue(ctrl->tagset)`，而LDD3上讲的原型是`request_queue_t *blk_init_queue(request_fn_proc *request, spinlock_t *lock)`；其他很多接口都有改动。<br><br>

> 3、代码看到这里，基本的流程都整明白，块设备驱动最关键的两个request queue也现身分析完毕了。但是还是隐隐觉得哪里不对，nvme不是块设备吗，怎么现在分析的代码只出现过字符设备驱动啊，块设备驱动结构呢？想到这，从我们刚刚分析的admin request queue反向找回去，肯定是能找到的吧！按照这个思路一搜，确实就找到了，nvme块设备驱动的注册位置在哪呢？居然在`nvme_scan_work`里面，它我们前面分析过，启动nvme设备的一堆work queue里面就有它。它内部的操作是遍历nvme设备的所有namespace，发现某个ns不存在就会用`nvme_alloc_ns`去注册一个块设备并加入到ns链表中，所以nvme设备的namespace才是一个块设备，而nvme设备是被视为了一个字符设备。这个块设备的内存结构为`nvme_ns`，操作集则是`nvme_fops`，一堆函数里面重点看看`nvme_ioctl`，这个函数和最开始注册的字符设备驱动的`nvme_dev_ioctl`的名称和内部实现都很像，看到这里我已经有点懵了，怎么整了两套ioctrl出来，实现也像。为什么要整两套出来，暂时没想通。但是两者还是有一点区别的，字符设备处理了NVME_IOCTL_ADMIN_CMD、NVME_IOCTL_IO_CMD、NVME_IOCTL_RESET、NVME_IOCTL_SUBSYS_RESET、NVME_IOCTL_RESCAN；而块设备处理了NVME_IOCTL_ADMIN_CMD、NVME_IOCTL_IO_CMD、NVME_IOCTL_ID、NVME_IOCTL_SUBMIT_IO。<br>
>> 3-1、块设备结构`nvme_ns`内部包含了`request_queue`,`gendisk`这两个块设备驱动必要成员。request_queue就用nvme_ctrl的tagset（是个指针，指向的是nvme_dev结构中的那个tagset）创建出来的，所以这里才去创建了nvme设备io请求的request queue哦。disk里面就会注册这个块设备的操作集`struct block_device_operations nvme_fops`，按照块设备的实现机制，这个操作集里面是不包含读写操作的，读写通过前面的queue来下发。而下发的实现就是通过`nvme_submit_io`这个函数来实现的，前面也已经提到过了。<br>
>> 3-2、NVME_IOCTL_SUBMIT_IO，NVME_IOCTL_ADMIN_CMD 这些命令宏是定义在公共头文件内的，所以用户态程序可见。猜测用户态下发命令请求，io请求的时候，应该都是通过用户态的ioctrl来实现的，连读写请求也是哦，因为仔细看`nvme_submit_io`这个函数内的实现，是将用户态的io请求参数拷贝到了内核空间，并且使用了里面的addr字段，表示数据所在的地址。<br>

***

# nvme/target/ 这个目录下的文件整的还不是很清楚，目前从看的部分代码上猜测这一些代码是为NVME OVER FABRIC实现的。完成fc协议到nvme的转换。因为fc协议是分initor和target的，这里应该就是fc的target端实现了。下面的这几个函数主要在`nvmet_req_init`中调用，去分别解析处理不同的command。
- nvmet_parse_connect_cmd
- nvmet_parse_io_cmd
- nvmet_parse_fabrics_cmd
- nvmet_parse_discovery_cmd
- nvmet_parse_admin_cmd

