#include "platform.h"
#include "ks_drv.h"
#include "ks_dev.h"
#include "ks_api.h"
#include <asm-generic/gpio.h>

#if (HZ==1000)
#define DEFAULT_SCAN_DURATION       40
#define DEFAULT_REPEAT_DURATION     300
#elif (HZ==100)
#define DEFAULT_SCAN_DURATION       4
#define DEFAULT_REPEAT_DURATION     30
#else
#define DEFAULT_SCAN_DURATION       40
#define DEFAULT_REPEAT_DURATION     300
#endif

#define DEFAULT_CLOCK_DIVIDER       120

/* module parameter */
static unsigned int clk_div = DEFAULT_CLOCK_DIVIDER;
module_param(clk_div, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(clk_div, "clock divider (base freq. is 12MHz)");


/* function for register device */
void device_release(struct device *dev);

/* function for register driver */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
int driver_probe(struct platform_device * pdev);
int __devexit driver_remove(struct platform_device *pdev);
#else
int driver_probe(struct device * dev);
int driver_remove(struct device * dev);
#endif

extern int Trans_Keyscan_To_Gpio(int key_num);

//int scu_remove(struct scu_t *p_scu);

/* function for register file operation */
static int file_open(struct inode *inode, struct file *filp);
static int file_release(struct inode *inode, struct file *filp);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
static int file_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
#else
static long file_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#endif
static ssize_t file_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static unsigned int file_poll(struct file *filp, poll_table *wait);

static struct resource _dev_resource[] = {
	[0] = {
		.start = DEV_PA_START,
		.end = DEV_PA_END,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = DEV_IRQ_START,
		.end = DEV_IRQ_END,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device _device = {
	.name = DEV_NAME,
	.id = -1, /* "-1" to indicate there's only one. */
	#ifdef HARDWARE_IS_ON
	.num_resources = ARRAY_SIZE(_dev_resource),
	.resource = _dev_resource,
	#endif
	.dev  = {
		.release = device_release,
	},
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
/*
 * platform_driver
 */
static struct platform_driver _driver = {
    .probe = driver_probe,
    .remove = driver_remove,
    .driver = {
           .owner = THIS_MODULE,
           .name = DEV_NAME,
           .bus = &platform_bus_type, /* verify on GM8210 */
    },
};
#else
/*
 * device_driver
 */
static struct device_driver _driver = {
    .owner = THIS_MODULE,
    .name = DEV_NAME,
    .bus = &platform_bus_type,      /* used to represent busses like PCI, USB, I2C, etc */
    .probe = driver_probe,
    .remove = driver_remove,
};
#endif

static struct file_operations _fops = {
	.owner 			= THIS_MODULE,
    #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
	.ioctl 			= file_ioctl,
	#else
    .unlocked_ioctl = file_ioctl,
    #endif
	.open 			= file_open,
	.release 		= file_release,
	.read			= file_read,
	.poll       	= file_poll,
};


void device_release(struct device *dev)
{
    PRINT_FUNC();
    return;
}


int register_device(struct platform_device *p_device)
{
    int ret = 0;

    if (unlikely((ret = platform_device_register(p_device)) < 0)) 
    {
        printk("%s fails: platform_device_register not OK\n", __FUNCTION__);
    }
    
    return ret;    
}

void unregister_device(struct platform_device *p_device)
{
    
    platform_device_unregister(p_device);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
int register_driver(struct platform_driver *p_driver)
{
    PRINT_FUNC();
	return platform_driver_register(p_driver);
}
#else
int register_driver(struct device_driver* p_driver)
{
	return driver_register(p_driver);
}
#endif


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
void unregister_driver(struct platform_driver *p_driver)
{
    platform_driver_unregister(p_driver);
}
#else
void ft2dge_unregister_driver(struct device_driver *p_driver)
{
    driver_unregister(p_driver);	
}
#endif

/*
 * register_cdev
 */ 
int register_cdev(struct dev_data* p_dev_data)
{
    int ret = 0;

    /* alloc chrdev */
    ret = alloc_chrdev_region(&p_dev_data->dev_num, 0, DEV_COUNT, DEV_NAME);
    if (unlikely(ret < 0)) {
        printk(KERN_ERR "%s:alloc_chrdev_region failed\n", __func__);
        goto err1;
    }
    
    cdev_init(&p_dev_data->cdev, &_fops);
    p_dev_data->cdev.owner = THIS_MODULE;

    ret = cdev_add(&p_dev_data->cdev, p_dev_data->dev_num, DEV_COUNT);
    if (unlikely(ret < 0)) {
        PRINT_E(KERN_ERR "%s:cdev_add failed\n", __func__);
        goto err2;
    }

    /* create class */
    p_dev_data->class = class_create(THIS_MODULE, CLS_NAME);
	if (IS_ERR(p_dev_data->class))
	{
        PRINT_E(KERN_ERR "%s:class_create failed\n", __func__);
        goto err3;
	}

    
    /* create a node in /dev */
    #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	p_dev_data->p_device = device_create(
        p_dev_data->class,              /* struct class *class */
        NULL,                           /* struct device *parent */
        p_dev_data->cdev.dev,    /* dev_t devt */
	    p_dev_data,                     /* void *drvdata, the same as platform_set_drvdata */
	    DEV_NAME                 /* const char *fmt */
	);
    #else
	class_device_create(
	    p_dev_data->class, 
	    p_dev_data->cdev.dev,
	    NULL, DEV_NAME
	);
    #endif
    
    PRINT_FUNC();
    return 0;
    
    err3:
        cdev_del(&p_dev_data->cdev);
    
    err2:
        unregister_chrdev_region(p_dev_data->dev_num, 1);
        
    err1:
        return ret;
}


/*
 * unregister_cdev
 */ 
void unregister_cdev(struct dev_data* p_dev_data)
{
    PRINT_FUNC();

    #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
    device_destroy(p_dev_data->class, p_dev_data->cdev.dev);
    #else
    class_device_destroy(p_dev_data->class, p_dev_data->cdev.dev);
    #endif
    
    class_destroy(p_dev_data->class);
    
    cdev_del(&p_dev_data->cdev);
    
    unregister_chrdev_region(p_dev_data->dev_num, 1);
    
}

static void* dev_data_alloc_specific(struct dev_data* p_dev_data)
{
	struct dev_specific_data_t* p_data = &p_dev_data->dev_specific_data;

    spin_lock_init(&p_data->lock);

    p_data->scan_duration = DEFAULT_SCAN_DURATION;
    p_data->repeat_duration = DEFAULT_REPEAT_DURATION;
	/* setting fifo */
    p_data->queue_len = 16;
    if (unlikely(kfifo_alloc(
		&p_data->fifo, 
		p_data->queue_len * sizeof(keyscan_pub_data), 
		GFP_KERNEL))
	)
	{
		panic("kfifo_alloc fail in %s\n", __func__);
        goto err0;
	}
    kfifo_reset(&p_data->fifo);
    init_waitqueue_head(&p_data->wait_queue);
	
	/* setting work and workqueue */
    INIT_DELAYED_WORK(&p_data->work_st, restart_hardware);	
    init_MUTEX(&p_data->oper_sem);

	printk("dev_data_alloc_specific done\n");
    return p_data;


    err0:
    return NULL;
}

static void* dev_data_alloc(void)
{
    struct dev_data* p_dev_data = NULL;
        
    /* alloc drvdata */
    p_dev_data = kzalloc(sizeof(struct dev_data), GFP_KERNEL);

    if (unlikely(p_dev_data == NULL))
    {
        PRINT_E("%s Failed to allocate p_dev_data\n", __FUNCTION__);        
        goto err0;
    }

	if (unlikely(dev_data_alloc_specific(p_dev_data) == NULL))
    {
        PRINT_E("%s Failed to allocate p_dev_data\n", __FUNCTION__);        
        goto err1;
    }

   	DRV_COUNT_RESET();
	return p_dev_data;
	
	err1:
	    kfree (p_dev_data);
		p_dev_data = NULL;
    err0:
        return p_dev_data;
}

static int dev_data_free_specific(struct dev_data* p_dev_data)
{
    struct dev_specific_data_t* p_data = &p_dev_data->dev_specific_data;
    kfifo_free(&p_data->fifo);
    return 0;
}


static int dev_data_free(struct dev_data* p_dev_data)
{      
    dev_data_free_specific(p_dev_data);
    /* free drvdata */
    kfree(p_dev_data);
    p_dev_data = NULL;
    
	return 0;
}

static irqreturn_t drv_interrupt(int irq, void *base)
{
    unsigned int now_jiffies;
	unsigned int new_scan_dur;
	unsigned int    	status;
	unsigned int 		len;
	unsigned int 		vbase;
    static keyscan_pub_data     prev_data;
    keyscan_pub_data       		new_data;
    struct dev_data*	p_dev_data = (struct dev_data*)base;
    struct dev_specific_data_t*	p_data = &p_dev_data->dev_specific_data;

	vbase = (unsigned int)p_dev_data->io_vadr;
    new_data.key_i = 0;
    new_data.key_o = 0;
    new_data.status = 0;
    new_scan_dur = p_data->scan_duration;
    status = Keyscan_GetStatus(vbase);
    Keyscan_SetStatus(vbase, KS_STS_INT);  //write one clear
    if(status & KS_STS_INT)
    {
        now_jiffies = jiffies;
		
		new_data.key_i = Keyscan_GetData(vbase); 
		if (new_data.key_i & ~(p_data->key_i_bits))
		{
			panic("gpio_in error0x%08X\n", new_data.key_i);
		}
		
		new_data.key_o = Keyscan_GetDataIndex(vbase);
 		if (((1 << new_data.key_o) & p_data->key_o_bits) == 0)
		{
			panic("gpio_out error 0x%08X\n", new_data.key_o);
		}
		
        if(prev_data.status == 0)
            new_data.status = KEY_IN;
        else
        {
            if((new_data.key_i == prev_data.key_i) &&
			   (new_data.key_o == prev_data.key_o)
			)
            {
                if(jiffies_diff(now_jiffies, p_data->last_scan_jiffies) < p_data->scan_duration*2)
                {
                    /* Gavin comment it because it will cause repeat event isn't triggerred immediately */
                    if(jiffies_diff(now_jiffies, p_data->last_key_jiffies) > p_data->repeat_duration)
                        new_data.status = KEY_REPEAT;
                    else
                        goto KEYSCAN_INT_SKIP_FIFO;
                }
                else
                    new_data.status = KEY_IN;
            }
            else
            {
                new_data.status = KEY_IN;
                /* Gavin comment it because it causes detect period too long */
                new_scan_dur = p_data->repeat_duration;
            }
        }
    	len = kfifo_in(
			&p_data->fifo, 
			&new_data,
			sizeof(keyscan_pub_data)
			);

    	if (len != sizeof(keyscan_pub_data))
    	{
    	    printk("Put keyscan data into filo failed.\n");
    	    goto KEYSCAN_INT_SKIP_FIFO;
        }
        //wake_up(&p_data->wait_queue);

        prev_data.key_i = new_data.key_i;
        prev_data.key_o = new_data.key_o;
        prev_data.status = new_data.status;
		
        p_data->last_key_jiffies = now_jiffies;

KEYSCAN_INT_SKIP_FIFO:
        p_data->last_scan_jiffies = now_jiffies;
    }

    //delay to restart hardware
    schedule_delayed_work(&p_data->work_st,new_scan_dur);

    return IRQ_HANDLED;
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19))
irqreturn_t drv_isr(int irq, void *dev)
#else
irqreturn_t drv_isr(int irq, void *dev, struct pt_regs *dummy)
#endif
{

	return 	drv_interrupt(irq, dev);
}

int init_hardware(struct dev_data *p_dev_data)
{
	struct dev_specific_data_t* p_data = &p_dev_data->dev_specific_data;
	unsigned int base = (unsigned int)p_dev_data->io_vadr;
    int bit = 0 ,gpio_n;
	int ret = -1;
    //setup hardware    
    ret = scu_probe(&p_dev_data->scu);
    if (unlikely(ret != 0)) 
    {
        PRINT_E("%s fails at scu_probe %d\n", __FUNCTION__, ret);
        goto err0;
    }
	
	set_pinmux();

	p_data->key_i_bits = KEY_I;
	for_each_set_bit(bit, (const long unsigned int *)(&p_data->key_i_bits),
	                 BITS_PER_BYTE * sizeof(p_data->key_i_bits)) {
	                 
		gpio_n = Trans_Keyscan_To_Gpio(bit);
		
		printk("gpio in %d\n", gpio_n);
	    if ((ret = gpio_request(gpio_n, DEV_NAME)) != 0) {
	        printk(KERN_ERR "%s: gpio_request %u failed\n", __func__, bit);
	        goto err0;
	    }

	    if ((ret = gpio_direction_input(gpio_n)) != 0) {
	        printk(KERN_ERR "%s: gpio_direction_input failed %d\n", __func__, ret);
	        goto err0;
			
	    }	
    }
		
	p_data->key_o_bits = KEY_O;
    for_each_set_bit(bit, (const long unsigned int *)(&p_data->key_o_bits),
                     BITS_PER_BYTE * sizeof(p_data->key_o_bits)) {
                     
		gpio_n = Trans_Keyscan_To_Gpio(bit);
		
		printk("gpio out %d\n", gpio_n);
        if ((ret = gpio_request(gpio_n, DEV_NAME)) != 0) {
            printk(KERN_ERR "%s: gpio_request %u failed\n", __func__, bit);
	        goto err0;
        }
        if ((ret = gpio_direction_output(gpio_n, 1)) != 0) {
            printk(KERN_ERR "%s: gpio_direction_output %u failed\n", __func__, bit);
	        goto err0;
        }
    }
	
	printk("key_o_bits 0x%08X key_i_bits 0x%08X\n", p_data->key_o_bits,p_data->key_i_bits);
	Keyscan_SetOutPinSelection(base, p_data->key_o_bits);
    Keyscan_SetInPinSelection(base, p_data->key_i_bits);
    Keyscan_SetPinPullUpSelection(base, KEY_PULLUP);
    Keyscan_SetClockDiv(base, 0xFE<<16|(clk_div));
    Keyscan_SetStatus(base, KS_STS_INT); //write 1'b1 to clear INT bit

	return 0;

	err0:
    return ret;
}

void exit_hardware(struct dev_data *p_dev_data)
{
    struct dev_specific_data_t *p_data = &p_dev_data->dev_specific_data;
    int bit = 0 ,gpio_n;

    PRINT_FUNC();
	
	/****************************************************/
	/**** start to implement your exit_hardware here ****/
	/****************************************************/
	
	/* free gpio */
	for_each_set_bit(bit, (const long unsigned int *)(&p_data->key_i_bits),
	                 BITS_PER_BYTE * sizeof(p_data->key_i_bits)) {
	     gpio_n = Trans_Keyscan_To_Gpio(bit);            
         gpio_free(gpio_n);
	}
	for_each_set_bit(bit, (const long unsigned int *)(&p_data->key_o_bits),
	                 BITS_PER_BYTE * sizeof(p_data->key_o_bits)) {
         gpio_n = Trans_Keyscan_To_Gpio(bit);            
         gpio_free(gpio_n);
	}
    /* remove pmu */
    scu_remove(&p_dev_data->scu);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
int driver_probe(struct platform_device * pdev)
#else
int driver_probe(struct device * dev)
#endif
{
    #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
    struct platform_device* pdev = to_platform_device(dev); // for version under 2.6.24 
    #endif
    struct dev_data *p_dev_data = NULL;
    #ifdef HARDWARE_IS_ON
    struct resource *res = NULL;  
    #endif
    struct dev_specific_data_t *p_data = NULL;
  
    int ret = 0;
    PRINT_FUNC();
    
    /* 1. alloc and init device driver_Data */
    p_dev_data = dev_data_alloc();
    if (unlikely(p_dev_data == NULL)) 
    {
        printk("%s fails: kzalloc not OK", __FUNCTION__);
        goto err1;
    }
    p_data = &p_dev_data->dev_specific_data;
	
    /* 2. set device_drirver_data */
    #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
    platform_set_drvdata(pdev, p_dev_data);
    #else
    dev_set_drvdata(dev, p_dev_data);
    #endif

    p_dev_data->id = pdev->id;    
   
    /* 3. request resource of base address */
    #ifdef HARDWARE_IS_ON
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

    if (unlikely((!res))) 
    {
        PRINT_E("%s fails: platform_get_resource not OK", __FUNCTION__);
        goto err2;
    }

    p_dev_data->io_size = res->end - res->start + 1;
    p_dev_data->io_padr = (void*)res->start;
	
    if (unlikely(!request_mem_region(res->start, p_dev_data->io_size, pdev->name))) {
        PRINT_E("%s fails: request_mem_region not OK", __FUNCTION__);
        goto err2;
    }

    p_dev_data->io_vadr = (void*) ioremap((uint32_t)p_dev_data->io_padr, p_dev_data->io_size);

    
    if (unlikely(p_dev_data->io_vadr == 0)) 
    {
        PRINT_E("%s fails: ioremap_nocache not OK", __FUNCTION__);
        goto err3;
    }

    /* 4. request_irq */
    p_dev_data->irq_no = platform_get_irq(pdev, 0);

    if (unlikely(p_dev_data->irq_no < 0)) 
    {
        PRINT_E("%s fails: platform_get_irq not OK", __FUNCTION__);
        goto err3;
    }

    ret = request_irq(
        p_dev_data->irq_no, 
        drv_isr, 
        #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
        SA_INTERRUPT, 
        #else
        0,
        #endif
        DEV_NAME, 
        p_dev_data
    ); 

    if (unlikely(ret != 0)) 
    {
        PRINT_E("%s fails: request_irq not OK %d\n", __FUNCTION__, ret);
        goto err3;
    }
     
    /* 6. init hardware */
	ret = init_hardware(p_dev_data);
    if (unlikely(ret < 0)) 
    {
        PRINT_E("%s fails: init_hardware not OK\n", __FUNCTION__);
        goto err4;
    }
    #endif
    

    /* 7. register cdev */
    ret = register_cdev(p_dev_data);
    if (unlikely(ret < 0)) 
    {
        PRINT_E("%s fails: ft2dge_register_cdev not OK\n", __FUNCTION__);
        goto err5;
    }


    /* 8. print probe info */
    printk("%s done, io_vadr 0x%08X, io_padr 0x%08X 0x%08X\n", 
    __FUNCTION__, 
    (unsigned int)p_dev_data->io_vadr, 
    (unsigned int)p_dev_data->io_padr, 
    (unsigned int)p_dev_data
    );

    return ret;
    
    err5:
        scu_remove(&p_dev_data->scu);

    #ifdef HARDWARE_IS_ON
    err4:
        free_irq(p_dev_data->irq_no, p_dev_data);
        
    err3:
        release_mem_region((unsigned int)p_dev_data->io_padr, p_dev_data->io_size);

    err2:
        #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
        platform_set_drvdata(pdev, NULL);
        #else
        dev_set_drvdata(dev, NULL);
        #endif
    #endif
        

    err1:
        return ret;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
int __devexit driver_remove(struct platform_device *pdev)
#else
int driver_remove(struct device * dev)
#endif
{      
	struct dev_data* p_dev_data = NULL;

    PRINT_FUNC();
	
	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
	p_dev_data = (struct dev_data *)platform_get_drvdata(pdev);	
	#else
	p_dev_data = (struct dev_data *)dev_get_drvdata(dev);
	#endif
    PRINT_I("%s p_dev_data 0x%08X\n", __FUNCTION__, (unsigned int)p_dev_data);

    /* unregister cdev */
    unregister_cdev(p_dev_data);
    
    #ifdef HARDWARE_IS_ON
    /* remove pmu */
	exit_hardware(p_dev_data);
	
    /* release resource */
    iounmap((void __iomem *)p_dev_data->io_vadr); 
    release_mem_region((u32)p_dev_data->io_padr, p_dev_data->io_size);

    /* free irq */
    free_irq(p_dev_data->irq_no, p_dev_data);
    #endif
    
	/* free device memory, and set drv data as NULL	*/
	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
	platform_set_drvdata(pdev, NULL);
	#else
	dev_set_drvdata(dev, NULL);
	#endif

    /* free device driver_data */
	dev_data_free(p_dev_data);
	return 0;
}



static int __init module_init_func(void)
{
    int ret = 0;  

    printk("Welcome to use %s, 20120731\n", DEV_NAME );

    PRINT_FUNC();   
    
    
    /* register platform device */
    ret = register_device(&_device);
    if (unlikely(ret < 0)) 
    {
        PRINT_E("%s fails: ft2dge_register_device not OK\n", __FUNCTION__);
        goto err1;
    }

    /* register platform driver     
     * probe will be done immediately after platform driver is registered 
     */
    ret = register_driver(&_driver);
    if (unlikely(ret < 0)) 
    {
        PRINT_E("%s fails: register_driver not OK\n", __FUNCTION__);
        goto err2;
    }
    

   
    return 0;

        
    err2:
        unregister_device(&_device);		

    err1:
    return ret;

}

static void __exit module_exit_func(void)
{

    printk(" ************************************************\n");
    printk(" * Thank you to use %s, 20130325, goodbye *\n", DEV_NAME );
    printk(" ************************************************\n");

    PRINT_FUNC();   
    
    
    /* unregister platform driver */     
    unregister_driver(&_driver);


	/* register platform device */
    unregister_device(&_device);
}

static int file_open_specific(struct dev_data* p_dev_data)
{
	unsigned int base = (unsigned int)p_dev_data->io_vadr;


	Keyscan_SetControl(base, KS_CTRL_START_SCAN);
    return 0;
}

static int file_open(struct inode *inode, struct file *filp)
{
	int ret_of_ft2dge_driver_open = 0;
	
	struct flp_data* p_flp_data = NULL;	
	struct dev_data* p_dev_data = NULL;
	
    /* set filp */
    p_dev_data = container_of(inode->i_cdev, struct dev_data, cdev);
    
	p_flp_data = kzalloc(sizeof(struct flp_data), GFP_KERNEL);
	p_flp_data->p_dev_data = p_dev_data;

	
    /* increase driver count */	
	DRV_COUNT_INC(); 

	/* driver specific open */
	file_open_specific(p_dev_data);
	
	/* assign to private_data */
    filp->private_data = p_flp_data;	


    PRINT_I("\n");
	if (unlikely(((unsigned int)p_dev_data != (unsigned int)inode->i_cdev)))
	{
    	PRINT_I("%s p_dev_data 0x%08X 0x%08X\n", __FUNCTION__, (unsigned int)p_dev_data, (unsigned int)inode->i_cdev);
    	PRINT_I("%s p_flp_data 0x%08X\n", __FUNCTION__, (unsigned int)p_flp_data);
		ret_of_ft2dge_driver_open = -1;
	}
    return ret_of_ft2dge_driver_open;
}


#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
static int file_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#else
static long file_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
    #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
	int ret = 0;
    #else
    long ret = 0;
    #endif

	struct flp_data* p_flp_data = filp->private_data;	
	struct dev_data* p_dev_data = p_flp_data->p_dev_data;
	struct dev_specific_data_t* p_data = &p_dev_data->dev_specific_data;


    down(&p_data->oper_sem);

    switch(cmd)
    {
      case KEYSCAN_SET_SCAN_INTERVAL:
      {
        int   ms;
	    if (unlikely(copy_from_user((void *)&ms, (void *)arg, sizeof(int))))
		{
			ret = -EFAULT;
			break;
	    }
        if(ms>0)
            p_data->scan_duration = ms;
        else
            ret = -EFAULT;
        break;
      }
      case KEYSCAN_SET_REPEAT_INTERVAL:
      {
        int   ms;
 	    if (unlikely(copy_from_user((void *)&ms, (void *)arg, sizeof(int))))
		{
			ret = -EFAULT;
			break;
	    }

        if(ms>0)
            p_data->repeat_duration = ms;
        else
            ret = -EFAULT;
        break;
      }
      default:
        printk("%s cmd(0x%x) no define!\n", __func__, cmd);
        break;
    }

    up(&p_data->oper_sem);

    return ret;
}



static ssize_t file_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
 	unsigned long 		cpu_flags;
    int         ret = 0;
	keyscan_pub_data  	data;
	struct flp_data* p_flp_data = filp->private_data;	
	struct dev_data* p_dev_data = p_flp_data->p_dev_data;
	struct dev_specific_data_t* p_data = &p_dev_data->dev_specific_data;


    if (down_interruptible(&p_data->oper_sem))
        return -ERESTARTSYS;

	spin_lock_irqsave(&p_data->lock, cpu_flags);
    if (kfifo_len(&p_data->fifo)==0)
    {
    	ret = 0;
        goto exit;
    }
    
    if (unlikely(
		kfifo_out(
			&p_data->fifo, 
			(unsigned char *)&data,
			sizeof(keyscan_pub_data)
		) <= 0))
	{
    	ret = -ERESTARTSYS;
        goto exit;
	}

    if (copy_to_user(buf, &data, sizeof(keyscan_pub_data)))
    {
    	ret = -ERESTARTSYS;
        goto exit;
    }

    ret = sizeof(keyscan_pub_data);

	exit:
	spin_unlock_irqrestore(&p_data->lock, cpu_flags);
    up(&p_data->oper_sem);

    return ret;
}

static unsigned int file_poll(struct file *filp, poll_table *wait)
{
    unsigned int mask = 0;
	struct flp_data* p_flp_data = filp->private_data;	
	struct dev_data* p_dev_data = p_flp_data->p_dev_data;
	struct dev_specific_data_t* p_data = &p_dev_data->dev_specific_data;
 	unsigned long 		cpu_flags;

    if(down_interruptible(&p_data->oper_sem))
        return -ERESTARTSYS;
    poll_wait(filp, &p_data->wait_queue, wait);
	spin_lock_irqsave(&p_data->lock, cpu_flags);
    if(kfifo_len(&p_data->fifo))
        mask |= POLLIN | POLLRDNORM;
 	spin_unlock_irqrestore(&p_data->lock, cpu_flags);
    up(&p_data->oper_sem);

	return mask;

}

static int file_release(struct inode *inode, struct file *filp)
{
	struct flp_data* p_flp_data = filp->private_data;	

    /* remove this filp from the asynchronously notified filp's */

    kfree(p_flp_data);

    filp->private_data = NULL;
    
    return 0;
}


module_init(module_init_func);
module_exit(module_exit_func);

MODULE_AUTHOR("GM Technology Corp.");
MODULE_DESCRIPTION("GM fasync test");
MODULE_LICENSE("GPL");


