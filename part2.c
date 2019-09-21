#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include<linux/slab.h>



unsigned long **sys_call_table;

//pointers to original system calls
asmlinkage long (*ref_sys_cs3013_syscall2)(void);

struct ancestry{
pid_t ancestors[10];
pid_t siblings[100];
pid_t children[100];
};

//new cs3013_syscall2 function
asmlinkage long new_sys_cs3013_syscall2(unsigned short *target_pid, struct ancestry *response){
	//task_struct that will coorilate to the user entered target_pid
	struct task_struct *myTask;
	//Task_struct pointer to be a place holder for iterated task structs
	struct task_struct *relatedProcesses;
	//Will be the current Pid of the child process, used to exit doubly linked list of task_structs
	pid_t stopper;
	//struct pid *myPidStruct;
	unsigned short *tPid;
	//The ancestry sent back to the user
	struct ancestry *kAncestry;
	//list pointer for the first list_of_each macro.
	struct list_head *list;
	//list pointer for the second list_of_each macro.
	struct list_head *list1;
	//counters to iterate the index of the ancestry struct
	int childCounter = 0;
	int ancestorCounter = 0;
	int siblingCounter = 0;
	//a bunch of variable that are used in separate for loops to populate the struct with negative one's to start
	int x;
	int y;
	int z;
	tPid = kmalloc(sizeof(tPid), GFP_KERNEL);
	relatedProcesses = kmalloc(sizeof(relatedProcesses), GFP_KERNEL);
	kAncestry = kmalloc(sizeof(struct ancestry), GFP_KERNEL);
	for(x = 0; x < 100; x++){
		kAncestry->children[x] = -1;
	}
	for(y = 0; y < 100; y++){
		kAncestry->siblings[y] = -1;
	}
	for(z = 0; z < 10; z++){
		kAncestry->ancestors[z] = -1;
	}

	//recieve the Pid and ancestry struct from the magic portal (user space)
	if(copy_from_user(tPid, target_pid, sizeof(*target_pid))){
		return EFAULT;
	}
	if(copy_from_user(kAncestry, response, sizeof(kAncestry))){
		return EFAULT;
	}

	//so my task is a valid PID, whereas the user specified pid may not be
	myTask = current;
	stopper = myTask -> pid;
	//iterates the first task_struct so that the loop does not end on the first iteration
	myTask = list_entry(myTask -> tasks.next, struct task_struct, tasks);
	while(myTask -> pid != *tPid){
			if (myTask -> pid == stopper){
				printk(KERN_INFO "Pid Not In Linked List\n");
				break;
			}
			myTask = list_entry(myTask -> tasks.next, struct task_struct, tasks);
	}
	//We should now have either the current task_struct or the correct task_struct

	//procede if we found the task_struct of the pid
	if(myTask -> pid == *tPid){
		printk(KERN_INFO "WE FOUND THE PID\n");
		//fill child portion of kAncestry
		list_for_each(list, &myTask -> children){
			relatedProcesses = list_entry(list, struct task_struct, sibling);
			printk(KERN_INFO "Process with PID %d is a child process to Process with PID %d\n", relatedProcesses -> pid, myTask -> pid);
			kAncestry -> children[childCounter] = relatedProcesses -> pid;
			childCounter+=1;
		}
		//childCounter = 0;
		//fill sibling portion of kAncestry
		list_for_each(list1, &myTask -> sibling){
			relatedProcesses = list_entry(list1, struct task_struct, sibling);
			printk(KERN_INFO"Process with PID %d is a sibling process to Process with PID %d\n", relatedProcesses -> pid, myTask -> pid);
			kAncestry -> siblings[siblingCounter] = relatedProcesses -> pid;
			siblingCounter+=1;
		}
		//childCounter = 0;
		//fill ancestry portion of kAncestry, but don't add the node itself
		myTask = myTask->real_parent;
		for(relatedProcesses = myTask; relatedProcesses != &init_task; relatedProcesses = relatedProcesses->real_parent){
			printk(KERN_INFO "Process with PID %d is an ancestor process to Process with PID %d\n", relatedProcesses -> pid, myTask -> pid);
			kAncestry -> ancestors[ancestorCounter] = relatedProcesses -> pid;
			ancestorCounter+=1;
		}
				copy_to_user(response, kAncestry, sizeof(struct ancestry));
		//found process case
	}
	else{
		return 0;
		}

	return 0;

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
	ref_sys_cs3013_syscall2 = (void*)sys_call_table[__NR_cs3013_syscall2];

  /* Replace the existing system calls */
  disable_page_protection();
  sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)new_sys_cs3013_syscall2;
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
	sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)ref_sys_cs3013_syscall2;
  enable_page_protection();

  printk(KERN_INFO "Unloaded interceptor!");
}

MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);
