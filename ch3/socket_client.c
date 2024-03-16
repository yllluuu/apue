#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<getopt.h>
#include<netdb.h>

#define SERVER_IP               "127.0.0.1"
#define SERVER_PORT             8889
#define MSG_STR                 "hello,Unix network program world"

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
	char                    buf[1024];
	struct sockaddr_in      serv_addr;
	char                    *servip;
	int                     port;
	char                    **hostip=NULL;
	char					ipstr[32];
	char					*servdn=NULL;
	struct hostent			*servhost=NULL;
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

	//域名解析
	if(!servip)
	{
		if((servhost = gethostbyname(servdn)) == NULL)
		{
			printf("Get host error: %s\n", strerror(errno));
			return -1;
		}
		switch(servhost->h_addrtype)
		{
			case AF_INET:
			case AF_INET6:
				hostip = servhost->h_addr_list;
					for(;*hostip != NULL; hostip++)
						printf("IP:%s\n", inet_ntop(servhost->h_addrtype, *hostip, ipstr, sizeof(ipstr)));
					servip=ipstr;
					break;
			default:
					printf("error adress\n");
					break;
		}
	}


	/*if(argc<3)
	{
		printf("program usage:%s [ServerIP] [Port]\n",argv[0]);
		return -1;
	}

	servip=argv[1];
	port=atoi(argv[2]);*/

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

	if(write(conn_fd,MSG_STR,strlen(MSG_STR))<0)
	{
		printf("write data to server[%s:%d] failure:%s\n",servip,port,strerror(errno));
		goto cleanup;
	}
	printf("Write successfully!\n");

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
	printf("read %d bytes data from server:'%s'\n",rv,buf);

cleanup:
	close(conn_fd);
}
