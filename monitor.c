#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>

int main(int argv,char** argc){
	int fd;
	struct input_event ev;

	fd=open("/dev/input/event2",O_RDONLY);
	if(fd<0){
		perror("Cannot access input device");
		return 1;
	}

	printf("Monitoring the touch event...\n");

	while(1){
		read(fd,&ev,sizeof(struct input_event));

		if(ev.type==EV_ABS){
			if(ev.code==ABS_X) printf("X: %d\n",ev.value);
			if(ev.code==ABS_Y) printf("Y: %d\n",ev.value);
		}else if(ev.type==EV_KEY && ev.code==BTN_TOUCH){
			printf("Status of Touch: %s\n",ev.value?"press":"release");
		}
	}
	return 0;
}
