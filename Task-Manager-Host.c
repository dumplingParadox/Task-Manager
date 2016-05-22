#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

//Run : gcc Task-Manager-Host.c -o Task-Manager-Host -pthread

#define MAXPENDING 5    /* Max connection requests */
          
void Die(char *mess) { perror(mess); return; }

void handlerTasks(void *socket_desc){
	int sock = *(int*)socket_desc;
	char buffer[512];
	int received = -1;
	while(1){
		/* Receive message */
		if ((received = recv(sock, buffer, 512, 0)) < 5) {
			Die("Failed to receive initial bytes from client");
			break;
		}
		buffer[received]='\0';
		printf("%s\n",buffer);
		if(strcmp(buffer,"BYE")==0)
			break;
		FILE *fp;	
		printf("Executing tasks.\n");
		if((fp=popen(buffer,"r"))==NULL){
						char msg[]="Error opening pipe!\n";
						continue;
					}
		/* Send bytes and check for more incoming data in loop */
		/*while (received > 0) {
			// Send back received data 
			if (send(sock, buffer, received, 0) != received) {
				Die("Failed to send bytes to client");
			}

			// Check for more data 
			if ((received = recv(sock, buffer, 32, 0)) < 0) {
				Die("Failed to receive additional bytes from client");
			}
		}*/
		printf("SendingTasks : %s\n",buffer);
		while(fgets(buffer,512,fp)!=NULL){
			if (send(sock, buffer, 512, 0) != 512) {
				Die("Failed to send bytes to client");
				continue;
			}

		}	
		strcpy(buffer,"DONE");
		printf("SendingTasks : Task%s\n",buffer);
		if (send(sock, buffer, 512, 0) != 512){
			Die("Failed to send bytes to client");
			continue;
		}
		sleep(1);
	}
	printf("Disconnected.\n");
	close(sock);
}

void handlerCPU(void *socket_desc){
	int sock = *(int*)socket_desc;
	char buffer[512];
	int received = -1;
	while(1){
		/* Receive message */
		if ((received = recv(sock, buffer, 512, 0)) < 5) {
			Die("Failed to receive initial bytes from client");
			break;
		}
		buffer[received]='\0';
		printf("%s\n",buffer);
		if(strcmp(buffer,"BYE")==0)
			break;
		FILE *fp;	
		printf("Executing CPU.\n");
		if((fp=popen(buffer,"r"))==NULL){
						char msg[]="Error opening pipe!\n";
						//send(sock,msg,strlen(msg),0);
						continue;
					}
		/* Send bytes and check for more incoming data in loop */
		/*while (received > 0) {
			// Send back received data 
			if (send(sock, buffer, received, 0) != received) {
				Die("Failed to send bytes to client");
			}

			// Check for more data 
			if ((received = recv(sock, buffer, 32, 0)) < 0) {
				Die("Failed to receive additional bytes from client");
			}
		}*/

		while(fgets(buffer,512,fp)!=NULL){
			printf("SendingCPU : %s\n",buffer);
			if (send(sock, buffer, 512, 0) != 512) {
				Die("Failed to send bytes to client");
				continue;
			}

		}	
		strcpy(buffer,"DONE");
		printf("SendingCPU : CPU%s\n",buffer);
		if (send(sock, buffer, 512, 0) != 512){
			Die("Failed to send bytes to client");
			continue;
		}
		sleep(1);
	}
	printf("Disconnected.\n");
	close(sock);
}

