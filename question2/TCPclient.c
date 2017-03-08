#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int tsock, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[1000];
    char newbuffer[1000];

    if (argc < 3) 
    {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    portno = atoi(argv[2]);
    tsock = socket(AF_INET, SOCK_STREAM, 0);

    if (tsock < 0) 
        error("SOCKET ERROR");

    server = gethostbyname(argv[1]);

    if (server == NULL) 
    {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(atoi(argv[2]));
    
    connect(tsock,(struct sockaddr *) &serv_addr,sizeof(serv_addr));

    strcpy(buffer,argv[3]);
    write(tsock, buffer, sizeof(buffer));

    if (argc>4)
    {
	strcpy(buffer,argv[4]);
	write(tsock, buffer, sizeof(buffer));
	strcpy(buffer,argv[5]);
	write(tsock, buffer, sizeof(buffer));
    }

    bzero(buffer,1000);
    while((n = read(tsock,newbuffer,sizeof(newbuffer)))>0)
    {
	newbuffer[n] = '\0';
	printf("%s\n",newbuffer);
	
    } 
  	
close(tsock);
return 0;
}
