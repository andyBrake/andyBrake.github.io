#include <linux/module.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/blkdev.h>
#include <linux/blkpg.h>
#include <linux/delay.h>
#include <linux/io.h>

#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/dma.h>


#define SIZE (1024)

static DEFINE_SPINLOCK(ram_lock);
static int major;

typedef struct tag_vmem_device 
{
    struct gendisk *disk;
    struct request_queue *que;
    void *buf;
    spinlock_t lock;
    ssize_t size;
}vmem_device;

vmem_device g_ramBlockDev;

static void do_queue_request(request_queue_t *q)
{
    struct request *req;
    static int cnt = 0;

    while ((req = elv_next_request(q)) != NULL) {
        unsigned long start = req->sector << 9;
        unsigned long len  = req->current_nr_sectors << 9;

        if (start + len > (SIZE * 512)) {
            printk( KERN_ERR  ": bad access: block=%lu, count=%u\n",
                req->sector, req->current_nr_sectors);
            end_request(req, 0);
            continue;
        }
        
        if (READ == rq_data_dir(req)) {
            memcpy(req->buffer, (const void *)(g_ramBlockDev.buf + start), len);
        } else {
            memcpy(g_ramBlockDev.buf+start, (const void *)req->buffer, len);
        }
        
        end_request(req, 1);
        printk("func:%s, line:%d, cnt:%d, R(0)W(1):%d, start:%x, len:%x, sector:%x, nr:%x\n",
            __func__, __LINE__, cnt++, rq_data_dir(req), start, len, req->sector, 
            req->current_nr_sectors);
    }

        
}

static int ram_blk_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
    geo->cylinders=1;
    geo->heads=1;
    geo->sectors=BLK_SIZE/SECTOR_SIZE;

    return 0;
}


int ram_blk_open(struct block_device *pDev, fmode_t mod)
{
    printk("Open block device!\n");
}

int ram_blk_ioctl(struct block_device *, fmode_t mode, unsigned op, unsigned long sz)
{
    printk("Ctl block device, op %u, sz %lu!\n", op, sz);
}

static struct block_device_operations vmem_fops={

.owner=THIS_MODULE,

.getgeo=ram_blk_getgeo,

.ioctl=ram_blk_ioctl,

.open=ram_blk_open,

};

static int ramblock_init(void)
{
    /*
        1.分配一个gendisk结构体
        2.设置一个队列,将我们的请求放到队列里去
        3.设置这个gnedisk结构体的属性,如容量等
        4.add_gendisk函数
        5.另外需要自己分配一块内存空间用来当做块设备,在request函数中memcpy访问
    他,模仿块设备读写

    --------怎么进行测试
    1. insmod ramblock.ko
    2. 格式化: mkdosfs /dev/ramblock
    3. 挂接: mount /dev/ramblock /tmp/
    4. 读写文件: cd /tmp, 在里面vi文件
    5. cd /; umount /tmp/
    6. cat /dev/ramblock > /mnt/ramblock.bin
    7. 在PC上查看ramblock.bin
    sudo mount -o loop ramblock.bin /mnt
    */
    major = register_blkdev(0, "ramblock");

    if (major < 0) {
        printk("fail register block device!\n");
    }
    /* 分配磁盘 */
    g_ramBlockDev.disk =  alloc_disk(16);
    if (!g_ramBlockDev.disk){
        printk("alloc disk fail!\n");
        return -1;
    }

    g_ramBlockDev.que = blk_init_queue(do_queue_request, &g_ramBlockDev.lock);
    if (!g_ramBlockDev.que) {
        printk("queue is init fail!\n");
        return -1;
    }

    g_ramBlockDev.buf = kzalloc(SIZE * 512, GFP_KERNEL);
    if (!g_ramBlockDev.buf) {
        printk("alloc failed!\n");
        return -1;
    }
    g_ramBlockDev.disk->major = major;
    g_ramBlockDev.disk->first_minor = 0;
    g_ramBlockDev.disk->fops = &ram_fops;
    sprintf(g_ramBlockDev.disk->disk_name, "ramblock");

    g_ramBlockDev.disk->queue = g_ramBlockDev.que;
    set_capacity(g_ramBlockDev.disk, SIZE);
    add_disk(g_ramBlockDev.disk);

    printk("Ram Blk Init!\n");

    return 0;
}

static void ramblock_exit(void)
{
    if (unregister_blkdev(major, "ramblock")) {
        printk( KERN_ERR ": unregister of device failed\n");
    }
    del_gendisk(g_ramBlockDev.disk);
    put_disk(g_ramBlockDev.disk);

    blk_cleanup_queue(g_ramBlockDev.que);
}

module_init(ramblock_init);
module_exit(ramblock_exit);

MODULE_LICENSE("GPL");