#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/device.h>

#include <linux/jiffies.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/sched.h>

#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/random.h>

#define DEVICE_NAME "rtc38"
#define CLASS_NAME "rtc"

#define MIL 1000000L
#define S_FACTOR 100
#define S_QUOT (MIL / S_FACTOR)
#define S_REMAINDER (MIL % S_FACTOR)

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Grigoriy Romanov");

static int    majorNumber = 2150;
static struct class* rtcClass = NULL;
static struct device* rtcDevice = NULL;

static long   epoch_time_sec = 0;
static int    epoch_time_usec = 0;
static int    speed = 100;
static int    random = 0;
static short  random_bound = 300;

module_param(epoch_time_sec, long, S_IRUGO | S_IWUGO);
module_param(epoch_time_usec, int, S_IRUGO | S_IWUGO);
module_param(speed, int, S_IRUGO | S_IWUGO);
module_param(random, int, S_IRUGO | S_IWUGO);
module_param(random_bound, short, S_IRUGO | S_IWUGO);

static int main_thread(void* data);

struct task_struct *task;

static int rtc_romanov_init(void)
{
  struct timeval* tv;
  tv = kmalloc(sizeof(struct timeval), GFP_KERNEL);
  do_gettimeofday(tv);

  /*Init time parameters, which store seconds and useconds*/
  epoch_time_sec = tv->tv_sec;
  epoch_time_usec = tv->tv_usec;

  task = kthread_run(main_thread, &tv, "main loop");
  wake_up_process(task);

  /*Try to dinamically allocated major number
  majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
  if(majorNumber < 0){
    printk(KERN_ALERT "RTC_Romanov failed to register a major number\n");
    return majorNumber;
  }
  printk(KERN_ALERT "RTC_Romanov registered with major number %d\n",
      majorNumber);
 
  rtcClass = class_create(THIS_MODULE, CLASS_NAME);
  if(IS_ERR(rtcClass)){
    unregister_chrdev(majorNumber, DEVICE_NAME);
    printk(KERN_ALERT "Failed to register device class. \n");
    return PTR_ERR(rtcClass);
  }
  printk(KERN_ALERT "RTC_Romanov device class registered. \n");

  rtcDevice = device_create(rtcClass, NULL, MKDEV(majorNumber,0), NULL, DEVICE_NAME);
  if(IS_ERR(rtcDevice)){
    class_destroy(rtcClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    printk(KERN_ALERT "Failed to create the device. \n");
    return PTR_ERR(rtcDevice);
  }
  */

  printk(KERN_ALERT "RTC_Romanov module have been istalled. \n");

  return 0;
}

void rtc_romanov_exit(void)
{
  device_destroy(rtcClass, MKDEV(majorNumber, 0));
  class_unregister(rtcClass);
  class_destroy(rtcClass);
  unregister_chrdev(majorNumber, DEVICE_NAME);

  kthread_stop(task);
  printk(KERN_ALERT "RTC_Romanov module was removed.\n");
}

static int main_thread(void* data)
{
  unsigned short rand_speed;
  while(!kthread_should_stop()){
    set_current_state(TASK_INTERRUPTIBLE);
    schedule_timeout(HZ);
    if(speed < 0){
      printk("Speed should be non-negative number. Thread stopped.\n");
      return -1;
    }

    if(random){
      get_random_bytes(&rand_speed, sizeof(rand_speed));
      rand_speed %= random_bound;

      epoch_time_sec += (S_QUOT * rand_speed +
          S_REMAINDER * rand_speed + epoch_time_usec) / MIL;
      epoch_time_usec = (S_QUOT * rand_speed + 
          S_REMAINDER * rand_speed + epoch_time_usec) % MIL;
      
    } else {
      epoch_time_sec += (S_QUOT * speed +
          S_REMAINDER * speed + epoch_time_usec) / MIL;
      epoch_time_usec = (S_QUOT * speed + 
          S_REMAINDER * speed + epoch_time_usec) % MIL;
    }

    printk("After increment secs %ld, usecs %d\n", 
        epoch_time_sec, epoch_time_usec);
  }
    return 0;
}

module_init(rtc_romanov_init);
module_exit(rtc_romanov_exit);
