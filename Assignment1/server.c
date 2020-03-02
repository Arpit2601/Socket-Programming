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
// #include<netinet/in.h>
#include<sys/types.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_CONNECTIONS 100

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

int main(int argc, char* argv[])
{
    if(argc!=2)
    {
        perror("Invalid number of arguments\n");
        return -1;
    }

    int server_port = atoi(argv[1]);
    if(server_port<0 || server_port>65535 || !isNumber(argv[1]))
    {
        perror("Port number not correct\n");
        return -1;
    }

    int sockfd=0;
    struct sockaddr_in server_addr;

    // Creating socket file descriptor 
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))<=0)
    {
        perror("Socket creation failed\n");
        return -1;
    }

    int opt=1;
    // Forcefully attaching socket to the port  
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){ 
        perror("Unable to start the server socket with required options for server"); 
        return -1; 
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;   // localhost address
    server_addr.sin_port = htons(server_port);

    // Binding socket to port specified as input
    if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr))<0)
    {
        perror("Binding failed\n");
        return -1;
    }

    if(listen(sockfd, MAX_CONNECTIONS) < 0)
    {
        perror("Listen error\n");
        return -1;
    }
    else if (listen(sockfd, MAX_CONNECTIONS) == 0)
    {
        printf("Listening!\n");
    }
    

    
}