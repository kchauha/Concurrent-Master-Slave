#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include <sys/wait.h>
#include <signal.h>
#define MYPORT 1025 // the port users will be connecting to
#define BACKLOG 10 // how many pending connections queue will hold
void sigchld_handler(int s)
{
	while(wait(NULL) > 0);
}
int main(void)
{
	time_t timer;
	int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
	struct sockaddr_in server_addr; // my address information
	struct sockaddr_in client_addr; // connector.s address information
	int sin_size;
	struct sigaction sa;
	int yes=1;
        char outstr[200];
        time_t t;
        struct tm *tmp;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	 {
		perror("socket");
		exit(1);
	}
	if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1)
	{
		perror("setsockopt");
		exit(1);
	}
	server_addr.sin_family = AF_INET; // host byte order
	server_addr.sin_port = htons(MYPORT); // short, network byte order
	server_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
	memset(&(server_addr.sin_zero), 0, 8); // zero the rest of the struct

	if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
	{
		perror("bind");
		exit(1);
	}
	if (listen(sockfd, BACKLOG) == -1)
	{
		perror("listen");
		exit(1);
	}
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) 
	{
		perror("sigaction");
		exit(1);
	}
	while(1)  // main accept() loop
	{
		sin_size = sizeof(struct sockaddr_in);
		if ((new_fd = accept(sockfd, (struct sockaddr *)&client_addr,&sin_size)) == -1)
		{
			//perror("accept");
			continue;
		}
		printf("Received request from Client: %s:%d\n",
		inet_ntoa(client_addr.sin_addr),MYPORT);
		if (!fork()) 
		{ // this is the child process
			close(sockfd); // child doesn.t need the listener
			//timer = time(NULL);
                        t = time(NULL);
                        tmp = localtime(&t);
                         if (tmp == NULL) {
                           perror("localtime");
                           exit(EXIT_FAILURE);
                         }
                        const char *format= "%m/%d/%Y %l:%M:%S %a";
                        if (strftime(outstr, sizeof(outstr), format, tmp) == 0) {
                        fprintf(stderr, "strftime returned 0");
                        exit(EXIT_FAILURE);
                        }
			if (send(new_fd, outstr, 30, 0) == -1)
			perror("send");
			close(new_fd);
			exit(0);
		        }
	close(new_fd); // parent doesn.t need this
	}
	return 0;
}


