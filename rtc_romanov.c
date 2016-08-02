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
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/random.h>

#define DEVICE_NAME "rtc38"
#define procfs_name "helloworld"

#define MIL 1000000L
#define S_FACTOR 100
#define S_QUOT (MIL / S_FACTOR)
#define S_REMAINDER (MIL % S_FACTOR)

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Grigoriy Romanov");

static long   time_sec = 0;
static int    time_usec = 0;
static int    speed = 100;
static int    random = 0;
static short  random_bound = 300;

module_param(time_sec, long, S_IRUGO | S_IWUGO);
module_param(time_usec, int, S_IRUGO | S_IWUGO);
module_param(speed, int, S_IRUGO | S_IWUGO);
module_param(random, int, S_IRUGO | S_IWUGO);
module_param(random_bound, short, S_IRUGO | S_IWUGO);

static int main_thread(void* data);
static int rtc_romanov_init(void);
void rtc_romanov_exit(void);

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "rtc38"
#define BUF_LEN 80

static int Major;
static int Device_Open = 0;
static char msg[BUF_LEN];
static char *msg_Ptr;

struct proc_dir_entry *proc_file;

static struct file_operations fops = {
  .read = device_read,
  .write = device_write,
  .open = device_open,
  .release = device_release
};

struct task_struct *task;

static int rtc_romanov_init(void)
{
  struct timeval* tv;
  tv = kmalloc(sizeof(struct timeval), GFP_KERNEL);
  do_gettimeofday(tv);

  /*Init time parameters, which store seconds and useconds*/
  time_sec = tv->tv_sec;
  time_usec = tv->tv_usec;

  task = kthread_run(main_thread, &tv, "main loop");
  wake_up_process(task);

  Major = register_chrdev(0, DEVICE_NAME, &fops);

  if(Major < 0) {
    printk(KERN_ALERT "Registering chardevice failed with %d\n", Major); 
    return Major;
  }
  printk(KERN_INFO "I was assigned major number %d. To talk to \n", Major);
  printk(KERN_INFO "the driver, create a dev file with\n");
  printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
  printk(KERN_INFO "the device file.\n");
  printk(KERN_INFO "Remove the device file and module when done.\n");

  return SUCCESS;
}

void rtc_romanov_exit(void)
{
  unregister_chrdev(Major, DEVICE_NAME);

  /*
  if(ret < 0)
    printk(KERN_ALERT "Error in unregistere_chrdev: %d\n", ret);
  */

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

      time_sec += (S_QUOT * rand_speed +
          S_REMAINDER * rand_speed + time_usec) / MIL;
      time_usec = (S_QUOT * rand_speed + 
          S_REMAINDER * rand_speed + time_usec) % MIL;
      
    } else {
      time_sec += (S_QUOT * speed +
          S_REMAINDER * speed + time_usec) / MIL;
      time_usec = (S_QUOT * speed + 
          S_REMAINDER * speed + time_usec) % MIL;
    }
  }
    return 0;
}

static int device_open(struct inode *inode, struct file *file)
{
  if(Device_Open)
    return -EBUSY;
  Device_Open++;
  sprintf(msg, "Seconds: %ld; Useconds: %d.\n",
      time_sec, time_usec);
  msg_Ptr = msg;
  try_module_get(THIS_MODULE);

  return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
  Device_Open--;
  module_put(THIS_MODULE);
  return 0;
}

static ssize_t device_read(struct file *filp,
                           char *buffer,
                           size_t length,
                           loff_t * offset)
{
  int bytes_read = 0;
  if(*msg_Ptr == 0)
    return 0;

  while(length && *msg_Ptr){
    put_user(*(msg_Ptr++), buffer++);
    length--;
    bytes_read++;
  }
  return bytes_read;
}

static ssize_t device_write(struct file *filp, 
                            const char *buffer, 
                            size_t length, 
                            loff_t * offset)
{
  printk(KERN_ALERT "Write not implementing on RTC Romanov module.\n");
  return -EINVAL;
}

module_init(rtc_romanov_init);
module_exit(rtc_romanov_exit);
