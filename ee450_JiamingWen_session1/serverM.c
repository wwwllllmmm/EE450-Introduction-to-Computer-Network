/**
**
** Author: Jiaming Wen
** USC-ID#: 4152412003
** use blocks of code from Beej’s socket programming tutorial(Beej’s guide to network programming)
**
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>

#define PORTCLIENTA "25003"     // the port for TCP with clientA
#define PORTCLIENTB "26003"     //the port for TCP with clientB

#define UDPPORT "24003" // the port use UDP

#define SERVERPORTA "21003"	   // the port users connecting to Backend-ServerA
#define SERVERPORTB "22003"	   // the port users will be connecting to Backend-ServerB
#define SERVERPORTC "23003"	   // the port users will be connecting to Backend-ServerC
#define IPADDRESS "127.0.0.1"  // local IP address

#define BACKLOG 10	 // how many pending connections queue will hold

#define MAXBUFLEN 4000

#define F_PATH "./alichain.txt"

// block of code from Beej’s socket programming tutorial
void sigchld_handler(int s){
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    
    while(waitpid(-1, NULL, WNOHANG) > 0);
    
    errno = saved_errno;
}

//compare
int max(int a, int b) 
{
	return a > b ? a : b;
}

// block of code from Beej’s socket programming tutorial
void *get_in_addr(struct sockaddr *sa) {
    if(sa->sa_family == AF_INET6) {
        // IPv6
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }
    // IPv4
    return &(((struct sockaddr_in*)sa)->sin_addr);
}

// split received message
void split_args2(char *args[], char *message) {
    char *p = strtok (message, " ");
    int i = 0;
    while (p != NULL)
    {
        args[i++] = p;
        p = strtok (NULL, " ");
    }
}

// split received message
void split_args(char *args[], char *message) {
    char *p = strtok (message, "\n");
    int i = 0;
    while (p != NULL)
    {
        args[i++] = p;
        p = strtok (NULL, "\n");
    }
}

// block of code from Beej’s socket programming tutorial
// setup TCP with client at port
int setupTCP(char* port) {
    int rv;
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    struct sigaction sa;
    int yes = 1;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((rv = getaddrinfo(IPADDRESS, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
            perror("setsockopt");
            exit(1);
        }
        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);

    if(p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if(listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    
    sa.sa_handler = sigchld_handler; // reap all dead processes sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1); 
    }
    return sockfd;
}

// block of code from Beej’s socket programming tutorial
// create UDP socket
int setupUDP(char* port)
{
	int sockfd;
	int rv;
	struct addrinfo hints, *servinfo, *p;

	socklen_t addr_len;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(IPADDRESS, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}

	freeaddrinfo(servinfo); // done with servinfo

	return sockfd;
}


void udp_send_and_receive(int sockfd, char *query, char *port, char *data) {
    int numbytes;
    int rv;
    struct addrinfo hints, *servinfo, *p;
	socklen_t addr_len;
	memset(&hints, 0, sizeof hints);
    char recv_data[MAXBUFLEN]; // data received from backend server
    int recv_bytes;
    char* ar[6];   //ar[0] send money  ar[1] reveive money   ar[2] max number
    int wallet_sent = 0;
    int wallet_receive = 0;
    int balance_x = 0;
    char temp[10];

    if ((rv = getaddrinfo(IPADDRESS, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}

    for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}
        break;
    }

    if (p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return;
	}
    
    // send 
	if ((numbytes = sendto(sockfd, query, strlen(query), 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
             }
	
    //receive
    recv_bytes = recvfrom(sockfd, recv_data, sizeof recv_data, 0, NULL, NULL);
    if(recv_bytes == -1) {
        perror("recvfrom");
        exit(1);
    }
    recv_data[recv_bytes] = '\0';

    split_args2(ar, recv_data);
    wallet_sent = wallet_sent + atoi(ar[0]);
    wallet_receive =  wallet_receive + atoi(ar[1]);
    balance_x = wallet_receive-wallet_sent;
    sprintf(temp, "%d",  balance_x);
    strcpy(data, temp);
    strcat(data, " ");
    if(wallet_sent == 0 &&  wallet_receive ==0){
           strcat(data, "0");
        }else{
            strcat(data, "1");
        }
    strcat(data, " ");
    strcat(data, ar[2]);

}

void udp_connection(int sockfd, char *query, char *port,char *data) {
    //int server_sock;
    int numbytes;
    int rv;
    struct addrinfo hints, *servinfo, *p;
	socklen_t addr_len;
	memset(&hints, 0, sizeof hints);
    char recv_data[MAXBUFLEN]; // data received from backend server
    int recv_bytes;
    char* ar[6];   //ar[0] send money  ar[1] reveive money   ar[2] max number
    char temp[10];

    if ((rv = getaddrinfo(IPADDRESS, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}

    for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}
        break;
    }

    if (p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return;
	}
    
    // send 
	if ((numbytes = sendto(sockfd, query, strlen(query), 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}
    if (strcmp(port, SERVERPORTA)==0) {
		printf("The main server sent a request to server A\n");
	} else if (strcmp(port, SERVERPORTB)==0) {
		printf("The main server sent a request to server B\n");
    } else {
        printf("The main server sent a request to server C\n");
    }

     //receive
    recv_bytes = recvfrom(sockfd, recv_data, sizeof recv_data, 0, NULL, NULL);
    if(recv_bytes == -1) {
        perror("recvfrom");
        exit(1);
    }
    recv_data[recv_bytes] = '\0';

     if (strcmp(port, SERVERPORTA)==0) {
		printf("The main server received the feedback from Server A using UDP over port %s\n",SERVERPORTA);
	} else if (strcmp(port, SERVERPORTB)==0) {
		printf("The main server received the feedback from Server B using UDP over port %s\n",SERVERPORTB);
    } else {
        printf("The main server received the feedback from Server C using UDP over port %s\n",SERVERPORTC);
    }

 strcpy(data, recv_data);

    
}
void udp_test(int sockfd, char *query, char *port,char *data) {
    //int server_sock;
    int numbytes;
    int rv;
    struct addrinfo hints, *servinfo, *p;
	socklen_t addr_len;
	memset(&hints, 0, sizeof hints);
    char recv_data[MAXBUFLEN]; // data received from backend server
    int recv_bytes;
    char* ar[6];   //ar[0] send money  ar[1] reveive money   ar[2] max number
    int wallet_sent = 0;
    int wallet_receive = 0;
    int balance_x = 0;
    char temp[10];

    //sockfd = setupUDP(query, port);
    if ((rv = getaddrinfo(IPADDRESS, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}

    for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}
        break;
    }

    if (p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return;
	}   
    // send 
	if ((numbytes = sendto(sockfd, query, strlen(query), 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}
     //receive
    recv_bytes = recvfrom(sockfd, recv_data, sizeof recv_data, 0, NULL, NULL);
    if(recv_bytes == -1) {
        perror("recvfrom");
        exit(1);
    }
    recv_data[recv_bytes] = '\0';
     //printf("receive data :%s\n",recv_data);
     strcpy(data, recv_data);
}



/*--------------------------------------------------------------------------------------------
    --------------------------------------CHECK WALLET--------------------------------------------
    ----------------------------------------------------------------------------------------------*/
