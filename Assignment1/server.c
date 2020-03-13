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

#define MAX_CONNECTIONS 50
#define BUFFER_SIZE 2010
int BIT_ERROR_RATE;

#define server_ip "0.0.0.0"


int allsockets[MAX_CONNECTIONS]; /*  an array to maintain a list of all the sockets that are currently active and being used
 The array is intialised with -1s*/

bool isNumber(char *str)  // checks if the given strign is a number
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




void* print_close_socket(int f){ //prints the connection info of the client that is being disconnected

    struct sockaddr_in sock;
    socklen_t len = sizeof(struct sockaddr_in);
    char str[INET_ADDRSTRLEN];
    getpeername(f, (struct sockaddr *)&sock, (socklen_t *)&len);
    inet_ntop(AF_INET, &(sock.sin_addr), str, INET_ADDRSTRLEN);
    printf("Client disconnected. Ip address: %s and port number %d\n", str, ntohs(sock.sin_port));

}



void* print_connection_info(struct sockaddr_in * accepted_socket){ // prints the connection info of the client that is just connected
    socklen_t len = sizeof(struct sockaddr_in);
    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &((*accepted_socket).sin_addr), str, INET_ADDRSTRLEN);
    printf("New Client Connected. IP address: %s and port number: %d\n", str, ntohs((*accepted_socket).sin_port));
}





void control_c_handler(int sig){ // this is the function that handle Ctrl+C
// If control c is pressed we have to gracefully close the connections 
    printf("Server Closing. Please Wait ........ \n");

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



// takes the generator polynomial, and data as the input and outputs the data with appended crc checksum

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
    transmitted_data[data_bits_length]='\0';
}




// Inverts random bits based on BER 
void BER(float ber,char transmitted_data[])
{
    int transmitted_data_length = strlen(transmitted_data);

    double temp;

    for(int i=0;i<transmitted_data_length;i++)
    {
        temp = (double)rand() / (double)RAND_MAX ;   // generates a random number between 0 and 1
        if(temp>=(double)ber)           // if more than ber than do nothing else flip the bits
        {

        }
        else
        {
            if(transmitted_data[i]=='0'){transmitted_data[i]='1';}
            else transmitted_data[i]='0';
        }
        
    }

}


