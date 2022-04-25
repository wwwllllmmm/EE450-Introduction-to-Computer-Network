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
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>

#define PORT "25003" // the TCP port of serverM which client A connects to
#define IPADDRESS "localhost"
#define MAXBUFLEN 4000 // the maximum number of bytes

// split results into char
void split_result(char *args[], char *message)
{
    char *p = strtok(message, "\n");
    int i = 0;
    while (p != NULL)
    {
        args[i++] = p;
        p = strtok(NULL, "\n");
    }
}

// block of code from Beej’s socket programming tutorial
// funciton to get socket address from a sockaddr struct
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET6)
    {
        // IPv6
        return &(((struct sockaddr_in6 *)sa)->sin6_addr);
    }
    // IPv4
    return &(((struct sockaddr_in *)sa)->sin_addr);
}

// block of code from Beej’s socket programming tutorial
int setupTCP(char *port)
{
    int received;
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    struct sigaction sa;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((received = getaddrinfo(IPADDRESS, PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(received));
        return 1;
    }

    // loop all the results and connect the first
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            continue;
        }
        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
    printf("The client A is up and running.\n");

    freeaddrinfo(servinfo); // all done with this structure
    return sockfd;
}

int main(int argc, char *argv[])
{
    int sockfd;
    int numbytes;
    char buf[MAXBUFLEN];
    char *results[5];
    char args_str[100];
    char *check[10];
    char *num;

    // check whether the message is enough
    if (2 != argc && 4 != argc)
    { 
        fprintf(stderr, "usage: ./clientA <username1> or \n usage: ./clientA <username1> <username2> <transfer amount>\n");
        exit(1);
    }

    /*--------------------------------------------------------------------------------------------
    --------------------------------------CHECK WALLET--------------------------------------------
    ----------------------------------------------------------------------------------------------*/
    if (2 == argc && (strcmp(argv[1], "TXLIST") != 0))
    {
        sockfd = setupTCP(PORT);

        printf("<%s> sent a balance enquiry request to the main server.\n", argv[1]);

        sprintf(args_str, "%s", argv[1]);
        //send the message to serverM
        if ((numbytes = send(sockfd, args_str, strlen(args_str), 0)) == -1)
        {
            perror("send");
            exit(1);
        }
        fflush(stdout);

        if (-1 == (numbytes = recv(sockfd, buf, MAXBUFLEN - 1, 0)))
        {
            perror("recv");
            exit(1);
        }
        buf[numbytes] = '\0'; // The last byte is set as the terminator.
        split_result(check, buf);

        if (atoi(check[1]) == 1)
        {
            printf("The current balance of <%s> is : <%s> alicoins.\n", argv[1], check[0]);
        }
        else if (atoi(check[1]) == 0)
        {
            printf("%s is not part of the network.\n", argv[1]);
        }
    }

    /*----------------------------------------------------------------------------------------------
       -----------------------------------------TXCOINS---------------------------------------------
       ---------------------------------------------------------------------------------------------
       0 :  successful
       1 :  if both the clients are not part of the network
       2 :  if one of the clients is not part of the network  (SENDER_USERNAME)
       3 :  if one of the clients is not part of the network  (RECEIVER_USERNAME)
       4 :  if transaction fails due to insufficient balance
       -*/
    if (4 == argc)
    {
        //check the transfer number
        num = argv[3];
        for (int i = 0; i < strlen(argv[3]); i++)
        {
            if (!isdigit(num[i]))
            {
                printf("transfer number should be an integer.\n");
                exit(1);
            }
        }
        sockfd = setupTCP(PORT);

        sprintf(args_str, "%s %s %s", argv[1], argv[2], argv[3]);
        if ((numbytes = send(sockfd, args_str, strlen(args_str), 0)) == -1)
        {
            perror("send");
            exit(1);
        }
        fflush(stdout);

        printf("<%s> has requested to transfer <%s> coins to <%s>.\n", argv[1], argv[3], argv[2]);

        if (-1 == (numbytes = recv(sockfd, buf, MAXBUFLEN - 1, 0)))
        {
            perror("recv");
            exit(1);
        }
        buf[numbytes] = '\0'; // The last byte is set as the terminator.
        split_result(check, buf);

        if (atoi(check[0]) == 0)
        {
            printf("<%s> successfully transferred <%s> alicoins to <%s>.\n", argv[1], argv[3], argv[2]);
            printf("The current balance of <%s> is : <%s> alicoins.\n", argv[1], check[1]);
        }
        else if (atoi(check[0]) == 1)
        {
            printf("Unable to proceed with the transaction as <%s> and <%s> are not part of the network.\n", argv[1], argv[2]);
        }
        else if (atoi(check[0]) == 2)
        {
            printf("Unable to proceed with the transaction as <%s> is not part of the network.\n", argv[1]);
        }
        else if (atoi(check[0]) == 3)
        {
            printf("Unable to proceed with the transaction as <%s> is not part of the network.\n", argv[2]);
        }
        else if (atoi(check[0]) == 4)
        {
            printf("<%s> was unable to transfer <%s> alicoins to <%s> because of insufficient balance.\n", argv[1], argv[3], argv[2]);
            printf("The current balance of <%s> is : <%s> alicoins.\n", argv[1], check[1]);
        }
    }
     /*--------------------------------------------------------------------------------------------
    --------------------------------------TXLIST---------------------------------------------------
    ----------------------------------------------------------------------------------------------*/
    if (2 == argc && (strcmp(argv[1], "TXLIST") == 0))
    {
        sockfd = setupTCP(PORT);

        printf("client A sent a sorted list request to the main server.\n");

        sprintf(args_str, "%s", argv[1]);
        if ((numbytes = send(sockfd, args_str, strlen(args_str), 0)) == -1)
        {
            perror("send");
            exit(1);
        }
        fflush(stdout);

        if (-1 == (numbytes = recv(sockfd, buf, MAXBUFLEN - 1, 0)))
        {
            perror("recv");
            exit(1);
        }
        buf[numbytes] = '\0'; // The last byte is set as the terminator.
    }

    close(sockfd);
    return 0;
}
