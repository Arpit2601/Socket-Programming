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
#include <string.h>
// #include<netinet/in.h>
#include<sys/types.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_CONNECTIONS 100


string server_ip  = "0.0.0.0";





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


void control_c_handler(int sig){



    // do the control c handling


}



// void printnew(int socketfd){
//     struct sockaddr_in address;
//     socklen_t addrlen = sizeof(struct sockaddr_in);
//     getpeername(socketfd,(struct sockaddr*)&address,(socklen_t*)&addrlen);
//     printf("New client connected from %s:%d\n",inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
// }


void * threadfunc(void * arg){
    int fd_accepted_socket = *((int *) arg);

    




}





int print_connection_info(){

// print tthe socket ifo as required



}







int main(int argc, char* argv[])
{
    if(argc!=2)
    {
        perror("Invalid number of arguments\n");
        return -1;
    }

    int server_port = atoi(argv[1]);


    signal(SIGINT, control_c_handler);

    int fd_listening_socket = 0;
    struct sockaddr_in listening_socket;
    struct sockaddr_storage accepted_socket;
    socklen_t len_accepted_socket;
    int fd_accepted_socket = 0;



    if(server_port<0 || server_port>65535 || !isNumber(argv[1]))
    {
        perror("Port number not correct\n");
        return -1;
    }


    // Creating socket file descriptor 
    if((fd_listening_socket = socket(AF_INET, SOCK_STREAM, 0))<=0)
    {
        perror("Socket creation failed\n");
        return -1;
    }

    int opt=1;
    // Forcefully attaching socket to the port  
    


    if (setsockopt(fd_listening_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){ 
        perror("Unable to start the server socket with required options for server"); 
        return -1; 
    }

    listening_socket.sin_family = AF_INET;
    listening_socket.sin_addr.s_addr = inet_addr(server_ip);   // localhost address
    listening_socket.sin_port = htons(server_port);

    memset((listening_socket.sin_zero), 0 , sizeof(listening_socket.sin_zero));




    // Binding socket to port specified as input
    if(bind(fd_listening_socket, (struct sockaddr *)&listening_socket, sizeof(listening_socket))<0)
    {
        perror("Binding failed\n");
        return -1;
    }






    if(listen(fd_listening_socket, MAX_CONNECTIONS) < 0)
    {
        perror("Listening error\n");
        return -1;
    }
    else if (listen(fd_listening_socket, MAX_CONNECTIONS) == 0)
    {
        printf("Listening!\n");
    }


pthread_t threadids[MAX_CONNECTIONS+10] = {[0 ... MAX_CONNECTIONS +9] = pthread_self()};
int j = 0



    while(1){

        len_accepted_socket = size(accepted_socket);

        fd_accepted_socket = accept(fd_listening_socket, (struct sockaddr *)&accepted_socket, &len_accepted_socket);


        print_connection_info((struct sockaddr_in *)&accepted_socket);


        if (pthread_create(&threadids[j], NULL, threadfunc, &fd_accepted_socket) == 0){

            // success
        }
        else{

            // failiure 
            printf("Not able to create thread. Error")
        }

        j++;
        j %= MAX_CONNECTIONS;

        while (pthread_kill(threadids[j], 0) == 0 && threadids[j] != pthread_self()){
            j++;
            j %= MAX_CONNECTIONS;

        }


        if(threadids[j] != pthread_self()){
            pthread_join(threadids[j], NULL);
            threadids[j] = pthread_self();
        }








    }
    

    
}
