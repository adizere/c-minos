/*
 *
 *  Minos - Kernel Circular Buffer for logging inside /proc
 *  
 *  Adrian Seredinschi, November, 2012
 *  
 */

// __KERNEL__: Code usable only in the kernel
#undef __KERNEL__
#define __KERNEL__

// not sure what this does / why it's needed
#undef MODULE
#define MODULE

// Linux Kernel/LKM headers: module.h is needed by all modules and kernel.h is needed for KERN_INFO.
#include <linux/module.h>   // included for all kernel modules
#include <linux/kernel.h>   // included for KERN_INFO
#include <linux/init.h>     // included for __init and __exit macros
#include <linux/slab.h>     // for kmalloc(size_t size, int flags);


/* Module basic definitions
 */
#define MODULE_NAME "minos"
#define _ERROR_EXIT 1
#define _SUCCESS_EXIT 0
#define _LOG_SIZE 16


/* Macros
 */ 
#define FORMAT_LOG_ENTRY(e) MODULE_NAME e




/* Function pointers inside Data Structures 
    -- let's simulate some OOP
   
   Sample
 */
/* struct log {
    // ...
    void (*put_line)(char* line);
    char* (*read_line)(void);
    void (*write_line)(char* line);
    // ...
 };*/


static int __init minos_init(void)
{
    printk(KERN_INFO MODULE_NAME FORMAT_LOG_ENTRY(" Loading module"));

    char* logd;
    logd = kmalloc(_LOG_SIZE, GFP_KERNEL);
    if (logd==NULL) {
        printk(KERN_INFO MODULE_NAME " Failed allocation.\n");
        return _ERROR_EXIT;
    }

    return _SUCCESS_EXIT;   // Non-zero return means that the module couldn't be loaded.
}


static void __exit minos_cleanup(void)
{
    printk(KERN_INFO MODULE_NAME " Cleaning up module.\n");
}


module_init(minos_init);
module_exit(minos_cleanup);