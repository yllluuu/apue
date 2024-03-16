#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<dirent.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>

int get_temperature(float *temp);

int main(int argc,char *argv[])
{
	float           temp;
	int             rv;

	rv=get_temperature(&temp);
	if(rv<0)
	{
		printf("get temperature failure.reutrn value:%d",rv);
		return -1;
	}

	printf("temp:%f\n",temp);
}


int get_temperature(float *temp)
{
	int             fd=-1;
	char            buf[128];
	char           *ptr=NULL;
	DIR            *dirp=NULL;
	struct dirent  *direntp=NULL;
        char            w1_path[64]="/sys/bus/w1/devices/";
	char            chip_sn[32];
	int             found=0;

	dirp=opendir(w1_path);
	if(!dirp)
	{
		printf("open folder %s failur:%s\n",w1_path,strerror(errno));
		return -1;
	}
	while(NULL!=(direntp=readdir(dirp)))
	{
		if(strstr(direntp->d_name,"28-"))
		{
			strncpy(chip_sn,direntp->d_name,sizeof(chip_sn));
			found=1;
		}
	}
	closedir(dirp);

	if(!found)
	{
		printf("cannot find ds18b20 chipset\n");
		return -2;
	}

	strncat(w1_path,chip_sn,sizeof(w1_path)-strlen(w1_path));
	strncat(w1_path,"/w1_slave",sizeof(w1_path)-strlen(w1_path));

	if((fd=open(w1_path,O_RDONLY))<0)
	{
		printf("open file failure:%s\n",fd,strerror(errno));
		perror("open file failure");
		return -3;
	}

	memset(buf,0,sizeof(buf));

	if(read(fd,buf,sizeof(buf))<0)
	{
		printf("read data from fd=%d failure:%s\n",fd,strerror(errno));
		return -4;
	}

	ptr=strstr(buf,"t=");

	if(!ptr)
	{
		printf("cannot find t= string\n");
		return -5;
	}

	ptr +=2;

	*temp=atof(ptr)/1000;
	printf("temperature:%f\n", *temp);

	close(fd);

	return 0;
}

