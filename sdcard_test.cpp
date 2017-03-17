#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mount.h>
#include <mntent.h>
#include <error.h>
#include <stdlib.h>

#define DEBUG(msg)		printf("%s\n",msg);
#define PROC_MOUNTS_FILENAME "/proc/mounts"
#define SDCARD_PATH "/tmp/sdcard_test"
#define MAX_PATH 1024
struct MountedVolume
{
	const char *device;
	const char *mount_point;
	const char *filesystem;
	const char *flags;
};

typedef struct 
{
	MountedVolume *volumes;
	int volumes_allocd;
	int volume_count;
} MountsState;

static MountsState g_mounts_state = { NULL, 0, 0 };

static inline void free_volume_internals(const MountedVolume *volume, int zero)
{
	free((char *)volume->device);
	free((char *)volume->mount_point);
	free((char *)volume->filesystem);
	free((char *)volume->flags);
	if (zero)
	{
		memset((void *)volume, 0, sizeof(*volume));
	}
}

int scan_mounted_volumes()
{
	FILE *fp;
	const int numv = 32;
	int i = 0;
	struct mntent *mentry;
	MountedVolume *v, *volumes;
	
	if (g_mounts_state.volumes == NULL)
	{
		volumes = (MountedVolume *)malloc(numv * sizeof(*volumes));
		
		if (volumes == NULL)
		{
			return -1;
		}
		
		g_mounts_state.volumes = volumes;
		g_mounts_state.volumes_allocd = numv;
		memset(volumes, 0, numv * sizeof(*volumes));
	}
	else
	{
		for (i = 0; i < g_mounts_state.volume_count; i++)
		{
			free_volume_internals(&g_mounts_state.volumes[i],1);
		}
	}	
	g_mounts_state.volume_count = 0;
	
	fp = setmntent(PROC_MOUNTS_FILENAME, "r");
	if (fp == NULL)
	{
		perror("creat error");
		return -1;
	}
	
	while ((mentry = getmntent(fp)) != NULL)
	{
		v = &g_mounts_state.volumes[g_mounts_state.volume_count++];
		v->device = strdup(mentry->mnt_fsname);
		v->mount_point = strdup(mentry->mnt_dir);
		v->filesystem = strdup(mentry->mnt_type);
		v->flags = strdup(mentry->mnt_opts);
	}

	endmntent(fp);
	return 0;

}
	
const MountedVolume *find_mounted_volume_by_device(const char *device)
{
	int i = 0;
	MountedVolume *v;

	if (g_mounts_state.volumes != NULL)
	{
		for (i = 0; i < g_mounts_state.volume_count; i++)
		{
			v = &g_mounts_state.volumes[i];
			
			if (v->device != NULL)
			{
				if (strcmp(v->device, device) == 0)
				{
					return v;
				}
			}
		}
	}
	return NULL;
}
/*
const MountedVolume *find_mounted_volume_by_mount_point(const char *mount_point)
{
	int i = 0;
	MountedVolume *v;

	if(g_mounts_state.volumes !=NULL)
	{
		for (i = 0; i < g_mounts_state.volume_count; i++)
		{
			v = &g_mounts_state.volumes[i];
			
			if (v->mount_point != NULL)
			{
				if (strcmp(v->mount_point, mount_point) == 0)
				{
					return v;
				}
			}
		}
	}
	return NULL;

}
*/
int umount_mounted_volume(const MountedVolume *volume)
{
	int ret = -1;
	ret = umount(volume->mount_point);
	if (ret == 0)
	{
		free_volume_internals(volume,1);
		return 0;
	}
	return ret; 
}

int sdcard_test(const char *device)
{
	int result = 0;
	char path[MAX_PATH];	

	result = scan_mounted_volumes();
	if (result < 0)
	{
		DEBUG("failed to scan mounted volumes\n");
		return -1;
	}
	
	
	const MountedVolume *mv = find_mounted_volume_by_device(device);	
	
	if (mv) 
	{
		snprintf(path, MAX_PATH, "%s/sdcard_test.txt", mv->mount_point);
		result = open(path, O_CREAT);
		if (result < 0)
		{
			unlink(path);
			DEBUG("creat file failed");
			return -1;
		}
		else
		{
			close(result);
			unlink(path);
			DEBUG("success by auto_mount\n");
			return 0;
		}
	}	
	result = access(SDCARD_PATH,0);
	if (result == 0)
	{	
		result = rmdir(SDCARD_PATH);	
		if (result < 0)
		{
			printf("%s is exist and it's not empty\n",SDCARD_PATH);
			return -1;
		}
	}
	
	result = mkdir(SDCARD_PATH, 0777);
	
	if (result < 0)
	{
		printf("failed to creat %s\n", SDCARD_PATH);
		return -1;
	}

	result = mount(device, SDCARD_PATH, "vfat", 
		MS_NOATIME | MS_NODEV | MS_NODIRATIME, "shortname=mixed,utf8");
	
	if (result < 0)
	{
		DEBUG("mount failed!\n");
		rmdir(SDCARD_PATH);
		return -1;
	}
	
	result = scan_mounted_volumes();
	if (result < 0)
	{
		DEBUG("failed to scan mounted volumes\n");
		return -1;
	}
	const MountedVolume *ownmv = find_mounted_volume_by_device(device);	
	
	if (ownmv == NULL) 
	{
		printf("%s no find %s\n",PROC_MOUNTS_FILENAME,device);
		return -1;
	}

	snprintf(path, MAX_PATH, "%s/sdcard_test.txt",SDCARD_PATH);
	result = open(path,O_CREAT);
	if (result < 0)
	{
		unlink(path);
		usleep(200000);
		int i = 0;
		for (i = 0; i < 5; i++)
		{
			result = umount_mounted_volume(ownmv);
			
			if (result == 0)
			{
				rmdir(SDCARD_PATH);
				DEBUG("creat file failed");
				return -1;
			}		
		}
		printf("failed umount %s\n",device);
		return -1;
	}
	
	close(result);
	unlink(path);
	usleep(200000);
	int i = 0;
	for (i = 0; i < 5; i++)
	{
		result = umount_mounted_volume(ownmv);
		if (result == 0)
		{
			rmdir(SDCARD_PATH);
			DEBUG("success by handle_mount\n");
			return 0;
		}		
	}	
	printf("failed umount %s\n",device);
	return 0;

}

#if 1
int main()
{
	int ret = 0;
	ret = sdcard_test("/dev/sdb1");
	if (ret == 0)
		printf("success!\n");
	else 
		printf("failed!\n");
	return 0;
}
#endif
