# nvme 协议1.3版学习笔记：
- nvme协议规定了nvme的命令格式，command占用64Byte，前4个Byte为command Dword0，为通用格式。命令分为Admin Command 10个，IO Command 3个（read，write，flush）；Admin/IO command大小为64B，对应的Completion大小为16B。
- NVMe 规范定义的 class code 是 0x010802
- nvme 设备只使用PCIE HEADER（共64Byte）中的BAR0，BAR1两个寄存器，合并为一个64bit的寄存器使用。该寄存器内的base address中用64-14个bit描述nvme设备的寄存器基地址（即地址一定是一个64K对齐的位置）。这些寄存器在协议的3.1节定义。
- namespace:一个NVMe命令一共64字节，其中第4到第7个Byte指定了要访问的NS,协议中叫NSID。（Copyright: www.ssdfans.com 蛋蛋读NVMe之六 版权所有，转载请注明来自 http://www.ssdfans.com/?p=1459）
- nvme协议规定使用queue进行命令交互，分为admin queue和IO queue两种，每种queue都分为submission 和completion两种queue。其中admin queue全系统只能有一个；IO queue一般是一个core一个（最多可以有64K个），如果一个core分配多个IO submission queue（但是只对应一个completion queue）则一般用于qos任务分级处理；Admin SQ/CQ的队列深度是2~4K；而IO SQ/CQ的队列深度是2~64K。
***