void handlerMem(void *socket_desc){
	int sock = *(int*)socket_desc;
	char buffer[512];
	int received = -1;
	while(1){
		/* Receive message */
		if ((received = recv(sock, buffer, 512, 0)) < 5) {
			Die("Failed to receive initial bytes from client");
			break;
		}
		buffer[received]='\0';
		printf("%s\n",buffer);
		if(strcmp(buffer,"BYE")==0)
			break;
		FILE *fp;	
		printf("Executing Mem.\n");
		if((fp=popen(buffer,"r"))==NULL){
						char msg[]="Error opening pipe!\n";
						//send(sock,msg,strlen(msg),0);
						continue;
					}
		/* Send bytes and check for more incoming data in loop */
		/*while (received > 0) {
			// Send back received data 
			if (send(sock, buffer, received, 0) != received) {
				Die("Failed to send bytes to client");
			}

			// Check for more data 
			if ((received = recv(sock, buffer, 32, 0)) < 0) {
				Die("Failed to receive additional bytes from client");
			}
		}*/

		while(fgets(buffer,512,fp)!=NULL){
			printf("SendingMem : %s\n",buffer);
			if (send(sock, buffer, 512, 0) != 512) {
				Die("Failed to send bytes to client");
				continue;
			}

		}	
		strcpy(buffer,"DONE");
		printf("SendingMem : Mem%s\n",buffer);
		if (send(sock, buffer, 512, 0) != 512){
			Die("Failed to send bytes to client");
			continue;
		}
		sleep(1);
	}
	printf("Disconnected.\n");
	close(sock);
}

void handlerRun(void *socket_desc){
	int sock = *(int*)socket_desc;
	char buffer[512];
	int received = -1,pid;
	while(1){
		/* Receive message */
		if ((received = recv(sock, buffer, 512, 0)) < 3) {
			Die("Failed to receive initial bytes from client");
			break;
		}
		//buffer[received]='\0';
		printf("%s\n",buffer);
		if(strcmp(buffer,"BYE")==0)
			break;
		FILE *fp;	
		printf("EXECUTING RUN.\n");
				pid=fork();
		if(pid==0){
			system(buffer);
		}
		sleep(1);
	}
	printf("Disconnected.\n");
	close(sock);
}


int main(int argc, char *argv[]) {
	int serversock, clientsock,*new_sock,sockCount=0;
	struct sockaddr_in echoserver, echoclient;
	int port=8796;
	/* Create the TCP socket */
	if ((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		Die("Failed to create socket");
	}

	/* Construct the server sockaddr_in structure */
	memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
	echoserver.sin_family = AF_INET;                  /* Internet/IP */
	echoserver.sin_addr.s_addr = htonl(INADDR_ANY);   /* Incoming addr */
	echoserver.sin_port = htons(port);       /*server port*/

	/* Bind the server socket */
	if (bind(serversock, (struct sockaddr *) &echoserver,sizeof(echoserver)) < 0) {
		Die("Failed to bind the server socket");
	}

	/* Listen on the server socket */
	if (listen(serversock, MAXPENDING) < 0) {
		Die("Failed to listen on server socket");
	}

	/* Run until cancelled */
	while(1){
		pthread_t sniffer_thread[4];
		sockCount=0;
		while (sockCount<4) {
			unsigned int clientlen = sizeof(echoclient);
			/* Wait for client connection */
			if ((clientsock = accept(serversock, (struct sockaddr *) &echoclient,&clientlen)) < 0) {
				Die("Failed to accept client connection");
				break;
			}
			new_sock = malloc(1);
			*new_sock = clientsock;
			if(sockCount==0){
				if(pthread_create( &sniffer_thread[sockCount++] , NULL ,  handlerTasks , (void*) new_sock) < 0){
					perror("could not create thread");
			    		return 1;
				}
			}else if(sockCount==1){
				if(pthread_create( &sniffer_thread[sockCount++] , NULL ,  handlerCPU , (void*) new_sock) < 0){
					perror("could not create thread");
			    		return 1;
				}
			}else if(sockCount==2){
				if(pthread_create( &sniffer_thread[sockCount++] , NULL ,  handlerMem , (void*) new_sock) < 0){
					perror("could not create thread");
			    		return 1;
				}
			}else if(sockCount==3){
				if(pthread_create( &sniffer_thread[sockCount++] , NULL ,  handlerRun , (void*) new_sock) < 0){
					perror("could not create thread");
			    		return 1;
				}
			}
			//pthread_join( sniffer_thread , NULL);
			fprintf(stdout, "Client connected: %s\n",inet_ntoa(echoclient.sin_addr));
			//HandleClient(clientsock);
		}
		pthread_join( sniffer_thread[0] , NULL);
		pthread_join( sniffer_thread[1] , NULL);
		pthread_join( sniffer_thread[2] , NULL);
		pthread_join( sniffer_thread[3] , NULL);
		sleep(1);
	}

}
