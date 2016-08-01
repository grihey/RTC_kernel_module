#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/jiffies.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/kthread.h>

#define MIL 1000000L
#define S_FACTOR 100
#define QUOT (MIL / S_FACTOR)
#define REMAINDER (MIL % S_FACTOR)

MODULE_LICENSE("Dual BSD/GPL");

static long epoch_time_sec = 0;
static int  epoch_time_usec = 0;
static int  speed = 100;
static int  random = 0;

module_param(epoch_time_sec, long, S_IRUGO | S_IWUGO);
module_param(epoch_time_usec, int, S_IRUGO | S_IWUGO);
module_param(speed, int, S_IRUGO | S_IWUGO);
module_param(random, int, S_IRUGO | S_IWUGO);

static int main_thread(void* data);

struct task_struct *task;

static int hello_init(void)
{
  struct timeval* tv;

  tv = kmalloc(sizeof(struct timeval), GFP_KERNEL);

  printk(KERN_ALERT "Hello, im here. \n");

  do_gettimeofday(tv);

  /*Init time parameters, which store seconds and useconds*/
  epoch_time_sec = tv->tv_sec;
  epoch_time_usec = tv->tv_usec;

  task = kthread_run(main_thread, &tv, "main loop");
  wake_up_process(task);

  return 0;
}

static int main_thread(void* data)
{
  while(!kthread_should_stop()){
    set_current_state(TASK_INTERRUPTIBLE);
    schedule_timeout(HZ);
    if(speed < 0){
      printk("Speed should be non-negative number. Thread stopped.\n");
      return -1;
    }

    if(random){
      
    } else {
      epoch_time_sec += (QUOT * speed +
          REMAINDER * speed + epoch_time_usec) / MIL;
      epoch_time_usec = (QUOT * speed + 
          REMAINDER * speed + epoch_time_usec) % MIL;
    }

    printk("After increment secs %ld, usecs %d", 
        epoch_time_sec, epoch_time_usec);
  }
    return 0;
}

void hello_exit(void)
{
  kthread_stop(task);
  printk(KERN_ALERT "Goodbye, cruel world.\n");
}

module_init(hello_init);
module_exit(hello_exit);
