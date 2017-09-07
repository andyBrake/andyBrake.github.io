# nvme 协议1.3版学习笔记：
- nvme协议规定了nvme的命令格式，command标准情况占用64Byte（可设置SQES字段变更），前4个Byte为command Dword0，为通用格式。命令分为Admin Command 10个，IO Command 3个（read，write，flush）；Admin/IO command大小为64B，对应的Completion大小为16B。
- NVMe 规范定义的 class code 是 0x010802
- nvme 设备只使用PCIE HEADER（共64Byte）中的BAR0，BAR1两个寄存器，合并为一个64bit的寄存器使用。该寄存器内的base address中用64-14个bit描述nvme设备的寄存器基地址（即地址一定是一个64K对齐的位置）。这些寄存器在协议的3.1节定义。
- BAR0 BAR1确认的地址内划分了多个寄存器，比如CAP,VS,INTMS,INTMC,CC,CSTS,AQA,ASQ等等。
- IVMS 寄存器读的时候返回的是nvme设备中断掩码，而不是寄存器的值。
- CC寄存器是配置寄存器，主机程序必须设置若干字段才能正常使用nvme设备，包括：CC.AMS, CC.MPS, CC.CSS, CC.EN, CC.IOCQES, CC.IOSQES，设置这些字段的值可能需要参考CAP中返回的值，否者可能会导致未定义行为。
- 如果controller完成过shutdown（CC.SHST变成了10b），则再次下发命令前需要reset一次(清零CC.EN字段)；CC.RDY字段变为1之后，才表示controller可以开始SQ doorbell写。
- SQT,CQH只用了16个bit用于表示指针，是不是位数不够哦？理论上应该有64bit才够啊！----看了内核代码后（__nvme_submit_cmd）搞清楚了，文档中的pointer不应该理解为指针，而是queue下标，从0开始计数。16bit正好可以对应IO queue的最大深度64k。每次提交这个值1就行了；Admin 的SQ，CQ队列深度是2至4K。
- controller用current head entry pointer指向的是下一个即将被处理的entry。当一个entry处理完成后，再增加游标。
- 主机软件一定要保证先创建CQ成功，再创建关联SQ。删除时需要先删除SQ，再删除CQ。
- 主机写非法值给两种doorbell都会触发异步事件。(p49)
- 主机是不可见CQ tail的，controller才可见，controller用SQHD pointer字段告诉主机controller处理到哪个entry了。CQ entry中Phase字段标识该entry是不是新放入的，默认是0表示是老entry，新插入的会置1（_协议中说的是controller每次对新entry翻转该字段，看了内核代码是用判是否为1来确认新entry的_）.
- The queue is Empty when the Head entry pointer equals the Tail entry pointer;The queue is Full when the Head equals one more than the Tail.
- namespace:一个NVMe命令一共64字节，其中第4到第7个Byte指定了要访问的NS,协议中叫NSID。（Copyright: www.ssdfans.com 蛋蛋读NVMe之六 版权所有，转载请注明来自 http://www.ssdfans.com/?p=1459）
- nvme协议规定使用queue进行命令交互，分为admin queue和IO queue两种，每种queue都分为submission 和completion两种queue。其中admin queue全系统只能有一个；IO queue一般是一个core一个（最多可以有64K个），如果一个core分配多个IO submission queue（但是只对应一个completion queue）则一般用于qos任务分级处理；Admin SQ/CQ的队列深度是2至4K；而IO SQ/CQ的队列深度是2至64K。每个queue都有一个16bits的唯一标识作为queue的ID。
***
- COMMAND的DWORD0 中PSDT字段用于表示本命令是用PRP还是SGL形式传输数据。NVME OVER PCIE一般使用PRP作为载体；字段FUSE可用于复合命令；字段OPC是最重要的，表示这个command的OPCODE。
- COMMAND的format在page 53，figure 11定义。其中NSID字段标识本次command用于的namespace。MPTR，DPTR字段分别用于命令中的metadata和data的载体描述。比如使用PRP时，DPTR的39:24Byte用于表示PRP entry（不跨page则只使用低8字节）。具体PRP的结构在page 54，figure 13中，它所描述的地址一定是4字节对齐的。PRP可以组链，结构参考figure 15.
- metadata区域具体是干撒的？ 4.5节描述没整清楚。
- completion queue entry的定义在4.6节，一个entry至少占用16 Byte，其中前4字节（DW0）是有command差异的，DW1保留，DW2和DW3是通用的，参见协议page 62.
- CMBSZ 寄存器可以用来设置controller内置内存的用途，即CMB的用途，可以用于queue内存也可以用于PRP,SGL内存。
- command的选择和执行是不保序的：The controller may select candidate commands for processing in any order. The order in which commands are selected for processing does not imply the order in which commands are completed.
- command 的仲裁机制有Round Robin Arbitration，Weighted Round Robin with Urgent Priority Class Arbitration，Vendor Specific Arbitration 三种。
***

