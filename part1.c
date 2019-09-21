#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>

unsigned long **sys_call_table;

//pointers to original system calls
asmlinkage long (*ref_sys_open)(const char __user *filename, int flags, umode_t mode);
asmlinkage long (*ref_sys_close)(unsigned int fd);
asmlinkage long (*ref_sys_read)(int fd, void *buf, size_t count);


//new sys_open function
asmlinkage long new_sys_open(const char __user *filename, int flags, umode_t mode){
	//gets the current user's user ID
	kuid_t userID = current_uid();
	//only print out info for regular users
	if(userID.val  >= 1000){
		printk(KERN_INFO "User %d is opening file: %s\n",userID.val,filename);
	}
	return ref_sys_open(filename, flags, mode);
}


//new sys_close function
asmlinkage long new_sys_close(unsigned int fd){
	//gets the current user's user ID
	kuid_t userID = current_uid();
	//only print out info for regular users
	if(userID.val >= 1000){
		printk(KERN_INFO "User %d is closing file descriptor: %d\n",userID.val,fd);
	}
	return ref_sys_close(fd);
}


//new sys_read function
asmlinkage long new_sys_read(int fd, void *buf, size_t count){
	kuid_t userID = current_uid();
	//create a 'needle' called virus to see if its value 'VIRUS' is in the haystack (the buffer that gets filled with info from the file)
	const char virus[] = "VIRUS";
	//creating a char pointer because strstr() needs a const char pointer
	const char *tmp;
	tmp = buf;
	// returns null if there isn't a match
	if(strstr(tmp, virus) != NULL){
		printk(KERN_INFO "User %d read from file description %d, but that read contained malicious code!\n",userID.val,fd);
	}
	return ref_sys_read(fd, buf, count);
}

static unsigned long **find_sys_call_table(void) {
  unsigned long int offset = PAGE_OFFSET;
  unsigned long **sct;

  while (offset < ULLONG_MAX) {
    sct = (unsigned long **)offset;

    if (sct[__NR_close] == (unsigned long *) sys_close) {
      printk(KERN_INFO "Interceptor: Found syscall table at address: 0x%02lX",
	     (unsigned long) sct);
      return sct;
    }

    offset += sizeof(void *);
  }

  return NULL;
}

static void disable_page_protection(void) {
  /*
    Control Register 0 (cr0) governs how the CPU operates.

    Bit #16, if set, prevents the CPU from writing to memory marked as
    read only. Well, our system call table meets that description.
    But, we can simply turn off this bit in cr0 to allow us to make
    changes. We read in the current value of the register (32 or 64
    bits wide), and AND that with a value where all bits are 0 except
    the 16th bit (using a negation operation), causing the write_cr0
    value to have the 16th bit cleared (with all other bits staying
    the same. We will thus be able to write to the protected memory.

    It's good to be the kernel!
  */
  write_cr0 (read_cr0 () & (~ 0x10000));
}

static void enable_page_protection(void) {
  /*
   See the above description for cr0. Here, we use an OR to set the
   16th bit to re-enable write protection on the CPU.
  */
  write_cr0 (read_cr0 () | 0x10000);
}

static int __init interceptor_start(void) {
  /* Find the system call table */
  if(!(sys_call_table = find_sys_call_table())) {
    /* Well, that didn't work.
       Cancel the module loading step. */
    return -1;
  }

	//copy of the original system calls
	ref_sys_open = (void *)sys_call_table[__NR_open];
	ref_sys_close = (void *)sys_call_table[__NR_close];
	ref_sys_read = (void *)sys_call_table[__NR_read];

  /* Replace the existing system calls */
  disable_page_protection();
	sys_call_table[__NR_open] = (unsigned long *)new_sys_open;
	sys_call_table[__NR_close] = (unsigned long *)new_sys_close;
	sys_call_table[__NR_read] = (unsigned long *)new_sys_read;
  enable_page_protection();

  /* And indicate the load was successful */
  printk(KERN_INFO "Loaded interceptor!");

  return 0;
}

static void __exit interceptor_end(void) {
  /* If we don't know what the syscall table is, don't bother. */
  if(!sys_call_table)
    return;

  /* Revert all system calls to what they were before we began. */
  disable_page_protection();
	sys_call_table[__NR_open] = (unsigned long *)ref_sys_open;
	sys_call_table[__NR_close] = (unsigned long *)ref_sys_close;
	sys_call_table[__NR_read] = (unsigned long *)ref_sys_read;
  enable_page_protection();

  printk(KERN_INFO "Unloaded interceptor!");
}

MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);
