#include <uapi/linux/types.h>
#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/time32.h>
#include <linux/time64.h>
#include <linux/timekeeping.h>
#include <linux/timekeeping32.h>
#include <linux/uaccess.h>


asmlinkage int sys_my_time(unsigned long __user *integer, unsigned long __user *float_number,
								int pid, int start_or_finish){
	unsigned long k1 = *integer;
	unsigned long k2 = *float_number;
	struct timespec ttime;
	getnstimeofday(&ttime);
	*integer = ttime.tv_sec;
	*float_number = ttime.tv_nsec;
	
	if(start_or_finish == 1)
		printk("[Project1] %d %lu.%08lu %lu.%08lu\n",
        pid, k1, k2, *integer, *float_number);
	
	printk("fuck! %lu.%lu\n", *integer, *float_number);
	return 0;
}