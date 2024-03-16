#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<dirent.h>
#include<stdlib.h>
#include<getopt.h>
#include<fcntl.h>

#define SERVER_IP               "127.0.0.1"
#define SERVER_PORT             8889
#define MSG_STR                 "hello,Unix network program world"

int get_temperature(float  *temp);

void print_usage(char *progname)
{
	printf("%s usage:\n",progname);
	printf("-i(--ipaddr):sepcify server IP adderss.\n");
	printf("-p(--port):sepcify server port.\n");
	printf("-h(--help):print this help information.\n");
	return ;
}


int main(int argc,char **argv)
{
	int                     conn_fd=-1;
	int                     rv=-1;
	int                     rv1;
	int                     rv2=-1;
	float                   temp;
	char                    buf[1024];
	struct sockaddr_in      serv_addr;
	char                    *servip;
	int                     port;
	int                     ch;
	struct option           opts[]={
		{"ipaddr",required_argument,NULL,'i'},
		{"port",required_argument,NULL,'p'},
		{"help",no_argument,NULL,'h'},
		{NULL,0,NULL,0}
	};

	while((ch=getopt_long(argc,argv,"i:p:h",opts,NULL))!=-1)
	{
		switch(ch)
		{
			case 'i':
				servip=optarg;
				break;
			case 'p':
				port=atoi(optarg);
				break;
			case 'h':
				print_usage(argv[0]);
				return 0;
		}
	}

	if(!servip || !port)
	{
		print_usage(argv[0]);
		return 0;
	}

	/*if(argc<3)
	{
		printf("program usage:%s [ServerIP] [Port]\n",argv[0]);
		return -1;
	}

	servip=argv[1];
	port=atoi(argv[2]);*/
while(1)
{
	conn_fd=socket(AF_INET,SOCK_STREAM,0);
	if(conn_fd<0)
	{
		printf("creat socket failure:%s\n",strerror(errno));
		return -1;
	}

	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	//serv_addr.sin_port=htons(SERVER_PORT);
	serv_addr.sin_port=htons(port);
	//inet_aton(SERVER_IP,&serv_addr.sin_addr);
	inet_aton(servip,&serv_addr.sin_addr);


	if( connect(conn_fd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)
	{

		printf("connect to server [%s:%d] failure:%s\n",servip,port,strerror(errno));
		return 0;
	}
	printf("connect to server [%s%d] successfully\n",servip,port);

	memset(buf,0,sizeof(buf));
	get_temperature(&temp);
	snprintf(buf,sizeof(buf),"%.2f",temp);

	if(write(conn_fd,buf,strlen(buf))<0)
	{
		printf("write data to server[%s:%d] failure:%s\n",servip,port,strerror(errno));
		goto cleanup;
	}
	printf("Write successfully!\n");
//	printf("temperature:%s\n",buf);

	memset(buf,0,sizeof(buf));
	rv=read(conn_fd,buf,sizeof(buf));
	if(rv<0)
	{
		printf("read data from server failure:%s\n",strerror(errno));
		goto cleanup;
	}

	else if(0==rv)
	{
		printf("client connect to server get disconnected\n");
		goto cleanup;
	}
    printf("temperature:%s\n",buf);
	printf("\n");
	//printf("read %d bytes data from server:'%s'\n",rv,buf);
	close(conn_fd);

cleanup:
	close(conn_fd);

sleep(10);
}
}

int get_temperature(float *temp)
{
	int           fd=-1;
	char          buf[128];
	char          *ptr=NULL;
	DIR           *dirp=NULL;
	struct dirent *direntp=NULL;
	char          w1_path[64]="/sys/bus/w1/devices/";
	char          chip_sn[32];
	int           found=0;

	dirp=opendir(w1_path);
	if(!dirp)
	{
		printf("open folder %s faliure:%s\n",w1_path,strerror(errno));
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
		printf("open file faliure:%s\n",strerror(errno));
		return -3;
	}

	memset(buf,0,sizeof(buf));
	if(read(fd,buf,sizeof(buf))<0)
	{
		printf("read data from fd=%d faliure:%s\n",fd,strerror(errno));
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
	//printf("temprature:%f\n",*temp);

	close(fd);
	return 0;
}
	
