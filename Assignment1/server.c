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
#include <pthread.h>
#include <math.h>
#include<sys/types.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_CONNECTIONS 2
#define BUFFER_SIZE 2010
#define BIT_ERROR_RATE 0.00

#define server_ip "0.0.0.0"


int allsockets[MAX_CONNECTIONS];


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




void* print_close_socket(int f){

    struct sockaddr_in sock;
    socklen_t len = sizeof(struct sockaddr_in);
    char str[INET_ADDRSTRLEN];
    getpeername(f, (struct sockaddr *)&sock, (socklen_t *)&len);
    inet_ntop(AF_INET, &(sock.sin_addr), str, INET_ADDRSTRLEN);
    printf("Client disconnected. Ip address: %s and port number %d\n", str, ntohs(sock.sin_port));

}



void* print_connection_info(struct sockaddr_in * accepted_socket){
    socklen_t len = sizeof(struct sockaddr_in);
    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &((*accepted_socket).sin_addr), str, INET_ADDRSTRLEN);
    printf("New Client Connected. IP address: %s and port number: %d\n", str, ntohs((*accepted_socket).sin_port));
}





void control_c_handler(int sig){

    printf("Server Closing. Please Wait \n");

    for(int i = 0; i < MAX_CONNECTIONS; i++){
        if(allsockets[i] != -1){
            print_close_socket(allsockets[i]);
            close(allsockets[i]);
            allsockets[i] = -1;
        }
    }
    printf("Done. Have a nice day \n");

    fflush(stdout);
    exit(0);


    // do the control c handling


}




void CRC(int Gen_poly[], char data[], char transmitted_data[])
{    

    int data_length = strlen(data);
    
    // data_bits after appending data with 8 zeros
    int data_bits[data_length + 8];

    for(int i=0;i<data_length;i++)
    {
        data_bits[i]=data[i]-'0';
    }
    
    for(int i=0;i<8;i++)
    {
        data_bits[data_length+i]=0;
    }

    int data_bits_length = sizeof(data_bits)/sizeof(data_bits[0]);

    // temp stores current dividend being evaluated
    int temp[9];
    for(int i=0;i<9;i++)
    {
        temp[i]=data_bits[i];
    }

    // remainder stores the remainder after completion of division
    int remainder[8];

    // temp_Gen_poly stores the temporary divisor on current dividend is being divided
    // the result after temporary division is stored back in temp as next dividend
    int temp_Gen_poly[9];
    for(int i=9;i<=data_bits_length;i++)
    {
        if(temp[0]==0)
        {
            for(int j=0;j<9;j++){temp_Gen_poly[j]=0;}
        }
        else 
        {
            for(int j=0;j<9;j++){temp_Gen_poly[j]=Gen_poly[j];}
        }
        
        for(int k=1;k<9;k++)
        {
            if(temp[k]==temp_Gen_poly[k]){temp[k-1]=0;}
            else temp[k-1]=1;
        }
        if(i!=data_bits_length){temp[8]=data_bits[i];}
        else 
        {
            for(int l=0;l<8;l++)
            {
                remainder[l]=temp[l];
            }
        }
    }
    char temp_char;
    for(int i=0;i<data_bits_length-8;i++)
    {
        if(data_bits[i]==0){temp_char='0';}
        else temp_char='1';
        transmitted_data[i]=temp_char;
    }

    // remainder is added to the data to be sent as transmitted data
    for(int i=0;i<8;i++)
    {
        if(remainder[i]==0){temp_char='0';}
        else temp_char='1';
        transmitted_data[data_bits_length-8+i]=temp_char;
    }
    transmitted_data[data_bits_length+1]='\0';
}





void BER(float ber,char transmitted_data[])
{
    int transmitted_data_length = strlen(transmitted_data);
    // number of bits you want error in 
    int total_error = floor(ber*transmitted_data_length);
    printf("Bits to be reversed: %d\n",total_error);

    int uniqueflag;
    int error_indices[total_error];
    int random;
    for(int i = 0; i < total_error; i++) 
    {
        do {
            /* Assume things are unique... we'll reset this flag if not. */
            uniqueflag = 1;
            random = rand() % transmitted_data_length+ 1;
            /* This loop checks for uniqueness */
            for (int j = 0; j < i && uniqueflag == 1; j++) 
            {
                if (error_indices[j] == random) 
                {
                    uniqueflag = 0;
                }
            }
        } while (uniqueflag != 1);
        error_indices[i] = random;
        // printf("Index %d\n", random);
    }

    
    for(int i=0;i<total_error;i++)
    {
        // printf("%d\n", error_indices[i]);
        if(transmitted_data[error_indices[i]]=='0'){transmitted_data[error_indices[i]]='1';}
        else transmitted_data[error_indices[i]]='0';
    }


}