void check_wallet(char udp_send[500],int clientfd,int udp_sock, char* PORTCLIENT){
    char udp_received[MAXBUFLEN];
    char* test_result[6];
    char result_buf[100]; // temp saving place for sending results to client though TCP
    char trans[100];//prepare transmitting information to client
    int serial_number = 0;
    int balance = 1000;
    int exist_name = 0;


        //connect to serverA
        udp_send_and_receive(udp_sock, udp_send, SERVERPORTA, udp_received);
        printf("The main server sent a request to server A\n");
        printf("The main server received transactions from Server A using UDP over port %s\n",SERVERPORTA);
        split_args2(test_result, udp_received);
        balance = balance + atoi(test_result[0]);
        exist_name = exist_name + atoi(test_result[1]);

        //connect to serverB
        udp_send_and_receive(udp_sock, udp_send, SERVERPORTB, udp_received);
        printf("The main server sent a request to server B\n");
        printf("The main server received transactions from Server B using UDP over port %s\n",SERVERPORTB);
        split_args2(test_result, udp_received); 
        balance = balance + atoi(test_result[0]);
        exist_name = exist_name + atoi(test_result[1]);

        //connect to serverC
        udp_send_and_receive(udp_sock, udp_send, SERVERPORTC, udp_received);
        printf("The main server sent a request to server C\n");
        printf("The main server received transactions from Server C using UDP over port %s\n",SERVERPORTC);
        split_args2(test_result, udp_received);
        balance = balance + atoi(test_result[0]);
        exist_name = exist_name + atoi(test_result[1]);
        sprintf(trans, "%d", balance);
        strcpy(result_buf, trans);
        strcat(result_buf, "\n");

        if(exist_name ==0){
        strcat(result_buf, "0");
        }else{
            strcat(result_buf, "1");
        }
        if(-1 == (send(clientfd, result_buf, strlen(result_buf), 0))) { 
				perror("send");
			}
         if(strcmp(PORTCLIENT,"25003") ==0){
              printf("The main server sent the current balance to client A\n");
         }   
         if(strcmp(PORTCLIENT,"26003") ==0){
              printf("The main server sent the current balance to client B\n");
         } 
     
}

  /*----------------------------------------------------------------------------------------------
    -----------------------------------------TXCOINS---------------------------------------------
    ----------------------------------------------------------------------------------------------
    splited_args[0]: sender 
    splited_args[1]: receiver
    splited_args[2]: money
    */
