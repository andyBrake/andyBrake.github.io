SLC（Single-Level Cell）即1bit/cell，速度快寿命长，价格昂贵（约MLC 3倍以上的价格），约10万次擦写寿命；

MLC（Multi-Level Cell）即2bit/cell，速度一般寿命一般，价格一般，约5000---10000次擦写寿命；
TLC（Trinary-Level Cell）即3bit/cell，有的Flash厂家也叫8LC，速度慢，寿命短，价格便宜，约500次擦写寿命，目前还没有厂家能做到1000次。

        TLC芯片是 X3(3-bit-per-cell)技术架构，是SMLC和MLC技术的延伸。最早期NAND Flash技术架构是SLC(Single-Level Cell)，原理是在1个存储器储存单元(cell)中存放1位元(bit)的数据，直到MLC(Multi-Level Cell)技术接棒后，架构演进为1个存储器储存单元存放2位元。 
2009年TLC架构正式问世，代表1个存储器储存单元可存放3位元，成本进一步大幅降低。
        如同上一波SLC技术转MLC技术趋势般，这次也是由NAND Flash大厂东芝(Toshiba)引发战火，之后三星电子(Samsung Electronics)也赶紧加入战局，使得整个TLC技术大量被量产且 应用在终端产品上。TLC芯片虽然储存容量变大，成本低廉许多，但因为效能也大打折扣，因此仅能用在低阶的NAND Flash相关产品上，象是低速快闪记忆卡、小型记忆卡microSD或随身碟等。象是内嵌世纪液体应用、智能型手机(Smartphone)、固态硬碟 (SSD)等技术门槛高，对于NAND Flash效能讲求高速且不出错等应用产品，则一定要使用SLC或MLC芯片。

       读写速度：采用H2testw v1.4测试，三星MLC写入速度: 4.28-5.59 MByte/s，读取速度: 12.2-12.9 MByte/s。三星SLC写入速度: 8.5MByte/s，读取速度: 14.3MByte/s。
        需要说明的闪存的寿命指的是写入(擦写)的次数，不是读出的次数，因为读取对芯片的寿命影响不大。下面是SLC、MLC、TLC三代闪存的寿命差异：
        SLC 利用正、负两种电荷  一个浮动栅存储1个bit的信息，约10万次擦写寿命。
        MLC 利用不同电位的电荷，一个浮动栅存储2个bit的信息，约5K-1W次擦写寿命，SLC-MLC【容量大了一倍，寿命缩短为1/10】。
        TLC 利用不同电位的电荷，一个浮动栅存储3个bit的信息，约500-1000次擦写寿命，MLC-TLC【容量大了1/2倍，寿命缩短为1/20】。
SLC
    SLC英文全称(Single Level Cell——SLC)即单层式储存 。主要由三星、海力士、美光、东芝等使用。
　　SLC技术特点是在浮置闸极与源极之中的氧化薄膜更薄，在写入数据时通过对浮置闸极的电荷加电压，然后透过源极，即可将所储存的电荷消除，通过这样的 方式，便可储存1个信息单元，这种技术能提供快速的程序编程与读取，不过此技术受限于Silicon efficiency的问题，必须要由较先进的流程强化技术(Process enhancements)，才能向上提升SLC制程技术。

MLC

       MLC英文全称（Multi Level Cell——MLC)即多层式储存。主要由东芝、Renesas、三星使用。

　　英特尔（Intel）在1997年9月最先开发成功MLC，其作用是将两个单位的信息存入一个Floating
Gate（闪存存储单元中存放电荷的部分），然后利用不同电位（Level）的电荷，通过内存储存的电压控制精准读写。MLC通过使用大量的电压等级，每 个单元储存两位数据，数据密度比较大。SLC架构是0和1两个值，而MLC架构可以一次储存4个以上的值，因此，MLC架构可以有比较好的储存密度。

与SLC比较MLC的优势：
        签于目前市场主要以SLC和MLC储存为主，我们多了解下SLC和MLC储存。SLC架构是0和1两个值，而MLC架构可以一次储存4个以上的值， 因此MLC架构的储存密度较高，并且可以利用老旧的生产程备来提高产品的容量，无须额外投资生产设备，拥有成本与良率的优势。
与SLC相比较，MLC生产成本较低，容量大。如果经过改进，MLC的读写性能应该还可以进一步提升。

与SLC比较MLC的缺点：
        MLC架构有许多缺点，首先是使用寿命较短，SLC架构可以写入10万次，而MLC架构只能承受约1万次的写入。
其次就是存取速度慢，在目前技术条件下，MLC芯片理论速度只能达到６MB左右。SLC架构比MLC架构要快速三倍以上。
再者，MLC能耗比SLC高，在相同使用条件下比SLC要多15%左右的电流消耗。
        虽然与SLC相比，MLC缺点很多，但在单颗芯片容量方面，目前MLC还是占了绝对的优势。由于MLC架构和成本都具有绝对优势，能满足2GB、4GB、8GB甚至更大容量的市场需求。
        
        
        # 参考：
        [http://blog.chinaunix.net/uid-25314474-id-3034726.html](http://blog.chinaunix.net/uid-25314474-id-3034726.html)