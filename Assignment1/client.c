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

void CRC(int Gen_poly[], int data_bits[], char transmitted_data[], int data_bits_length)
{    
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

void BER(float ber,char transmitted_data[],int transmitted_data_length)
{
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
    }
    for(int i=0;i<total_error;i++)
    {
        if(transmitted_data[error_indices[i]]=='0'){transmitted_data[i]='1';}
        else transmitted_data[error_indices[i]]='0';
    }


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
    // if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    // {
    //     printf("Connection failed\n");
    //     return -1;
    // } 

    printf("Enter the data to be transmitted\n");

    // data taken in 0s and 1s as string with max length=2000
    // data appended and coneverted to int array as data_bits
    // data to be transmitted as transmitted_data as string
    char data[2000];
    fgets(data, 1000, stdin);
    data[strlen(data)-1]='\0';

    // Generator polynomial is x**8+x**2+x+1;
    int Gen_poly[9] = {1,0,0,0,0,0,1,1,1};
    
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

    // transmitted data as string with max size 2008
    char transmitted_data[2008];
    int data_bits_length = sizeof(data_bits)/sizeof(data_bits[0]);
    CRC(Gen_poly, data_bits, transmitted_data, data_bits_length);
    printf("CRC based transmitted data: %s\n", transmitted_data);

    printf("Enter BER\n");
    float ber;
    scanf("%f", &ber);
    if(ber<0 || ber>1)
    {
        printf("Enter correct BER\n");
        return -1;
    }
    int transmitted_data_length = data_bits_length;
    // printf("%d\n",transmitted_data_length);
    BER(ber, transmitted_data, transmitted_data_length);

    printf("Data to be transmitted after BER: %s\n", transmitted_data);
}