void txcoin(char *splited_args[4],int clientfd, int udp_sock,char* PORTCLIENT){

    char udp_send[500];
    char udp_received[MAXBUFLEN];
    char* test_result[6];
    char result_buf[100]; // temp saving place for sending results to client though TCP
    char trans[100];//prepare transmitting information to client
    int serial_number = 0;

    int temp[2]={1,1}; //temp[1,1] :  2 name exist
                       //temp[1,0] :  sender exist
                       //temp[0,1] :  receiver exist
                       //temp[0,0] :  0 name exist
    int compare = 1;// compare = 1:  can write log to file 
                    // compare = 0:  cannot write log to file 
    int mark = 1;   //mark = 1: cannot write to the text
                        //mark = 0: write to the text successfully
    int money_newest[2]={1000,1000}; //the money two people have seperately

    printf("The main server received from <%s> to transfer <%s> coins to <%s> using TCP over port <%s>.\n",splited_args[0],splited_args[2],splited_args[1],PORTCLIENT);
        
    for(int i = 0; i<= 1; i++){
        int balance = 1000; 
        int exist_name = 0;

            sprintf(udp_send, "%s", splited_args[i]);
           
            //connect to serverA
            udp_send_and_receive(udp_sock, udp_send, SERVERPORTA, udp_received);
            split_args2(test_result, udp_received);
            balance = balance + atoi(test_result[0]);
            exist_name = exist_name + atoi(test_result[1]);
            serial_number = max(serial_number,atoi(test_result[2]));

            //connect to serverB
            udp_send_and_receive(udp_sock, udp_send, SERVERPORTB, udp_received);
            split_args2(test_result, udp_received); 
            balance = balance + atoi(test_result[0]);
            exist_name = exist_name + atoi(test_result[1]);
            serial_number = max(serial_number,atoi(test_result[2]));

            //connect to serverC
            udp_send_and_receive(udp_sock, udp_send, SERVERPORTC, udp_received);
            split_args2(test_result, udp_received);
            balance = balance + atoi(test_result[0]);
            exist_name = exist_name + atoi(test_result[1]);
            serial_number = max(serial_number,atoi(test_result[2]));
            
            money_newest[i] = balance;
            
           if(exist_name == 0){
              temp[i] = 0;
              
           }
        
           if(balance < atoi(splited_args[2]) || temp[0] != 1 || temp[1] != 1){
               compare = 0;
           }
        }
        printf("The main server sent a request to server A\n");
        printf("The main server received  the feedback from Server A using UDP over port %s\n",SERVERPORTA);
        printf("The main server sent a request to server B\n");
        printf("The main server received  the feedback from Server B using UDP over port %s\n",SERVERPORTB);
        printf("The main server sent a request to server C\n");
        printf("The main server received  the feedback from Server C using UDP over port %s\n",SERVERPORTC);
      
       if(compare == 1){        //can write the log to text
           int x ; 
           serial_number = serial_number+1;
           srand((unsigned)time(NULL));
           x = rand()%3+1;

            sprintf(udp_send, "%d", serial_number);
            strcat(udp_send, " ");
            strcat(udp_send, splited_args[0]);
            strcat(udp_send, " ");
            strcat(udp_send, splited_args[1]);
            strcat(udp_send, " ");
            strcat(udp_send, splited_args[2]);
            //printf("udp_send : %s,x = %d\n",udp_send,x);
           if (x == 1) {
               udp_connection(udp_sock, udp_send, SERVERPORTA,udp_received);
               split_args2(test_result, udp_received);
               mark = atoi(test_result[0]);
	        } else if (x == 2) {
                udp_connection(udp_sock, udp_send, SERVERPORTB,udp_received);
                split_args2(test_result, udp_received);
                mark = atoi(test_result[0]);
            } else {
                udp_connection(udp_sock, udp_send, SERVERPORTC,udp_received);
                split_args2(test_result, udp_received);
                mark = atoi(test_result[0]);
            }

       }
       //mark different status
        if( mark == 0){
                strcpy(result_buf, "0");
                money_newest[0] = money_newest[0] - atoi(splited_args[2]);
                sprintf(trans, "%d", money_newest[0]);
                strcat(result_buf, "\n");
                strcat(result_buf, trans);

        }else if(temp[1] == 0 && temp[0]== 0){
                 strcpy(result_buf, "1");
        }else if(temp[0] == 0){
                strcpy(result_buf, "2");
        }else if(temp[1] == 0){
                strcpy(result_buf, "3");
        }else if(compare == 0){
                strcpy(result_buf, "4");
                sprintf(trans, "%d", money_newest[0]);
                strcat(result_buf, "\n");
                strcat(result_buf, trans);
            }
            
            //send the success/unsuccesss to client
            if(-1 == (send(clientfd, result_buf, strlen(result_buf), 0))) { 
				perror("send");
			}
            if(strcmp(PORTCLIENT,"25003") ==0){
                printf("The main server sent the result of the transaction to client A.\n");
            }
            if(strcmp(PORTCLIENT,"26003") ==0){
                printf("The main server sent the result of the transaction to client B.\n");
            }

         

}


