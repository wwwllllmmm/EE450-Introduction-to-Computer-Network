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
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>

#define IPADDRESS "127.0.0.1" // local IP address
#define MYPORT "22003"        // the port used for UDP connection with AWS
#define BLOCK "./block2.txt"  // file directory to get block information
#define MAXBUFLEN 4000

// block of code from Beej’s socket programming tutorial
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// split  message into char[]
void split_args(char *args[], char *message)
{
    char *p = strtok(message, " ");
    int i = 0;
    while (p != NULL)
    {
        args[i++] = p;
        p = strtok(NULL, " ");
    }
}

// serach send_money base on name
int search_sendmoney(char *name)
{
    char *line = NULL;
    char *ar[5];
    size_t len = 0;
    ssize_t read;
    int solution;
    int coin_send = 0;

    FILE *file = fopen(BLOCK, "r");
    if (file == NULL)
    {
        printf("File %s not found", BLOCK);
        exit(1);
    }

    while ((read = getline(&line, &len, file)) != -1)
    {
         ar[3] = "0" ;
        split_args(ar, line);
        solution = strcmp(name, ar[1]);
        if (solution == 0)
        {
            coin_send = coin_send + atoi(ar[3]);
        }
    }
    return coin_send;
}

// serach receive_money base on name
int search_reveivemoney(char *name)
{
    char *line = NULL;
    char *ar[5];
    size_t len = 0;
    ssize_t read;
    int solution;
    int coin_receive = 0;

    // open corresponding txt file
    FILE *file = fopen(BLOCK, "r");
    if (file == NULL)
    {
        printf("File %s not found", BLOCK);
        exit(1);
    }
    while ((read = getline(&line, &len, file)) != -1)
    {
         ar[3] = "0" ;
        split_args(ar, line);
        solution = strcmp(name, ar[2]);
        if (solution == 0)
        {
            coin_receive = coin_receive + atoi(ar[3]);
        }
    }
    return coin_receive;
}

//serach serial number base on name
int search_maxnumber(char *name)
{
    char *line = NULL;
    char *ar[5];
    size_t len = 0;
    ssize_t read;
    int solution;
    int max_serial = 0;

    // open corresponding txt file
    FILE *file = fopen(BLOCK, "r");
    if (file == NULL)
    {
        printf("File %s not found", BLOCK);
        exit(1);
    }
    while ((read = getline(&line, &len, file)) != -1)
    {
         ar[3] = "0" ;
        split_args(ar, line);
        if (max_serial < atoi(ar[0]))
        {
            max_serial = atoi(ar[0]);
        }
    }
    return max_serial;
}

// block of code from Beej’s socket programming tutorial
int setupUDP(char *port)
{
    int sockfd;
    int rv;
    struct addrinfo hints, *servinfo, *p;

    socklen_t addr_len;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(IPADDRESS, port, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("talker: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }
    if (p == NULL)
    {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }

    freeaddrinfo(servinfo); // done with servinfo

    return sockfd;
}

int main(void)
{
    int sockfd;
    int numbytes;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;

    char recv_data[MAXBUFLEN];
    char data[MAXBUFLEN];

    addr_len = sizeof their_addr;

    sockfd = setupUDP(MYPORT);

    printf("The Server B is up and running using UDP on port %s\n", MYPORT);

    while (1)
    {
        char *args[4] = {NULL, NULL, NULL, NULL};
        int coin_send = 0;
        int coin_receive = 0;
        int max_serial = 0;

        // receive message from serverM
        if ((numbytes = recvfrom(sockfd, recv_data, MAXBUFLEN - 1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
        {
            perror("recvfrom");
            exit(1);
        }
        recv_data[numbytes] = '\0';

        printf("The Server B received a request from the Main Server.\n");

        // split messages into char[]
        split_args(args, recv_data);

         //check wallet
        if (args[2] == NULL && strcmp(args[0], "TXLIST") != 0)
        {
            coin_send = search_sendmoney(args[0]);
            coin_receive = search_reveivemoney(args[0]);
            max_serial = search_maxnumber(args[0]);

            char c1[100];
            char c2[100];
            // int -> char*
            sprintf(c1, "%d", coin_send);
            sprintf(c2, "%d", coin_receive);
            // sort out c1,c2 ,prepare transmitting information to serverM
            strcpy(data, c1);
            strcat(data, " ");
            strcat(data, c2);
            sprintf(c2, "%d", max_serial);
            strcat(data, " ");
            strcat(data, c2);

            // send message to serverM
            numbytes = sendto(sockfd, data, strlen(data), 0, (struct sockaddr *)&their_addr, addr_len);
            if (numbytes == -1)
            {
                perror("listener: sendto");
                exit(1);
            }

            printf("The Server B finished sending the response to the Main Server.\n");
        }

        //txcoin--write to txt
        if (args[2] != NULL)
        {
            FILE *file = fopen(BLOCK, "a");
            if (file == NULL)
            {
                printf("File %s not found", BLOCK);
                exit(1);
            }
            fprintf(file, "\n%s %s %s %s", args[0], args[1], args[2], args[3]);
            fclose(file);
            strcpy(data, "0");
            numbytes = sendto(sockfd, data, strlen(data), 0, (struct sockaddr *)&their_addr, addr_len);
            if (numbytes == -1)
            {
                perror("listener: sendto");
                exit(1);
            }
            printf("The Server B finished sending the response to the Main Server.\n");
        }

         //txlist
        if (args[2] == NULL && strcmp(args[0], "TXLIST") == 0)
        {
            char *line = NULL;
            char *ar[5];
            size_t len = 0;
            ssize_t read;
            int solution;
            int coin_receive = 0;
            char txlist[5000];
            char args_str[100];

            memset(txlist, '\0', sizeof(txlist));
            FILE *file = fopen(BLOCK, "r");
            if (file == NULL)
            {
                printf("File %s not found", BLOCK);
                exit(1);
            }

            while ((read = getline(&line, &len, file)) != -1)
            {
                split_args(ar, line);
                sprintf(args_str, "%s %s %s %s", ar[0], ar[1], ar[2], ar[3]);
                strcat(txlist, args_str);
            }
            strcat(txlist, "\n");

            numbytes = sendto(sockfd, txlist, strlen(txlist), 0, (struct sockaddr *)&their_addr, addr_len);
            if (numbytes == -1)
            {
                perror("listener: sendto");
                exit(1);
            }
            memset(txlist, '\0', sizeof(txlist));
            printf("The Server B finished sending the response to the Main Server.\n");
        }
    }

    return 0;
}
