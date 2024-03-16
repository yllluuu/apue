#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<getopt.h>
#include<stdlib.h>

#define LISTEN_PORT          8889
#define BACKLOG              13
void print_usage(char *progname)
{
	printf("%s usage:\n",progname);
	printf("-p(--port):sepcify server listen port.\n");
	printf("-h(--help): print this help information.\n");
	return ;
}

int main(int argc,char **argv)
{
	int                  rv=-1;
	int                  listen_fd,client_fd=-1;
	struct sockaddr_in   serv_addr;
	struct sockaddr_in   cli_addr;
	socklen_t            cliaddr_len=0;
	char                 buf[1024];
	int                  port;
	int                  on=1;
	int                  ch;
	struct option        opts[]={
		{"port",required_argument,NULL,'p'},
		{"help",no_argument,NULL,'h'},
		{NULL,0,NULL,0}
	};

	while((ch=getopt_long(argc,argv,"p:h",opts,NULL))!=-1)
	{
		switch(ch)
		{
			case 'p':
				port=atoi(optarg);
				break;
			case 'h':
				print_usage(argv[0]);
				return 0;
		}
	}

	if(!port)
	{
		print_usage(argv[0]);
		return 0;
	}

	listen_fd=socket(AF_INET,SOCK_STREAM,0);
	if(listen_fd<0)
	{
		printf("create socket failure:%s\n",strerror(errno));
		return -1;
	}
	printf("socket create fd[%d]\n",listen_fd);

	setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(/*port*/LISTEN_PORT);
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	

	if(bind(listen_fd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)
	{
		printf("create socket failure:%s\n",strerror(errno));
		return -2;
	}

	listen(listen_fd,BACKLOG);

	printf("socket[%d] bind on port[%d] for all IP address ok\n",listen_fd,/*port*/LISTEN_PORT);

	//listen(listen_fd,BACKLOG);

	while(1)
	{
		printf("\nStart waitting and accept new client connect...\n",listen_fd);
		client_fd=accept(listen_fd,(struct sockaddr*)&cli_addr,&cliaddr_len);
		if(client_fd<0)
		{
			printf("accept new socket failure:%s\n",strerror(errno));
		//	return -2;
		        continue;
		}
		printf("accept new client[%s:%d] with fd [%d]\n",inet_ntoa(cli_addr.sin_addr),ntohs(cli_addr.sin_port),client_fd);
	        
	        memset(buf,0,sizeof(buf));
	        if((rv=read(client_fd,buf,sizeof(buf)))<0)
		{
		        printf("Read data from client socket[%d] failure:%s\n",client_fd,strerror(errno));
		        close(client_fd);
			continue;
		}
	        else if(rv==0)
		{
		        printf("client socket[%d] disconnected\n",client_fd);
		        close(client_fd);
		        continue;
		}
	        printf("read %d bytes data back to client[%d] and echo it back temperature: '%s'\n",rv,client_fd,buf);

	        if(write(client_fd,buf,rv)<0)
		{
		        printf("Write %d bytes data back to client[%d] failure:%s\n",rv,client_fd,strerror(errno));
		        close(client_fd);
		}

	        sleep(1);
	        close(client_fd);
	
	}
        close(listen_fd);
}	