void start_process(char message_buf[400],int tcpfd_child, int udp_sock,char* PORTCLIENT){
    char *splited_args[4]= {NULL,NULL,NULL,NULL};
    char udp_send[500];

      split_args2(splited_args, message_buf);
             //check wallet
                if(splited_args[1] == NULL && (strcmp(splited_args[0],"TXLIST") !=0)){
                     printf("The main server received input=<%s> from the client using TCP over port <%s>.\n", splited_args[0],PORTCLIENT);        
                     sprintf(udp_send, "%s", splited_args[0]);
                     check_wallet(udp_send,tcpfd_child,udp_sock,PORTCLIENT);          
                }

             //txcoin 
                if(splited_args[1] != NULL){
                txcoin(splited_args,tcpfd_child,udp_sock,PORTCLIENT);
                }  

            //txlist
                if(splited_args[1] == NULL && (strcmp(splited_args[0],"TXLIST") ==0)){
                    
                     char udp_received[MAXBUFLEN];
                     char data_txlist[6000];
                     char save[6000];
                     char result[6000];
                     char *split_line[1000];
                     char *save_line[1000];
                     char *split_data[100];
                     char *result_line[1000];
                     int  max_serial = 1;
                     int  i = 0;
                
                printf("A TXLIST request has been received from the client using TCP over port <%s>.\n", PORTCLIENT);

                memset(data_txlist,'\0',sizeof(data_txlist));
                 sprintf(udp_send, "%s", splited_args[0]);
                 udp_test(udp_sock, udp_send, SERVERPORTA,udp_received);
                 strcat(data_txlist, udp_received);
                 udp_test(udp_sock, udp_send, SERVERPORTB,udp_received);
                 strcat(data_txlist, udp_received);
                 udp_test(udp_sock, udp_send, SERVERPORTC,udp_received);
                 strcat(data_txlist, udp_received);

                 strcpy(save, data_txlist);
                 memset(split_line,'\0',sizeof(split_line));
                
                 split_args(split_line, data_txlist);
                 split_args(save_line, save);

               //sort the data using serial number
                 while(split_line[i] != NULL) {
                     
                     int temp = 0;
                       split_args2(split_data,  split_line[i]);
                       temp = atoi(split_data[0]);
                       result_line[temp] = save_line[i];
                       if(max_serial < atoi(split_data[0])){
                               max_serial = atoi(split_data[0]);
                        }
                        i++;
                 }
                  memset(result,'\0',sizeof(result));
                 for(int j = 1; j <= max_serial; j++){
                       strcat(result, result_line[j]);
                       strcat(result, "\n"); 
                 }
                 strcat(result, "\n"); 

                printf("The sorted file is up and ready.\n");
                  
                 FILE *fp=NULL;
                 fp=fopen(F_PATH,"w");  
                 if(fp == NULL) {
                     printf("File %s not found", F_PATH);
                      exit(1);
                 }
                 fprintf(fp, "%s", result);
                 fclose(fp);
                 fp=NULL;
                if(-1 == (send(tcpfd_child, "0\n3", strlen("0\n3"), 0))) { 
				perror("send");

               

			}

                 }  
}

