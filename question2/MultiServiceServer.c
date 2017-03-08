#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/signal.h>
	

	struct 
	{
		pthread_mutex_t	st_mutex;
		unsigned int	totalDVDs;
		unsigned int	tcptotal;
		unsigned int	udptotal;
		long int	orderno;
		long int	orderquantity;
	} stats;

	#define	INTERVAL	1	/* secs */

	void	counter(void);
	int	tcp_process(long int ssock);
	int	udp_process(long int usock);
	int	udp_process1(long int usock);

	struct dvd
	{	
		long int dvd_item_no;
		char title[50];
		long int quantity;
	};

     	struct dvd dvd_items[3]={1001, "Harry Potter", 100, 1002, "Star wars", 80, 1003, "Inside Out", 50};
	
	struct sockaddr_in cr;

int main(int argc, char *argv[])
{

	char newbuffer[1000];
     
	pthread_t	thread;
	pthread_attr_t	thread_a;

     	int ret, on, on1, ret1;
     	int tsock, udpsock, nfds;
	
 	socklen_t clilen;
 	struct sockaddr_in serv_addr, cli_addr;
    
     	nfds= getdtablesize();
     	fd_set rfds;
     	FD_ZERO(&rfds);

     	tsock = socket(AF_INET, SOCK_STREAM, 0);
     	udpsock = socket(AF_INET, SOCK_DGRAM, 0);
     	on = 1;
     	ret = setsockopt( tsock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
     	on1 = 1;
     	ret1 = setsockopt( udpsock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on1) );

     	serv_addr.sin_family = AF_INET;
     	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
     	serv_addr.sin_port = htons(8000);
     
     	bind(tsock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    	if(listen(tsock,1)==0)
		printf("Waiting for Client\n");
     	else
		printf("Error\n");
     
     	bind(udpsock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
      
	(void) pthread_attr_init(&thread_a);
	(void) pthread_attr_setdetachstate(&thread_a, PTHREAD_CREATE_DETACHED);
	(void) pthread_mutex_init(&stats.st_mutex, 0);

	
	while(1)
	{
	
		if (pthread_create(&thread, &thread_a, (void * (*)(void *))counter, 0) < 0)
			printf("pthread_create(counter): error occurred");

     		FD_SET(tsock, &rfds); 
     		FD_SET(udpsock, &rfds); 

     		if (select(nfds, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0)
			printf("select error occurred");


    		if (FD_ISSET(tsock, &rfds)) 
     		{
          		
			int newsockfd;
     			clilen = sizeof(cli_addr);
     			newsockfd = accept(tsock, (struct sockaddr *) &cli_addr, &clilen);
			
			printf("\n\n\nServer Details: %s PORT %d \n",inet_ntop(AF_INET,&serv_addr.sin_addr,newbuffer,sizeof(newbuffer)),
					htons(serv_addr.sin_port));
    			printf("Request from %s PORT %d \n\n",inet_ntop(AF_INET,&cli_addr.sin_addr,newbuffer,sizeof(newbuffer)),
					htons(cli_addr.sin_port));


      			if (newsockfd < 0) 
			{
				if (errno == EINTR)
					continue;
				printf("Accept failed");
			}

			if (pthread_create(&thread, &thread_a, (void * (*)(void *))tcp_process, (void *)(long)newsockfd) < 0)
				printf("pthread_create(counter): error occurred");

    
    		}


  		if (FD_ISSET(udpsock, &rfds)) 
    		{
			char buffer[1000];
     			char buffer1[1000];
     			char buffer2[1000]; 
			int n;
			clilen = sizeof(cli_addr);

    
     			strcpy(buffer,"");
     			n = recvfrom(udpsock, buffer, 1000, 0, (struct sockaddr *) &cli_addr, &clilen);
			buffer[n]=0;
						cr=cli_addr;
			printf("\n\n\nServer Details: %s PORT %d \n",inet_ntop(AF_INET,&serv_addr.sin_addr,newbuffer,sizeof(newbuffer)),
					htons(serv_addr.sin_port));
     			printf("Request from %s PORT %d \n\n",inet_ntop(AF_INET,&cli_addr.sin_addr,newbuffer,sizeof(newbuffer)),
					htons(cli_addr.sin_port));
			printf("Client Request: %s\n",buffer);
   
			if(buffer[0]=='o' && buffer[1]=='r' && buffer[2]=='d' && buffer[3]=='e' && buffer[4]=='r')
			{
				(void) pthread_mutex_lock(&stats.st_mutex);

     				strcpy(buffer1,"");
				n = recvfrom(udpsock, buffer1, 1000, 0, (struct sockaddr *) &cli_addr, &clilen);
				buffer1[n]=0;
				stats.orderno=atoi(buffer1);
		
     				strcpy(buffer2,"");
     				n = recvfrom(udpsock, buffer2, 1000, 0, (struct sockaddr *) &cli_addr, &clilen);
				buffer2[n]=0;
				stats.orderquantity=atoi(buffer2);

				printf("Received order of item no %li and %li copies.\n",stats.orderno, stats.orderquantity);

				(void) pthread_mutex_unlock(&stats.st_mutex);

				if (pthread_create(&thread, &thread_a, (void * (*)(void *))udp_process1, (void *)(long)udpsock) < 0)
					printf("pthread_create(counter): error occurred");

     			}
			if(buffer[0]=='l' && buffer[1]=='i' && buffer[2]=='s' && buffer[3]=='t')
			{
				if (pthread_create(&thread, &thread_a, (void * (*)(void *))udp_process, (void *)(long)udpsock) < 0)
					printf("pthread_create(counter): error occurred");
			}

    		}
	}
	return 0; 
}


int tcp_process(long int ssock)
{

	char buffer[1000];
     	char newbuffer[1000];
     	
	bzero(buffer,1000);
	int cc, i;

    	memset(buffer, 0, sizeof(buffer));     
     	cc=read(ssock,buffer,sizeof(buffer));
     	printf("Client Request: %s\n\n",buffer);
   
     	if(buffer[0]=='l' && buffer[1]=='i' && buffer[2]=='s' && buffer[3]=='t')
     	{

		strcpy(newbuffer,"--------------------------------");
		write(ssock, newbuffer, sizeof(newbuffer));

		strcpy(newbuffer,"Item\tTitle\t\tQuantity");
		write(ssock, newbuffer, sizeof(newbuffer));

		strcpy(newbuffer,"--------------------------------");
		write(ssock, newbuffer, sizeof(newbuffer));

		for(i=0;i<3;i++)
		{
			snprintf(newbuffer,sizeof(newbuffer),"%li\t%s\t%li" ,dvd_items[i].dvd_item_no,dvd_items[i].title,
					dvd_items[i].quantity);

			write(ssock, newbuffer, sizeof(newbuffer));
		}

		strcpy(newbuffer,"--------------------------------");
		write(ssock, newbuffer, sizeof(newbuffer));

		strcpy(newbuffer,"Place the order\n");
		write(ssock, newbuffer, sizeof(newbuffer));

     	}

	if(buffer[0]=='o' && buffer[1]=='r' && buffer[2]=='d' && buffer[3]=='e' && buffer[4]=='r')
    	{
	


		(void) pthread_mutex_lock(&stats.st_mutex);

     		read(ssock,buffer,sizeof(buffer));
     		stats.orderno=atoi(buffer);

     		read(ssock,buffer,sizeof(buffer));
     		stats.orderquantity=atoi(buffer);

		printf("Received order of item no %li and %li copies.\n",stats.orderno, stats.orderquantity);

     		for(i=0;i<3;i++)
     		{
			
			if((dvd_items[i].dvd_item_no == stats.orderno)&&(dvd_items[i].quantity >= stats.orderquantity))
			{
				dvd_items[i].quantity=dvd_items[i].quantity-stats.orderquantity;

				stats.totalDVDs += stats.orderquantity;

				(void) pthread_mutex_unlock(&stats.st_mutex);

				printf("Order Request Fullfilled.  %li copies of item no- %li %s in stock\n\n" ,dvd_items[i].quantity,
					dvd_items[i].dvd_item_no,dvd_items[i].title);

				snprintf(newbuffer,sizeof(newbuffer),"Order Request Fullfilled.  %li copies of item no- %li %s in stock\n\n" ,
					dvd_items[i].quantity,dvd_items[i].dvd_item_no,dvd_items[i].title);
	
				write(ssock, newbuffer, sizeof(newbuffer));
			}
	
     		}
     
     	}

	(void) close(ssock);

	(void) pthread_mutex_lock(&stats.st_mutex);
	stats.tcptotal++;
	(void) pthread_mutex_unlock(&stats.st_mutex);

	return 0;
}




int	udp_process(long int usock)
{
	
     	char newbuffer[1000];
	int i;
	

	strcpy(newbuffer,"--------------------------------");
	sendto(usock, newbuffer, 1000, 0, (struct sockaddr *) &cr, sizeof(cr));       
	
	strcpy(newbuffer,"Item\tTitle\t\tQuantity");
	sendto(usock, newbuffer, 1000, 0, (struct sockaddr *) &cr, sizeof(cr));     
	
	strcpy(newbuffer,"--------------------------------");
	sendto(usock, newbuffer, 1000, 0, (struct sockaddr *) &cr, sizeof(cr));     
	
	for(i=0;i<3;i++)
	{

		snprintf(newbuffer,sizeof(newbuffer),"%li\t%s\t%li" ,dvd_items[i].dvd_item_no,
			dvd_items[i].title,dvd_items[i].quantity);
			
		sendto(usock, newbuffer, 1000, 0, (struct sockaddr *) &cr, sizeof(cr));     	
	}

	strcpy(newbuffer,"--------------------------------");
	sendto(usock, newbuffer, 1000, 0, (struct sockaddr *) &cr, sizeof(cr));   
	
	strcpy(newbuffer,"Place your order\n");
	sendto(usock, newbuffer, 1000, 0, (struct sockaddr *) &cr, sizeof(cr));    
	      
	(void) pthread_mutex_lock(&stats.st_mutex);
	stats.udptotal++;
	(void) pthread_mutex_unlock(&stats.st_mutex);

return 0;
}

int	udp_process1(long int usock)
{
	
     	char newbuffer[1000];
	int i;

	(void) pthread_mutex_lock(&stats.st_mutex);

  	for(i=0;i<3;i++)
	{


		if((dvd_items[i].dvd_item_no == stats.orderno)&&(dvd_items[i].quantity >= stats.orderquantity))
		{
			dvd_items[i].quantity=dvd_items[i].quantity-stats.orderquantity;

			printf("Order Request Fullfilled.  %li copies of item no- %li %s in stock\n\n" ,
			dvd_items[i].quantity,dvd_items[i].dvd_item_no,dvd_items[i].title);

			stats.totalDVDs += stats.orderquantity;
			(void) pthread_mutex_unlock(&stats.st_mutex);

			snprintf(newbuffer,sizeof(newbuffer),"Order Request Fullfilled.  %li copies of item no- %li %s in stock\n\n" ,
			dvd_items[i].quantity,dvd_items[i].dvd_item_no,dvd_items[i].title);

			sendto(usock, newbuffer, 1000, 0, (struct sockaddr *) &cr, sizeof(cr));    
		}	
        	
	}

	(void) pthread_mutex_lock(&stats.st_mutex);
	stats.udptotal++;
	(void) pthread_mutex_unlock(&stats.st_mutex);

return 0;
}


void counter(void)
{
	
	(void) sleep(INTERVAL);

	(void) pthread_mutex_lock(&stats.st_mutex); 
	
	(void) printf("%-32s: %u\n", "Total Number of  DVDs purchased", stats.totalDVDs);
	(void) printf("%-32s: %u\n", "Total Number of TCP Client Connections", stats.tcptotal);
	(void) printf("%-32s: %u\n", "Total Number of UDP Client Connections", stats.udptotal);
		
	(void) pthread_mutex_unlock(&stats.st_mutex);

}
