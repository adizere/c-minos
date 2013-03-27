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


static struct proc_dir_entry* proc_entry;
static struct log_entry* log_entry_head;
static struct log_entry* log_entry_tail;
static unsigned int data_length_sum = 0;

/*
TODO:
 - proper KERN_* printk() log levels
*/


/*
 ** Internal Method
 * _clear_log_entries
 * 
 * Traverses all the log entries starting from log_entry_head and clears them.
 * If copy_on_clear is set to true then all data found in entries will be saved
 * in the provided buf.
 */ 
int _clear_log_entries(bool copy_on_clear, char* buf)
{
    int len = 0;
    struct log_entry* current_entry;

    /* Check if the call to this function is valid
     */
    if (copy_on_clear == true && buf == NULL)
    {
        printk(KERN_INFO FORMAT_LOG_ENTRY("Error: cannot preserve the cleared "
            "data in an unitialized buffed!"));
        return 0;
    }

    /* Now consume
     */
    current_entry = log_entry_head;
    while(current_entry != NULL)
    {
        struct log_entry* next_entry;

        /* Only conserve if the copy_on_clear flag was set
         */
        if (copy_on_clear == true)
        {
            len += sprintf(buf+len, "%s\n", current_entry->data);
        }
        
        next_entry = current_entry->next;

        kfree(current_entry->data);
        kfree(current_entry);
        current_entry->data = NULL;
        current_entry = NULL;

        log_entry_head = current_entry = next_entry;
    }

    /* Reset the data length sum
     */
    data_length_sum = 0;

    return len;
}


/*
 ** Internal Method
 * _log_entry_from_data
 * 
 * Creates a new log_entry structure properly initialized from the provided
 * string in the parameter in_data. 
 */ 
struct log_entry* _log_entry_from_data(char* in_data)
{
    size_t data_length = 0;
    struct log_entry* new_entry = NULL;
    
    data_length = strlen(in_data) + 1;

    new_entry = (struct log_entry*)
        kmalloc(sizeof(struct log_entry), GFP_KERNEL);
    if (new_entry == NULL)
    {
        printk(KERN_INFO FORMAT_LOG_ENTRY("Error creating new log_entry from "
            "data: failed at memory allocation."));
        return NULL;
    }

    new_entry->length = data_length;
    new_entry->next = NULL;
    new_entry->data = (char*)kmalloc(data_length, GFP_KERNEL);
    strncpy(new_entry->data, in_data, data_length);
    if (new_entry->data == NULL)
    {
        printk(KERN_INFO FORMAT_LOG_ENTRY("Error creating new log_entry->data "
            "from data: failed at memory allocation."));
        return NULL;
    }

    new_entry->data[data_length-1] = '\0';

    return new_entry;
}


/*
 ** Internal Method
 * _insert_log_entry
 */ 
bool _insert_log_entry(struct log_entry* new_entry)
{
    /* Overwrite the oldest entry? (circular buffer)
     */
    if (data_length_sum + new_entry->length > _LOG_SIZE)
    {
        if (log_entry_head == NULL )
        {
            /* The entry is too big!
             */
            printk(KERN_INFO FORMAT_LOG_ENTRY("New entry is too big."));
            return false;
        }
        else if(log_entry_head->next == NULL)
        {
            kfree(log_entry_head->data);
            kfree(log_entry_head);

            log_entry_head = new_entry;
            return true;
        }
        else
        {
            
        }
    }
    /* Still have enough place, no need to overwrite
     */
    else {

    }
}


/*
 ** Proc Entry Method for Read
 * fetch_log_data
 */ 
int fetch_log_data(char *buf, char **start, off_t offset, int count, int *eof, 
    void *data)
{
    int len = 0;

    /*
     * Consume the entries and put the data in buf
     */
    len = _clear_log_entries(true, buf);

    return len;
}


/*
 ** Proc Entry Method for Write
 * clear_log_data
 */ 
int clear_log_data(struct file *file, const char __user *buffer,
                           unsigned long count, void *data)
{
    /*
     * Consume the data without conserving it 
     */
    _clear_log_entries(false, NULL);

    return 1;
}


static int __init minos_init(void)
{
    printk(KERN_INFO FORMAT_LOG_ENTRY("Loading module."));

    proc_entry = create_proc_entry(THIS_NAME, 0666, NULL);
    if(proc_entry == NULL)
    {
        printk(KERN_INFO "Error creating proc entry");
        return _ERROR_EXIT;
    }

    proc_entry->size = _LOG_SIZE;
    proc_entry->read_proc = fetch_log_data;
    proc_entry->write_proc = clear_log_data;

    /*
     * Test data
     */
    struct log_entry* new_entry = _log_entry_from_data("STAPH.");
    log_entry_head = new_entry;
    new_entry = _log_entry_from_data("STAPH 2222222222.");
    log_entry_head->next = new_entry;

    printk("%s\n", log_entry_head->data);
    printk("%s\n", log_entry_head->next->data);

    printk(KERN_INFO FORMAT_LOG_ENTRY("All good."));

    return _SUCCESS_EXIT;
}


static void __exit minos_cleanup(void)
{
    _clear_log_entries(false, NULL);

    remove_proc_entry(THIS_NAME, NULL);
    
    printk(KERN_INFO FORMAT_LOG_ENTRY("Cleaning up module."));
}



module_init(minos_init);
module_exit(minos_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(THIS_AUTHOR);
MODULE_DESCRIPTION(THIS_DESCRIPTION);

/*
 * END
 */