# nvme命令执行流程 （page 201）
1. host提交命令到SQ
2. host写doorbell
3. controller fetch 一批command
4. controller 乱序执行command
5. controller 把执行结果写入CQ
6. controller 产生中断通知host
7. host 处理CQE，注意这里可能有多个CQE需要处理，所以代码级别是有一个while循环处理的，一直处理到发现phase不满足的CQE结束
8. 最后一步，host 写CQ 的doorbell，通知controller。注意第七步可能处理了多个CQE，而这里只需要写一次doorbell。

# 提交Command
1. host 构造一个command：CDW0.OPC, CDW0.FUSE, CDW0.CID, CDW1.NSID, MPTR, PRP, CDW10 - CDW15的命令差异字段设置。
2. host写SQ的doorbell

# host处理CQ
1. host 从对应CQ中读出一个CQE
2. host 解析CQE，根据DW2.SQID, DW3.CID可以得到SQ id和command id。
3. 解析DW3.SF可以得到command的执行结果。
4. host 更新CQ的游标，写对应doorbell。清理中断。
5. 如果command执行失败，这里做recover操作。

# nvme设备的初始化流程
1. 设置PCI 寄存器，bar0，bar1中的那一堆
2. host等待设备之前的可能reset操作，等待CSTS.RDY 变0
3. 配置admin queue相关寄存器AQA， ASQ， ACQ
4. controller setting：CC.AMS CC.MPS CC.CSS
5. 使能设备 CC.EN 置1
6. host等待设备ready，CSTS.RDY变1
7. controller identify，然后对每一个namespace进行identify
8. 使用set feature命令配置IO SQ&CQ个数以及完成中断寄存器的配置
9. host准备好IO CQ的内存资源，使用create I/O completion queue的命令创建好CQ
10. host准备好IO SQ的内存资源，使用create I/O submission queue的命令创建SQ
11. 异步事件通知机制的配置

# normal shutdown流程
1. 确保host不再下发任何 IO COMMAND，并且允许任何正在处理中的command完成
2. host删除所有的IO SQ，成功的删除会将所有等待执行的command abort掉
3. host删除所有的IO CQ
4. host设置CC.SHN域为01b，当controller更新CSTS.SHST为10b时标志shutdown流程完成了

# Asynchronous Event Requests 这个功能还没整明白

# keep alive命令利用watchdog的timer提供了一种心跳机制，确保host和controller之间能及时感知故障。NVME over PCIe不需要keep alive机制。。。。

# 读写命令
- identify命令会上报namespace所支持的logic block size，一般是512byte。block是读写命令所能操作的最小单元。
- namespace id简称NSID，编号1开始，0是非法的，全F是广播地址。具体查看page 173.
- Namespace Size >= Namespace Capacity >= Namespace Utilization，其中的Capcity是用于thin模型的，不过看解释有点没看懂他的定义。page 174
- controller层面是不会对读写命令保序的，需要host或者业务层面自己保证。
- Identify Controller data structure中会表明各种情况下的原子写粒度，单位是block.
- Flush command会使得所有命令在他之前完成，相当于对command刷盘
- directive具体是撒特性，还没整清楚。从google的结果上看，是一种将数据聚合写入（stream流形式）的方法，这样数据的写入和擦除可以尽量在整block上操作，降低gc的消耗。`With directives (multi-streaming) in NVMe, streams allow data written together on media so they can be erased together which minimizes garbage collection resulting in reduced write amplification as well as efficient flash utilization.` - from [thislink](http://blog.seagate.com/intelligent/a-review-of-nvme-optional-features-for-cloud-ssd-customization/)


