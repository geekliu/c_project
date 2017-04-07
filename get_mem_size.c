#include <sys/sysinfo.h>
#include <stdio.h>

int get_mem_size(unsigned long *mem)
{
	struct sysinfo local_sysinfo;
	if (sysinfo(&local_sysinfo) == 0)
	{
		*mem = local_sysinfo.totalram;
		return 0;
	}
	return -1;
}

#if 1
int main(int argc, char *argv[])
{
	unsigned long mem;
	
	if (get_mem_size(&mem) == 0)
	{
		printf("%lu\n", mem);
	}
	else
	{
		printf("error!\n");
	}
	

}
#endif
