#include <linux/module.h>    // included for all kernel modules
#include <linux/proc_fs.h>   //for the proc file system
#include <linux/seq_file.h>  //for the sequence file	
#include <linux/kernel.h>    //for kernel info
#include <linux/mm.h>	     //for VM stats
#include <linux/vmstat.h> 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tejas Patil");
MODULE_DESCRIPTION("Write to Proc File System to find Number of Page Faults");


static struct proc_dir_entry* page_fault_file; //store entry in proce file


static int num_pgefault_show(struct seq_file *m, void *v)
{
 	//stores the number of pagefaults and prints
    static unsigned long memory[NR_VM_EVENT_ITEMS];
    all_vm_events(memory);
	
    seq_printf(m, "No of Pagefaults occured \n" );// prints fixed value
    seq_printf(m, "%lu\n", memory[PGFAULT]);
    return 0;
}


static int num_pgefault_open(struct inode *inode, struct file *file)
{
    return single_open(file, num_pgefault_show, NULL);
} 

static const struct file_operations pagefault_fops = {
    .owner    = THIS_MODULE,
    .open     = num_pgefault_open,
    .read     = seq_read,        
    .llseek   = seq_lseek,       
    .release  = single_release,  
};


static int __init num_pagefault_init(void)
{
 	// Create a new proc file num_pagefaults
 	page_fault_file = proc_create("num_pagefaults",0,NULL,&pagefault_fops);
	printk(KERN_INFO "num_pagefault file added to proc file system\n");
    	return 0;
}


static void __exit num_pagefault_exit(void)
{
	remove_proc_entry("num_pagefaults", NULL);
	printk(KERN_INFO "num_pagefault file removed from proc file system\n");
}


module_init(num_pagefault_init);
module_exit(num_pagefault_exit);

MODULE_LICENSE("GPL");


//References:
//https://www.linux.com/learn/linux-training/37985-the-kernel-newbie-corner-kernel-debugging-using-proc-qsequenceq-files-part-1

