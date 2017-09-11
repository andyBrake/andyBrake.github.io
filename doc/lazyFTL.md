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

***

# paper中伪代码解析
## Algorithm 1 Convert block B ： CBA|UBA －> DBA 
  1. line 1-2:初始化mapping_pages, update_entries 两个集合为空
  2. line 3- 11: 对于B中的每一个有效page P，先将P所在的映射项E从UMT中移除，然后若P对应的update flag为1，则将E所在的mapping page P\`加入到集合mapping pages中，将E加入集合updata_entries中。
  3. line 12-17: 对于UMT全集中的所有映射项进行处理，如果E所在的mapping page在集合mapping pages中，且E对应的update flag为1，则将update flag清零，并将该E加入到集合update_entries中。
  4. line 18: 对update_entries集合进行排序去重处理，mapping_pages这个集合此时已经不再需要了
  5. line 19-32:对于集合update_entries中的每一个entry进行如下处理：根据映射项page的逻辑号LPN对mapping page容量取模得到映射项entry在mapping page中的offset。如果entry对应的invalid flag为1，则该mapping page中的offset对应的entry项置无效，再清0对应invalid flag。然后，该映射项的指向为PPN（这一步感觉是不是不需要啊，映射项本来就是LPN:PPN）。这里invalidate P\`\`\[offset\] 这一句伪代码还有一种理解是修改的GMT中映射项，但是这里P\`\`很明显确定是一个mapping page页面，所以这个解释讲不通。```P′′[offset] ← PPNe′′``` 这一步操作感觉确实没必要，因为下一段代码直接将整个P干掉了。再看了一下invalid flag的定义，__感觉这里这种写法就是在修改GMT__，不然整个流程确实没有修改GMT的地方了，如果是这种解释，那么invalidate P\`\`\[offset\] 这句又是多余的了，因为后面又去设置了PPN给这个映射项赋值。？？？？
  6. line 27-31:最后，对于entry所在的mapping page如果没有再更新的情况了，则将这个mapping page写入MBA，更新对应GMD，将这个mapping page标记为无效，即在UMT中不会在用它了。
  7. 整个convert流程中，没有设置两个bitmap为1的步骤，只有清零的步骤。UMT的mapping page只存在于cache中，所以对其的invalid操作应该很简单。
  8. convert cost指一个block中valid page所使用的映射项所占用的不同mapping page个数。比如一个block中的valid page占了64个映射项，这64个映射项分布在2个mapping page中存储，则该block的convert cost为2.
  9. 一次convert操作，会将victim block在UMT的所有映射项全部干掉，而在GMT和MBA中写入这些映射关系。victim block的物理位置没有改变，也没有被写入或者erase任何东西，只是由于管理他的映射项都变了，所以这个victim从逻辑上由UBA的一个block变为了DBA中的一个block。受到波及的UBA中其他block可能部分或者全部的映射项也被更新进GMT，所以一次convert操作，可能实际最后convert了多个UBA中的block。
  10. UMT中的映射项entry只可能在convert的时候被移除，跟这个entry撒时候update没关系。（page 5中原文）

***

## Algorithm 2 GC B ： DBA | MBA -> erased 
  1. GC cost定义 ：（read_latch + write_latch） * valid_page_number_IN_block + erase_latch 表示一个block的GC消耗。
  2. line 1-5 : 如果B是一个mapping block,则将B中的所有有效page move 到MBA区域。这里的move操作可能隐含的有read, modify, write这几步操作；move完成后，erase B即完成了GC操作。
  3. line 6-22 : 如果B不是一个mapping block（是data block）则操作复杂多了：对B中所有的valid page执行如下操作：
   > 首先判断LPN是否在UMT有映射项，如果有则将对应的invalid flag标记位清零，然后就完了直接到4。注意此时的情况是B在DBA中，而在UMT中有映射项，说明这个B一定被overwrite了，这个B中的这个valid page是老数据（经过convert从UBA中转到DBA中的，然后相同lba又被写入新数据），新数据存在UBA中呢，所以UMT中有映射项。所以这种情况的处理比较简单，直接干掉这个page就行了，如果整个B中的page都满足这个条件则整个B可以直接erase，因为它上面没有存任何真正有效的数据了。invalid flag需要无条件清零也很清楚，因为这个page所在的B将会被erase。
   > 如果LPN在UMT中没有映射项，则说明B的这一个page没有被overwrite过。首先，找到CCB；然后move这个page到CCB中，注意这里的move操作和前面考虑的一样；再然后，把这个page的映射关系加入到UMT中，因为现在这个logic page的物理位置是在CBA中了；最后，将这个page对应的update flag置1，invalid flag清零。update flag需要置1是因为这里把映射项插入了UMT，那么GMT一定是需要进行同步更新的，所以这里要标记好；invalid flag清零很明显，因为这个logic page在GMT中指向的physical page所在的block B马上就要被erase了。
  4. line 23-24 : erase B，将B放入空闲block pool

***

## Algorithm 3 Write page P 