int isErrorFree(int Gen_poly[], char received_data[])
{
    printf("Received data %s\n", received_data);
    int received_data_length = strlen(received_data);
    int received_data_bits[received_data_length];

    for(int i=0;i<received_data_length;i++)
    {
        received_data_bits[i]=received_data[i]-'0';
    }
    // temp stores current dividend being evaluated
    int temp[9];
    for(int i=0;i<9;i++)
    {
        temp[i]=received_data_bits[i];
    }

    // remainder stores the remainder after completion of division
    int remainder[8];

    // temp_Gen_poly stores the temporary divisor on current dividend is being divided
    // the result after temporary division is stored back in temp as next dividend
    int temp_Gen_poly[9];
    for(int i=9;i<=received_data_length;i++)
    {
        if(temp[0]==0)
        {
            for(int j=0;j<9;j++){temp_Gen_poly[j]=0;}
        }
        else 
        {
            for(int j=0;j<9;j++){temp_Gen_poly[j]=Gen_poly[j];}
        }
        
        for(int k=1;k<9;k++)
        {
            if(temp[k]==temp_Gen_poly[k]){temp[k-1]=0;}
            else temp[k-1]=1;
        }
        if(i!=received_data_length){temp[8]=received_data_bits[i];}
        else 
        {
            for(int l=0;l<8;l++)
            {
                remainder[l]=temp[l];
            }
        }
    }

    for(int i=0;i<8;i++)
    {
        if(remainder[i])
        {
            return 0;
        }

    }
    return 1;
}


void * threadfunc(void * arg){
    int fd_accepted_socket = *((int *) arg);
    int Gen_poly[9] = {1,0,0,0,0,0,1,1,1};
    char data_recieved[BUFFER_SIZE+10];
    int sqno = 0;
    // int bytes_read;
    for(;;){
        if(read(fd_accepted_socket, data_recieved, BUFFER_SIZE) == 0){
            break;

        }
        else{
            if(strlen(data_recieved) == 0){

            }
            else{

                int recieved_sqn_no = data_recieved[strlen(data_recieved)-9]-'0';

                int check = isErrorFree(Gen_poly, data_recieved);

                char sent_data[100];
                sent_data[2] = '\0';
                struct sockaddr_in sock;
                socklen_t len = sizeof(struct sockaddr_in);
                char str[INET_ADDRSTRLEN];
                getpeername(fd_accepted_socket, (struct sockaddr *)&sock, (socklen_t *)&len);
                inet_ntop(AF_INET, &(sock.sin_addr), str, INET_ADDRSTRLEN);
                // printf("Message Recieved from %s and port number %d is : %s\n", str, ntohs(sock.sin_port), data_recieved);
                // printf("Recieved Sqno: %d", recieved_sqn_no);

                if (check == 1){
                    // error free
                    if (recieved_sqn_no == sqno){
                        // send ACK
                        sent_data[0] = '1';
                        sent_data[1] = sqno + '0';
                        sqno = 1-sqno;
                    }
                    else{
                        // send NACK
                        sent_data[0] = '0';
                        sent_data[1] = sqno + '0';
                    }

                }
                else{
                    // error is there
                    // send NACK

                    printf("The recieved value has error");
                    sent_data[0] = '0';
                    sent_data[1] = sqno + '0';

                }
                char sent_data2[BUFFER_SIZE];

                CRC(Gen_poly, sent_data, sent_data2);
                printf("Transmitted data after CRC %s\n", sent_data2);
                BER(BIT_ERROR_RATE, sent_data2);
                printf("Transmitted data after Bit error generation %s\n", sent_data2);
                send(fd_accepted_socket, sent_data2, strlen(sent_data2), 0);
                fflush(stdout);


            }

        }


    }



    // terminate the thread and connection
    // close socket
    print_close_socket(fd_accepted_socket);
    close(fd_accepted_socket);
    for(int i = 0; i < MAX_CONNECTIONS; i++){
        if (allsockets[i] == fd_accepted_socket){
            allsockets[i] = -1;
        }
    }
    fflush(stdout);

    pthread_exit(NULL);
    // close the thread

}







int main(int argc, char* argv[])
{



    for(int i = 0; i < MAX_CONNECTIONS; i++){
        allsockets[i] = -1;
    }


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


pthread_t threadids[MAX_CONNECTIONS] = {[0 ... MAX_CONNECTIONS -1] = pthread_self()};
int j = 0;



    while(1){

        len_accepted_socket = sizeof(accepted_socket);

        int k = 0;
        while(k < MAX_CONNECTIONS && allsockets[k] >= 0){
            k++;
        }


        if (k ==  MAX_CONNECTIONS){
            continue;

        }
        else{

            fd_accepted_socket = accept(fd_listening_socket, (struct sockaddr *)&accepted_socket, &len_accepted_socket);

            if(fd_accepted_socket < 0){
                perror("Error in connection");
                continue;
            }

            allsockets[k] = fd_accepted_socket;
            print_connection_info((struct sockaddr_in *)&accepted_socket);
            int l = 0;
            while(l < MAX_CONNECTIONS && allsockets[l] >= 0){
                l++;
            }
            if(l == MAX_CONNECTIONS){
                printf("No More sockets will be accepted. We have reached the socket limit.\n");
            }

            fflush(stdout);

        }




        if ((pthread_create(&threadids[j], NULL, threadfunc, &fd_accepted_socket)) == 0){
            // printf("Thread created %ld\n", threadids[j]);
            // success
        }
        else{

            // failiure 
            printf("Not able to create thread. Error\n");
            close(fd_accepted_socket);
            continue;
        }


        fflush(stdout);

        j++;
        j %= MAX_CONNECTIONS;

        while (pthread_kill(threadids[j], 0) == 0 && threadids[j] != pthread_self()){
            j++;
            j %= MAX_CONNECTIONS;

        }
        // printf("Dodo");
        // fflush(stdout);

        if(threadids[j] != pthread_self()){
            pthread_join(threadids[j], NULL);
            // printf("Thread joined %ld\n", threadids[j]);
            threadids[j] = pthread_self();
        }









    }
    

    
}
