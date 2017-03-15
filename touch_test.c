#include <stdio.h>
#include <linux/input.h>
#include <fcntl.h>

int touch_test(char *devpath)
{
	struct input_event ev;
	int fd = -1;
	size_t rb = 0;
	
	if ((fd = open(devpath,O_RDONLY)) < 0)
	{
		perror("open error");
		return -1;
	}
	
	while(1)
	{
		rb = read(fd, &ev, sizeof(struct input_event));
		
		if (rb < sizeof(struct input_event))
		{
			perror("read error");
			close(fd);
			return -1;
		}
		
		if (EV_ABS == ev.type)
		{
			if (ABS_PRESSURE == ev.code)
			{
				if (1 == ev.value)
				{
					close(fd);
					return 0;
				}
				else
				{
					;
				}
			}
			else
			{
				;
			}
		}
		else
		{
			;
		}	
	}
}

#if 1
int main(int argc, char *argv[])
{
	int ret;
	ret = touch_test("/dev/input/event5");
	printf("ret = %d\n",ret);
	return 0;
}
#endif
