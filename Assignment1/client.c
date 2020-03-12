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
#include<math.h>

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

// Inverts random bits based on BER 
void BER(float ber,char transmitted_data[])
{
    int transmitted_data_length = strlen(transmitted_data);

    double temp;

    for(int i=0;i<transmitted_data_length;i++)
    {
        temp = (double)rand() / (double)RAND_MAX ;
        if(temp>=(double)ber)
        {

        }
        else
        {
            if(transmitted_data[i]=='0'){transmitted_data[i]='1';}
            else transmitted_data[i]='0';
        }
        
    }
}

// if received data is completely divisible by generator function then no error was inserted and function returns true 
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

int main(int argc, char  *argv[])
{
    srand(time(0));
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

    //Establish connection with the server
    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Connection failed\n");
        return -1;
    } 

    // Max data bits you can send is 2000 bits
    char data[2000];

    // Generator polynomial is x**8+x**2+x+1;
    int Gen_poly[9] = {1,0,0,0,0,0,1,1,1};

    // transmitted data as string with max size 2009 as last 1 bit is used for sequence number
    char transmitted_data[2009];

    // Received data
    char received_data[2010];

    // Bit error rate
    float ber;
    // Sequence number
    int seq = 0;
    // time to wait before retransmitting i.e. wait for 5 seconds
    int timeout = 5;
    // Data after CRC
    // char after_CRC[2010];
    while (1)
    {
        // Take data as input then add sequence number at its end
        // Then encrpyt it using CRC then add errors using BER which is taken input
        // start the clock after the sending the data
        printf("Enter the data to be transmitted: \n");
        // Removing previous data 
        memset(data, 0, 2000);
        memset(transmitted_data, 0, 2000);
        fgets(data, 2000-1, stdin);
        int size = strlen(data);
        // printf("%d\n",size);
        if(seq==0)
        {
            data[size-1]='0';
        }
        else data[size-1]='1';
        data[size]='\0';
        // printf("%s\n", data);
        CRC(Gen_poly, data, transmitted_data);
        // strcpy(after_CRC, transmitted_data);
        printf("CRC based transmitted data: %s\n", transmitted_data);
        printf("Enter BER:\n");
        scanf("%f", &ber);
        char new_line[100];
        fgets(new_line, 100, stdin);
        if(ber<0 || ber>1)
        {
            printf("Enter correct BER\n");
            return -1;
        }
        
        BER(ber, transmitted_data);
        printf("Data to be transmitted after BER: %s\n", transmitted_data);

        // the attributes are:socket descriptor, buffer containing data, message length, flag
        send(sockfd, transmitted_data, strlen(transmitted_data), 0);
        clock_t start = clock();
        // Message sent now wait to receive message from server
        while (1)
        {
            // Server can send a maximum of 2010 bits data
            // 2000 bits of data 
            // 1 bit for sequence number
            // 1 bit for ACK/NAK
            // rest 8 bits for CRC

            int val = read(sockfd, received_data, 2010);
            if(!val)
            {
                printf("Server has closed the connection\n");
                return -1;
            }

            // If timeout occurs before server has data then retransmit
            if((clock()-start)/CLOCKS_PER_SEC>=timeout)
            {
                CRC(Gen_poly, data, transmitted_data);
                BER(ber, transmitted_data);
                send(sockfd, transmitted_data, strlen(transmitted_data), 0);
                start = clock();
                printf("Timeout Occurred\n");
                continue;
            }
            // If server sends no data then do something
            // ElseIf server has sent the data then 3 cases can occur
            // 1. Received data is not error free then retransmit
            // 2. Received data is error free but NAK then retransmit
            // 3. Received data error free and ACK but for wrong sequence number then retransmit
            // 4. Received data error free and ACK and for right sequence number then take input and send next data with new sequence number

            if(strlen(received_data)==0)
            {

            }
            else
            {
                // Received data in char[] array send it to isErrorFree function to check if the received function has somw error
                int val = isErrorFree(Gen_poly,received_data); 
                // The first bit in received data is flag and second is sequence number 
                // ACK/NAK flag
                int flag = received_data[0]-'0';
                // Received ACK/NAK flag for this sequence number
                int recv_seq = received_data[1]-'0';   
                
                // Case 1       
                if(!val)
                {
                    CRC(Gen_poly, data, transmitted_data);
                    BER(ber, transmitted_data);
                    send(sockfd, transmitted_data, strlen(transmitted_data), 0);
                    printf("Error in received data.\nRetrnasmitting...\n");
                    start = clock();
                }
                // Case 2
                else if(val && !flag)
                {
                    CRC(Gen_poly, data, transmitted_data);
                    BER(ber, transmitted_data);
                    send(sockfd, transmitted_data, strlen(transmitted_data), 0);
                    printf("received NAK.\nRetrnasmitting...\n");
                    start = clock();
                }
                // Case 3
                else if(val && flag && seq!=recv_seq)
                {
                    CRC(Gen_poly, data, transmitted_data);
                    BER(ber, transmitted_data);
                    send(sockfd, transmitted_data, strlen(transmitted_data), 0);
                    printf("Received ACK for wrong sequence number.\nRetrnasmitting...\n");
                    start = clock();
                }
                // Case 4
                else
                {
                    printf("Message received correctly by server \n");
                    seq = 1-seq;
                    break;
                }
                
            }
            
        }
        
    }
    // shuts doen the socket and frees resources allocated to that socket.
    close(sockfd);
    
}