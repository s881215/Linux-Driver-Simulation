#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/string.h>
#include <linux/pwm.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/delay.h>

#define GPIO_BUTTON 17
#define PWM_CHIP_ID 0

static int irq_number;

static int target_angle=0;
struct pwm_device* pwm_servo=NULL;
static struct kobject* my_kobj;

// define the pointer of input device
static struct input_dev *my_input_dev;

void move_servo(int angle){
	if(pwm_servo){
		long duty_ns=500000+(angle*1000000/180);
		pwm_config(pwm_servo, duty_ns, 20000000);
		pwm_enable(pwm_servo);
	}
}

// ISR, Top Half
static irqreturn_t gpio_irq_handler(int irq,void* dev_id){
	pr_info("Interrupt: Button pressed! Waking up thread...\n");
	return IRQ_WAKE_THREAD;
}

// ISR, Buttom Half
static irqreturn_t gpio_thread_fn(int irq,void* dev_id){
	pr_info("Thread: Doing heavy work...\n");
	pr_info("Input: Reporting virtual touch coordinate (512,512)\n");
	
	// simulate the touch event
	static int toggle=0;
	input_report_abs(my_input_dev,ABS_X,512+toggle);
	input_report_abs(my_input_dev,ABS_Y,512+toggle);
	input_report_key(my_input_dev,BTN_TOUCH,1); // press
	toggle = !toggle;
	input_sync(my_input_dev); // data sync
	
	move_servo(90);
	msleep(500);
	move_servo(0);

	// simulate the release event
	input_report_key(my_input_dev,BTN_TOUCH,0);
	input_sync(my_input_dev);
	return IRQ_HANDLED;
}

// 1. define Show function (call this function when user execute cat command)
static ssize_t angle_show(struct kobject* kobj,struct kobj_attribute* attr,char* buf){
	return sprintf(buf,"%d\n",target_angle);
}

// 2. define Store function (call this function when user execute echo command)
static ssize_t angle_store(struct kobject* kobj,struct kobj_attribute *attr,const char *buf,size_t count){
	int ret;

	ret=kstrtoint(buf,10,&target_angle);
	if(ret<0) return ret;

	pr_info("Sysfs: Setting servo angle to %d\n",target_angle);

	move_servo(target_angle);

	return count;
}

// 3. construct struct
static struct kobj_attribute angle_attribute=__ATTR(servo_angle,0660,angle_show,angle_store);

static int __init my_driver_init(void) {
    int ret, error;

    // 1. aply GPIO
    ret = gpio_request(GPIO_BUTTON, "button_irq");
    if (ret) return ret;
    gpio_direction_input(GPIO_BUTTON);

    // 2. get IRQ
    irq_number = gpio_to_irq(GPIO_BUTTON);
    if (irq_number < 0) {
        ret = irq_number;
        goto err_gpio;
    }

    // 3. aply interrupt
    ret = request_threaded_irq(irq_number, gpio_irq_handler, gpio_thread_fn,
                               IRQF_TRIGGER_RISING, "my_motor_controller", NULL);
    if (ret) goto err_gpio;

    // 4. aply PWM 
    pwm_servo = pwm_request(1, "servo_pwm");
    if (IS_ERR(pwm_servo)) {
        ret = PTR_ERR(pwm_servo);
        goto err_irq;
    }

    // 5. create Sysfs 
    my_kobj = kobject_create_and_add("my_servo", kernel_kobj);
    if (!my_kobj) {
        ret = -ENOMEM;
        goto err_pwm;
    }
    error = sysfs_create_file(my_kobj, &angle_attribute.attr);
    if (error) {
        ret = error;
        goto err_kobj;
    }

    // 6.1 apply the structure of input device
    my_input_dev=input_allocate_device();
    if(!my_input_dev) return -ENOMEM;

    // 6.2 set the supported event: ABS & KEY
    my_input_dev->name="Virtual_Touch_Servo";
    set_bit(EV_ABS,my_input_dev->evbit);
    set_bit(EV_KEY,my_input_dev->evbit);
    set_bit(BTN_TOUCH,my_input_dev->keybit);

    // 6.3 set the range of X-axis & Y-axis
    input_set_abs_params(my_input_dev,ABS_X,0,1024,0,0);
    input_set_abs_params(my_input_dev,ABS_Y,0,1024,0,0);

    // 6.4 register the input device
    ret=input_register_device(my_input_dev);
    if(ret){
	    input_free_device(my_input_dev);
	    goto err_pwm;
    }

    pr_info("Driver Init: Success!\n");
    return 0;

err_kobj:
    kobject_put(my_kobj);
err_pwm:
    pwm_free(pwm_servo);
err_irq:
    free_irq(irq_number, NULL);
err_gpio:
    gpio_free(GPIO_BUTTON);
    return ret;
}

static void __exit my_driver_exit(void){
	input_unregister_device(my_input_dev);
	sysfs_remove_file(my_kobj,&angle_attribute.attr);
	kobject_put(my_kobj);
	free_irq(irq_number,NULL);
	pwm_disable(pwm_servo);
	pwm_free(pwm_servo);
	gpio_free(GPIO_BUTTON);
}

module_init(my_driver_init);
module_exit(my_driver_exit);
MODULE_LICENSE("GPL");
