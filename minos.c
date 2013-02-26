/*
 *
 *  Minos - Kernel Circular Buffer for logging inside /proc
 *  
 *  Dragos-Adrian Seredinschi, November, 2012
 *  
 */

/*
 * __KERNEL__: Code usable only in the kernel
 */
#undef __KERNEL__
#define __KERNEL__

/*
 * Not sure what this does / why it's needed
 */
#undef MODULE
#define MODULE

/* 
 * Linux Kernel/LKM headers: module.h is needed by all modules and kernel.h is needed for KERN_INFO.
 */
#include <linux/module.h>   // included for all kernel modules
#include <linux/kernel.h>   // included for KERN_INFO
#include <linux/init.h>     // included for __init and __exit macros
#include <linux/slab.h>     // for kmalloc / kfree
#include <linux/errno.h>
#include <linux/proc_fs.h> 

#include <linux/string.h>


/* 
 * Module basic definitions
 */
#define THIS_AUTHOR "Dragos-Adrian Seredinschi <adizere@cpan.org>"
#define THIS_NAME "minos"
#define THIS_DESCRIPTION "Kernel circular buffer for logging inside /proc"

#define _ERROR_EXIT -1
#define _SUCCESS_EXIT 0

#define _LOG_SIZE 1600000


/* 
 * Macros fun
 */ 
#define FORMAT_LOG_ENTRY(entry) "[" THIS_NAME "] " entry "\n"


struct log_entry
{
    char* data;
    size_t length;
    struct log_entry* next;
};


static char* log_data;
static struct proc_dir_entry* proc_entry;
// static struct log_entry* log_entry_head;


/*
 * TODO: Should "consume" the data that is returned
 */
int fetch_log_data(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    int len = 0;

    sprintf(log_data, "Summertime.\n");
    len = sprintf(buf,"%s", log_data);

    return len;
}


struct log_entry* log_entry_from_data(char* in_data)
{
    size_t data_length = 0;
    struct log_entry *new_entry = NULL;
    
    data_length = strlen(in_data) + 1;

    new_entry = (struct log_entry*)kmalloc(sizeof(struct log_entry), GFP_KERNEL);
    if (new_entry == NULL)
    {
        printk(KERN_INFO FORMAT_LOG_ENTRY("Error creating new log_entry from data: failed at memory allocation."));
        exit(-ENOMEM);
    }

    new_entry->length = data_length;
    new_entry->next = NULL;
    new_entry->data = (char*)kmalloc(data_length, GFP_KERNEL);

    return new_entry;
}


static int __init minos_init(void)
{
    printk(KERN_INFO FORMAT_LOG_ENTRY("Loading module."));

    log_data = kmalloc(_LOG_SIZE, GFP_KERNEL);
    if (log_data == NULL)
    {
        printk(KERN_INFO FORMAT_LOG_ENTRY("Failed allocation."));
        return -ENOMEM;
    }

    printk(KERN_INFO FORMAT_LOG_ENTRY("Memory allocation done."));

    proc_entry = create_proc_entry(THIS_NAME, 0666, NULL);
    if(proc_entry == NULL)
    {
        printk(KERN_INFO "Error creating proc entry");
        return _ERROR_EXIT;
    }

    proc_entry->data = (void*) &log_data;
    proc_entry->size = _LOG_SIZE;
    proc_entry->read_proc = fetch_log_data;

    return _SUCCESS_EXIT;   
}


static void __exit minos_cleanup(void)
{
    kfree(log_data);
    
    /* TODO: free the log_entry elements
     */

    remove_proc_entry(THIS_NAME, NULL);
    
    printk(KERN_INFO FORMAT_LOG_ENTRY("Cleaning up module."));
}



module_init(minos_init);
module_exit(minos_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(THIS_AUTHOR);
MODULE_DESCRIPTION(THIS_DESCRIPTION);