// checks if the recieved_data is error free. similar to crc function
int isErrorFree(int Gen_poly[], char received_data[])
{
    // printf("Received data %s\n", received_data);
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
    /* This is the function that is run whenever a thread is created. Its argument is FD of the accepted socket */
    int fd_accepted_socket = *((int *) arg);
    int Gen_poly[9] = {1,0,0,0,0,0,1,1,1};
    char data_recieved[BUFFER_SIZE+10]; // will hold the data recieved
    int sqno = 0;
    // int bytes_read;
    for(;;){
        memset(data_recieved, 0, BUFFER_SIZE+10);  // flushed with null characters
        if(read(fd_accepted_socket, data_recieved, BUFFER_SIZE) == 0){   // if reading 0 bytes than break and close the connection
            break;                                                     // when the connection is terminated by the client than read func returns 0
                                                                        // if the socket has no new data recieved but the connection is stll there than the program will be in a waiting state at the read call 
        }
        else{                                                          
            if(strlen(data_recieved) == 0){

            }
            else{                                                        // if some data recieved 

                int recieved_sqn_no = data_recieved[strlen(data_recieved)-9]-'0';    //get the sequence number of the recieved data 

                int check = isErrorFree(Gen_poly, data_recieved);    // if data is corrupted or not 

                char sent_data[100];
                memset(sent_data, 0, 100);

                sent_data[2] = '\0';
                struct sockaddr_in sock;
                socklen_t len = sizeof(struct sockaddr_in);
                char str[INET_ADDRSTRLEN];
                memset(str, 0, INET_ADDRSTRLEN);
                
                getpeername(fd_accepted_socket, (struct sockaddr *)&sock, (socklen_t *)&len);
                inet_ntop(AF_INET, &(sock.sin_addr), str, INET_ADDRSTRLEN);

                printf("Value recieved at the server from ip address %s and port number %d : \n", str, ntohs(sock.sin_port));
                printf("%s \n", data_recieved);

                if (check == 1){                // if data is correct
                    // error free
                    if (recieved_sqn_no == sqno){  // recieved correct sequence number 
                    //    send ACK
                        printf("The recieved values are correct Sending ACK \n");
                        sent_data[0] = '1';
                        sent_data[1] = sqno + '0';  // send ack sqn no
                        sqno = 1-sqno;
                    }
                    else{                     // wrong sequence number    

                        printf("The recieved value has wrong sequence number Sending NACK \n");          
                        sent_data[0] = '0';      // send NACK sqn no
                        sent_data[1] = sqno + '0';
                    }

                }
                else{ // data incorrect
                    // CRC check failed send NACK

                    printf("The recieved value has error Sending NACK \n");
                    sent_data[0] = '0';
                    sent_data[1] = sqno + '0';

                }

                char sent_data2[BUFFER_SIZE];
                memset(sent_data2, 0, BUFFER_SIZE);

                CRC(Gen_poly, sent_data, sent_data2);
                printf("Transmitted data after CRC %s \n", sent_data2);
                BER(BIT_ERROR_RATE, sent_data2);
                printf("Transmitted data after Bit error generation %s \n", sent_data2);
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



// main function

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

    int server_port = atoi(argv[1]);   // port number on whcih to run server

    signal(SIGINT, control_c_handler);


    int fd_listening_socket = 0;    // FD of the listening socket
    struct sockaddr_in listening_socket;
    struct sockaddr_storage accepted_socket;
    socklen_t len_accepted_socket;
    int fd_accepted_socket = 0;   // FD of the accepted socket

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
        perror("Unable to start the server socket with required options for server \n"); 
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

// setting listening socket to listen to incoming connections
    if(listen(fd_listening_socket, MAX_CONNECTIONS) < 0)
    {
        perror("Listening error\n");
        return -1;
    }
    else if (listen(fd_listening_socket, MAX_CONNECTIONS) == 0)
    {
        printf("Listening!\n");
    }

    printf("Enter Bit Error rate for the server \n");
    scanf("%f", &BIT_ERROR_RATE);
    
    if(BIT_ERROR_RATE<0 || BIT_ERROR_RATE>1)
    {
        printf("Enter correct BER\n");
        return -1;
    }


    pthread_t threadids[MAX_CONNECTIONS] = {[0 ... MAX_CONNECTIONS -1] = pthread_self()};
    int j = 0;



    while(1){

        len_accepted_socket = sizeof(accepted_socket);

        int k = 0;    // find a entry in allsockets that is unused
        while(k < MAX_CONNECTIONS && allsockets[k] >= 0){
            k++;
        }

        // If not found than continue
        if (k ==  MAX_CONNECTIONS){
            continue;

        }
        else{   // if found than accept connection

            fd_accepted_socket = accept(fd_listening_socket, (struct sockaddr *)&accepted_socket, &len_accepted_socket);

            if(fd_accepted_socket < 0){
                perror("Error in connection \n");
                continue;
            }

            allsockets[k] = fd_accepted_socket;  // register the value in allsockets
            print_connection_info((struct sockaddr_in *)&accepted_socket);
            int l = 0;
            while(l < MAX_CONNECTIONS && allsockets[l] >= 0){
                l++;
            }
            if(l == MAX_CONNECTIONS){   // if after accepting allsockets is full than print warning message
                printf("No More sockets will be accepted. We have reached the socket limit.\n");
            }

            fflush(stdout);

        }

        if ((pthread_create(&threadids[j], NULL, threadfunc, &fd_accepted_socket)) == 0){   // create a thread for the socket


        }
        else{

            // failiure 
            printf("Not able to create thread. Error\n");
            close(fd_accepted_socket);
            for(int i = 0; i < MAX_CONNECTIONS; i++){
                if (allsockets[i] == fd_accepted_socket){
                    allsockets[i] = -1;
                }
            }
            continue;
        }

        fflush(stdout); 

        j++;
        j %= MAX_CONNECTIONS;
  
        while (pthread_kill(threadids[j], 0) == 0 && threadids[j] != pthread_self()){  // find a unused entry in threadids for the next socket
            j++;                                                                       // pthread_kill(id, 0) tells if a thread is currently running or not 
            j %= MAX_CONNECTIONS;

        }

        fflush(stdout);

        if(threadids[j] != pthread_self()){
            pthread_join(threadids[j], NULL);
            threadids[j] = pthread_self();
        
        }

    }
}