//use IO multiplexing, let clientA clientB can communicate with serverM
int Authorization(int tcpfd, int tcpfd1, int udp_sock)
{
	int error;
	int fdmax;			// max fd number
	struct sockaddr_in sa_A;
    struct sockaddr_in sa_B;
	socklen_t sin_size_A;
    socklen_t sin_size_B;
    int tcpfd_child_A ;
    int tcpfd_child_B ;
	char ip[INET6_ADDRSTRLEN]; //ip contains the readable ip address
	fd_set master;		// master file descriptor list 
	fd_set read_fds;	// temp fd list for select()


	struct timeval tv;	// set timeot
	tv.tv_sec = 10;
	tv.tv_usec = 0;

	int i, numbytes;
	char message_buf[400];
	int num = 0;

    sin_size_A = sizeof(sa_A);
    sin_size_B = sizeof(sa_B);

	FD_ZERO(&master);	//clear the master and temp sets

	FD_SET(tcpfd, &master);
    FD_SET(tcpfd1, &master);
	fdmax = max(tcpfd,tcpfd1);
    tcpfd_child_A = -1;
    tcpfd_child_B = -1;

	// for loop to monitor socket
	for (;;) {

		read_fds = master; // copy
        // use select() to implement timeout and exit 
		if ((error = select(fdmax+1, &read_fds, NULL, NULL, &tv)) == -1) {
			perror("select");
			exit(1);
		}
		// select() is timeout, it's time to finish Phase1...
		if (error == 0) {
			//printf("select(): error\n");
			continue;
		}
		// select() detects a socket changing its state 
			if (FD_ISSET(tcpfd, &read_fds)) {
				if ( tcpfd_child_A == -1) { // a new connection
					
					tcpfd_child_A = accept(tcpfd, (struct sockaddr *)&sa_A, &sin_size_A);
					if (tcpfd_child_A < 0 ) {
						perror("accept errpr");
                        exit(1);
					}
						FD_SET(tcpfd_child_A, &master);
						fdmax = max(tcpfd_child_A,fdmax);
                }else{
                    printf("cannot run more than one client A\n");
                    struct  sockaddr_in tmpaddr;
                    socklen_t tmpaddrsize = sizeof(tmpaddr) ;
                    close(accept(tcpfd_child_A, (struct sockaddr *)&tmpaddr, &tmpaddrsize));   
                }
            }else if (FD_ISSET(tcpfd1, &read_fds)) {
				if ( tcpfd_child_B == -1) { // a new connection
					
					tcpfd_child_B = accept(tcpfd1, (struct sockaddr *)&sa_B, &sin_size_B);
					if (tcpfd_child_B < 0 ) {
						perror("accept errpr");
                        exit(1);
					}
						FD_SET(tcpfd_child_B, &master);
						fdmax = max(tcpfd_child_B,fdmax);
                }else{
                    printf("cannot run more than one client B\n");
                    struct  sockaddr_in tmpaddr;
                    socklen_t tmpaddrsize = sizeof(tmpaddr) ;
                    close(accept(tcpfd_child_B, (struct sockaddr *)&tmpaddr, &tmpaddrsize));
                    
                }
            }else if (FD_ISSET(tcpfd_child_A, &read_fds)) {
				 if ((numbytes = recv(tcpfd_child_A, message_buf, sizeof(message_buf), 0)) <= 0) {
						if (numbytes == 0) {
							//printf("a client hung up\n");
						} else {
							perror("recv");
						}
						FD_CLR(tcpfd_child_A, &master);
                        close(tcpfd_child_A);
                        tcpfd_child_A = -1;
                        continue;
					}

             start_process(message_buf, tcpfd_child_A, udp_sock,PORTCLIENTA);
              memset(message_buf,'\0',sizeof(message_buf));

            }

            else if (FD_ISSET(tcpfd_child_B, &read_fds)) {
				 if ((numbytes = recv(tcpfd_child_B, message_buf, sizeof(message_buf), 0)) <= 0) {
						if (numbytes == 0) {
							//printf("a client hung up\n");
						} else {
							perror("recv");
						}
                        FD_CLR(tcpfd_child_B, &master);
						close(tcpfd_child_B);
                        tcpfd_child_B = -1;
                        continue;
					}

                   start_process(message_buf, tcpfd_child_B, udp_sock,PORTCLIENTB);
                   memset(message_buf,'\0',sizeof(message_buf));
            
            }else{
                printf("select error!!! exit\n");
                exit(1);
            }
    }

    return 0;
}

             

int main(void) {
    int sockfdA; 
    int sockfdB;
	int udp_sock;

	sockfdA = setupTCP(PORTCLIENTA);
    sockfdB = setupTCP(PORTCLIENTB);
    udp_sock = setupUDP(UDPPORT);
    printf("The main server is up and running.\n");


    while(1) {
          Authorization(sockfdA,sockfdB,udp_sock);
    
    }

    return 0;
}
