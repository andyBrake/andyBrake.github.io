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

  > 记录启动时间到`start_time`中。
  > 判断了namespace个数和ns size的合法性。
  > 增加instance计数，并记录当前设备的instance值。
  > 分配DiskInfo结构给disk字段，相当于nvme设备容器载体，读写的数据都放这个里的。这个结构比较重要。
  > 初始化SQ和CQ的`NVMEIOSQueue`结构，默认是有64对queue的。
  > 读配置文件，设置bar size，中断等。
  > 注册MMIO，调用的是`cpu_register_io_memory`和`pci_register_bar`，这一步的作用猜测是映射nvme设备的mem空间给cpu读写。内部细节还没分析。
  > 分配寄存器内存，相当于用内存模拟了nvme设备中的各种reg。
  > 设置`struct nvme_features`中的各种feature，很多值都是nvme协议规定的默认值。设置了identify信息和firmware slots，基本也是一些默认值。
  > 初始化第四步中分配出来的DiskInfo结构，里面会创建两个img file模拟flash mem，一个用于data数据，一个用于meta数据。
  > 设置异步事件的queue队列和timer，给异步事件的command使用。
  > 还设置了另外一个timer，给`sq_processing_timer`使用的，分析了一下代码的流程，推测这个timer的作用是模拟doorbell的机制，本来nvme设备是host写SQ后，触发doorbell，设备取出SQE执行。这个模拟器应该是定时去检查是否有SQE，有就取出一批执行。timer的执行函数为`sq_processing_timer_cb`。
