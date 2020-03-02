#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>
#include<stdbool.h>
#include<limits.h>
#include<time.h>
#include<ctype.h>


int isValidIp4 (char *str) {
    int segs = 0;   /* Segment count. */
    int chcnt = 0;  /* Character count within segment. */
    int accum = 0;  /* Accumulator for segment. */

    /* Catch NULL pointer. */

    if (str == NULL)
        return 0;

    /* Process every character in string. */

    while (*str != '\0') {
        /* Segment changeover. */

        if (*str == '.') {
            /* Must have some digits in segment. */

            if (chcnt == 0)
                return 0;

            /* Limit number of segments. */

            if (++segs == 4)
                return 0;

            /* Reset segment values and restart loop. */

            chcnt = accum = 0;
            str++;
            continue;
        }
        /* Check numeric. */

        if ((*str < '0') || (*str > '9'))
            return 0;

        /* Accumulate and check segment. */

        if ((accum = accum * 10 + *str - '0') > 255)
            return 0;

        /* Advance other segment specific stuff and continue loop. */

        chcnt++;
        str++;
    }

    /* Check enough segments and enough characters in last segment. */

    if (segs != 3)
        return 0;

    if (chcnt == 0)
        return 0;

    /* Address okay. */

    return 1;
}

bool isNumber(char *str)
{
    int i=0;
    if(str[0]=='-')
    {
        i=1;
    }
    for(;str[i]!='\0';i++)
    {
        if(!isdigit(str[i])){return false;}
    }
    return true;
}

int main(int argc, char  *argv[])
{
    
    if(argc!=3)
    {
        printf("Input not in specified format\n");
        return -1;
    }

    int server_port = atoi(argv[2]);
    if(server_port<0 || server_port>65535 || !isNumber(argv[2]))
    {
        printf("Port number not correct\n");
        return -1;
    }

    char * server_ip;
    if(isValidIp4(argv[1]))
    {
        server_ip=argv[1];
    }
    else
    {
        printf("ip not in valid format\n");
        return -1;
    }

    // socket descriptor
    int sockfd = 0;

    //socket address struct
    struct sockaddr_in server_addr;

    // Creating IPv4, TCP, IP socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket Creation error\n");
        return -1;
    }

    // Assigning ipv4 family and port number to server address struct
    server_addr.sin_family = AF_INET;
    
    // Converts integer from host byte order to network byte order
    server_addr.sin_port = htons(server_port);
    
    // Convert IPv4 address from text to binary and raise error if wrong address
    if(inet_pton(AF_INET, argv[1], &server_addr.sin_addr)<=0)
    {
        printf("Invalid address\n");
        return -1;
    }

    // Establish connection with the server
    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Connection failed\n");
        return -1;
    }
    



    
    
}