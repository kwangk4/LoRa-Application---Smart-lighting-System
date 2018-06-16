 /* client  program */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "sys/socket.h"
#include "sys/types.h"
#include "netinet/in.h"
#include "error.h"
#include "strings.h"
#include "unistd.h"
#include "arpa/inet.h"
#include "ctype.h"

#include <pthread.h>
#include <semaphore.h>


#define BUFFER    1024

typedef enum 
{   
    GROUP_ACK,
  GROUP_DATA,
  GROUP_CONTROL,
  GROUP_QUERY,
    GROUP_FAIL
}GROUP_ORDER;

GROUP_ORDER t;

int Get_data_socket_server();


int sosanhchuoi(char *s1,char *s2) //s1 la chuoi cho truoc, s2 la chuoi nhap tu ban phim
{     
  int n, m;
  n=strlen(s1);m=strlen(s2);   /*n,m lan luot la do dai chuoi s1,s2. DO s2 la chuoi nhap tu ban phim nen do dai chuoi tinh 
  luon ki tu NULL*/
  
    while (n==(m-1))     //Xem xet chuoi s1 co nho hon chuoi s2 hay ko?neu lon hon thi ko the la con cua s2 thi tra ve gtri -1
      { 
        
          while (*s1++ == *s2++)   /*neu tim duoc ki tu s2 = ki tu dau tien cua s1 
                             thi bat buoc cac ki tu sau no phai giong nhau neu khong se tra ve grti -1*/
              {
                if(*s1 == 0)
                return 1;
                else
                continue;
              }
                             /*khi ki tu s1 khac ki tu s2 thi se co 2 TH xay ra
                                         1.Chuoi s1 chay den ki tu NULL nen khac voi ki tu s2  -->tra ve gtri 0 = s1 la con s2
                                         2.Chuoi s1 co ki tu khac voi ki tu chuoi s2   -->tra ve gtri -1 = s1 ko la con s2 */ 
        
          
          return 0;
        
        
        }
        return 0;
}

//========================================================
  //Khai bao cac ham, bien cho tuyen CLIENT_APP
        void *CLIENT_GW(void *arg);
        sem_t bin_sem1;    //khai bao trong chuong trinh co 2 tuyen chay song song
        sem_t bin_sem2;     
        volatile int GloVar = 0;
   //Khai bao cac bien cho socket     
    char in[BUFFER];
    char out[BUFFER];
    char temp[BUFFER];
    int len;
    char order[BUFFER];
     struct sockaddr_in serv;
    int sock;


int main(int argc, char **argv) /* bien argv la mang chua cac doi so duoc truyen tu shell(terminal) & argc la so doi so.
Phan tu dau tien cua mang argv[0] la ten chuong trinh. vd: ./server 4444 => argc=2 & argv[0]= './server' & argv[1]='4444'. Tat ca deu duoc xem la dang chuoi */
{
   
    int res;
    pthread_t thread_1;

    //Tao socket client
       if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(-1);
    }
        
    serv.sin_family = AF_INET;
    serv.sin_port = htons(atoi(argv[2]));
    serv.sin_addr.s_addr = inet_addr(argv[1]);
    //bzero(&serv.sin_zero, 8);

    printf("\nThe TCPclient %d\n",ntohs(serv.sin_port));
        fflush(stdout);


    if((connect(sock, (struct sockaddr *)&serv, sizeof(struct sockaddr_in))) == -1)
    {
        perror("connect");
        exit(-1);
    }

  //Tao ra frame dau tien de xac dinh client
    strcpy(temp,"gateway ");
    write(sock, temp, BUFFER);


//Create Thread 1, Cap phep chay
        res = pthread_create(&thread_1, NULL, CLIENT_GW, 0);
    if (res != 0){
        perror("Thread creation failed");
        exit(EXIT_FAILURE);}
    
while(1) // Luong chinh trong chuong trinh: Lam nhiem vu luon doc data tu server

    {             
        len = read(sock, out, BUFFER);  //trinh khach nhan thong tin qua ham recv() hoac ham read()
        if(len)
        {
        out[len] = '\0';
        printf("message: %s\n", out);
        fflush(stdout);
         t = Get_data_socket_server(out);
          switch(t)
          {
            case GROUP_CONTROL:
                strcpy(temp, "ACK from GATEWAY ");
                write(sock, temp, BUFFER);
                break;
        
          }
      }
     
               
    }

}

//-------------------------------------------------------------------------------
//TUYEN PHU: lam nhiem vu gui request va truy van du lieu.
 void *CLIENT_GW(void *arg)
 {
    //dang nhap bang ten user
while(1)
{     
        sleep(2);
        printf("Input: ");
        fflush(stdin);
        fgets(in, BUFFER, stdin);
        write(sock, in, BUFFER);

     }
N
}
    
//----------------------------------------------------------------------------
// Cai dat ham doc du lieu truyen tu client app
int Get_data_socket_server( char data[BUFFER])
{   
    
    
    uint8_t x1 = sosanhchuoi("auto",data);
    uint8_t x2 = sosanhchuoi("handy",data);
    uint8_t x3 = sosanhchuoi("on",data);
    uint8_t x4 = sosanhchuoi("off",data);
               

    if (x1)
        { //strcpy(temp,"now is mode auto");
          t = GROUP_CONTROL;}//khong dung lenh gan '=' de gan chuoi vao bien temp.
    else if (x2)
        { //strcpy(temp,"now is mode handy");
           t = GROUP_CONTROL;}
    else if (x3)
        { //strcpy(temp,"the light is on");
          t = GROUP_CONTROL;}
    else if (x4)
        { //strcpy(temp,"the light is off");
          t = GROUP_CONTROL;}

    else t = GROUP_FAIL;
    return t;
}
