阅读源代码[nvmeqemu](../../nvmeqemu)的学习记录

# 重要的数据结构

  1. `PCIDeviceInfo` 描述 PCI设备，内部包含结构`DeviceInfo`，可以理解为继承了这个结构，`DeviceInfo`为设备模型的基类。
  2. `NVMEState` 这个结构描述nvme设备的模型，包含了内部寄存器，SQ&CQ, namespace， feature等等信息。其内部包含（继承）了`PCIDevice`,而`PCIDevice`又继承自`DeviceState`,

# 入口
  1. 从./hw/nvme.c开始，这个文件是nvme设备注册的入口，注册函数为`nvme_register_devices`,主要的信息在结构`nvme_info`中。这个函数注册的作用主要是将nvme设备挂入到PCI总线上去，通过单链表的形式。 注册的时候，函数内部会初始化完成一些父类对象的初始化，即把`DeviceInfo`初始化一下。
  2. 在nvme_info 定义的地方，有一个`.qdev.props`的字段初始化，非常绕，猜测作用是初始化了`NVMEState`的两个字段`num_namespaces`和`ns_size`的值。
  3. `NVMEState`这个结构怎么和`PCIDeviceInfo`连接起来的呢？`PCIDeviceInfo`包含 `DeviceInfo`，`DeviceInfo`字段`size`设为`sizeof(NVMEState)`.`DeviceInfo`字段`props`中的offset设为`NVMEState`中的相应字段偏移。
  4. 注册的reset操作实际为一个空函数，读写配置寄存器的函数为调用pci的接口。真正的工作体现在`pci_nvme_init`，其对应的退出函数为`pci_nvme_uninit`完成若干资源的释放。
  5. 重点分析`pci_nvme_init`，这个函数入参是`PCIDevice *pci_dev`，还不清楚是从哪来的。先将pci_dev结构的父结构即`NVMEState`找到。

 * 记录启动时间到`start_time`中。
 * 判断了namespace个数和ns size的合法性。
 * 增加instance计数，并记录当前设备的instance值。
 * 分配DiskInfo结构给disk字段，相当于nvme设备容器载体，读写的数据都放这个里的。这个结构比较重要。
 * 初始化SQ和CQ的`NVMEIOSQueue`结构，默认是有64对queue的。
 * 读配置文件，设置bar size，中断等。
 * 注册MMIO，调用的是`cpu_register_io_memory`和`pci_register_bar`，这一步的作用猜测是映射nvme设备的mem空间给cpu读写。内部细节还没分析。
 * 分配寄存器内存，相当于用内存模拟了nvme设备中的各种reg。
 * 设置`struct nvme_features`中的各种feature，很多值都是nvme协议规定的默认值。设置了identify信息和firmware slots，基本也是一些默认值。
 * 初始化第四步中分配出来的DiskInfo结构，里面会创建两个img file模拟flash mem，一个用于data数据，一个用于meta数据。
 * 设置异步事件的queue队列和timer，给异步事件的command使用。
 * 还设置了另外一个timer，给`sq_processing_timer`使用的，分析了一下代码的流程，推测这个timer的作用是模拟doorbell的机制，本来nvme设备是host写SQ后，触发doorbell，设备取出SQE执行。这个模拟器应该是定时去检查是否有SQE，有就取出一批执行。timer的执行函数为`sq_processing_timer_cb`。
  
 6. google了一下qemu的设计，qemu会模拟一个guest os的时钟中断，类似于上文提高的timer模拟中断。在`sq_processing_timer_cb`中会检查所有SQ是否有SQE存在，如果有，则开始取出执行，取的逻辑是没有实现nvme协议规定的多种仲裁机制的，而是直接从id 0（admin SQ）的SQ开始取，如果这个SQ非空，则继续在这个SQ取出执行，直到这个SQ空了再去处理一下SQ。每一次中断中，最多处理ENTRIES_TO_PROCESS个SQE，超过则退出中断，等待下一次中断再处理；如果把所有SQ都处理完了，那么就会把定时器del掉，使用`qemu_del_timer`，看了一下这个函数内部并不会释放这个timer，只是暂时deactive一下这个timer。 
 7. 在另外的流程中`nvme_mmio_write`提供给host用作写nvme 寄存器和SQ用。这套接口在发现写的地址是SQ时会调用到`process_doorbell`去处理，这个函数是会被写SQ和写CQ两套流程复用的，host只会写SQ，函数内部实现的最后会去判断当前的timer是否被deactive了，如果是那么会重启启动这个timer的。这样6中描述的流程就会被继续拉起。
 8. 函数`process_doorbell`还有处理CQ的流程，里面会判断条件后产生async event，模拟了nvme设备的async event机制。里面还有个额外的处理分支，即CQ满时，将timer的触发时间延后了5us，为什么要这样处理，暂时没想通。
