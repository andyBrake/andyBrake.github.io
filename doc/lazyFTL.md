LazyFTL 学习笔记

***

- The Flash Translation Layer (FTL) is a software layer built on raw flash memory that carries out garbage collection and wear leveling strategies and hides the special character- istics of flash memory from upper file systems by emulating a normal block device like magnetic disks. 

－ SSD的写单元是page，擦除的单元是block。


# 典型时延：
| type     | Read      | write    | erase       |
|---       |----       |-------   |-------      |
|Mag-Disk  |12.7ms     |13.7ms    | NA          |
|NAND-Flash|80us(2kb)  |200us(2kb)| 1.5ms(128kb)|


# 设计
1. 整个flash分为DBA，MBA，CBA，UBA 四大块；MBA不会保存数据，只存映射项相当于元数据（叫GMT），且有部分会放在SRAM做的cache中加快查询速度。
2. 由于只有部分GMT在cache中，所以还得有个GMD用于cache的映射项换入换出。GMD纪录了所有映射项所在的BLOCK，粒度较粗。
3. CBA和UBA中的数据由一个UMT的页级映射表管理。UMT占用空间比较小，可以使用btree或hash表等数据结构保证高效的增删查改。而且UMT只存在于cache中，不会固化到flash中。
4. CBA和UBA的比例会动态自动调整，如果热点数据比例增加，就是不停的数据发生更新，那么UBA将增加；如果是flash的空间占用率增加，即不停的写入新数据，那么CBA将增加。
5. update flag bitmap的bit表示指向的page是否需要在GMT中更新映射项，1表示需要。
6. invalide flag bitmap的bit表示GMT中映射项指向的page是否需要invalid。1表示是。
7. 这两个bitmap的下标，即bit的偏移代表的是撒，还没整懂？
8. covert操作的备选block从CBA或UBA中选择，规则是代价最小者。


# CBA&UBA可以自均衡

# covert操作：CBA|UBA －> DBA 

#update-flag 表示在GMT中映射关系是否需要更新，为1时说明数据一定再CBA&UBA中

#invalid-flag 表示GMT中entry指向的page是否已经无效，为0表示无效了？
