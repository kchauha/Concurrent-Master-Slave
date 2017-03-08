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
    int usock, portno, n;
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
    usock = socket(AF_INET, SOCK_DGRAM, 0);

    if (usock < 0) 
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
    

    strcpy(buffer,argv[3]);
    sendto(usock, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    if (argc>4)
    {
	strcpy(buffer,argv[4]);
	sendto(usock, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

	strcpy(buffer,argv[5]);
	sendto(usock, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    }

    bzero(buffer,1000);
    

    while((n = recvfrom(usock, newbuffer, 1000, 0, NULL, NULL))>0)
    {
	newbuffer[n] = 0;
	printf("%s\n",newbuffer);
    } 
  	
close(usock);
return 0;
}
