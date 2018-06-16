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
        void *CLIENT_APP(void *arg);
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
    typedef enum{AUTO, HANDY}mode;
    volatile mode flag = HANDY;


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

    // Tao ra frame dau tien de nhan dang client
    strcpy(temp,"client ");
    write(sock, temp, BUFFER);

//Create Thread 1, Cap phep chay
        res = pthread_create(&thread_1, NULL, &CLIENT_APP, 0);
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

          if ( sosanhchuoi(out,"now is mode auto ") == 1 )
             flag = AUTO;                  
          else if ( sosanhchuoi(out,"now is mode handy ") == 1 )
             flag = HANDY; 
        }

    } 
}

//-------------------------------------------------------------------------------
//TUYEN PHU: lam nhiem vu gui request va truy van du lieu.
 void *CLIENT_APP(void *arg)
 {
    //dang nhap bang ten user
while(1)
{     
        printf("\nusername: ");
        fflush(stdin);  //de xoa bo nho dem tranh viec enter cua chuoi s1 bi chen vao chuoi s2 ben duoi.
        fgets(order,BUFFER,stdin);
        while ( sosanhchuoi("Tai" , order) == 0 ) 
        {
            printf("\nusername: ");
            fflush(stdin); // Dung de xoa bo dem ky tu ko mong muon khi nhap tu ban phim
            fgets(order,BUFFER,stdin);
            
        }

    while(1)
    {   
      AGAIN:
        sleep(1);  // Dung cho 1s de cho trinh chinh doc va the hien data
        printf("\nMODE:AUTO OR HANDY ( / ) : ");
         fflush(stdin);
        fgets(in, BUFFER, stdin); //gan thong tin nhap tu man hinh terminal vao mang i
       
        if((sosanhchuoi("back", in) == 1)||sosanhchuoi("turnoff", in) == 1)
            break;
        else if((sosanhchuoi("on", in) == 1)||sosanhchuoi("off", in) == 1)
             {
                printf("MUST HANDY!");
                goto AGAIN;
             }
        else if(sosanhchuoi("handy", in) == 1)
        //thong tin duoc xu li o trinh chu(server) sau do gui lai cho trinh khach(client).
             {
                 write(sock, in, BUFFER); //gui thong tin tu mang i thong qua lenh truyen tu ham send() hoac ham write()
                 
              while(1)
                    { 
                      sleep(2);// Dung cho 1s de cho trinh chinh doc va the hien data
                      printf("\nlight ON/OFF: ");
                      fflush(stdin);
                    fgets(temp, BUFFER, stdin);
                    if(flag == AUTO)
                      break;
                    else if( flag == HANDY)
                    {
                      if ( (sosanhchuoi("back", temp) == 1)||(sosanhchuoi("turnoff", temp) == 1) )
                        break;
                      else  write(sock, temp, BUFFER);
                    }
                    if (sosanhchuoi("turnoff", temp) == 1)
                      break;
                    }
                    flag = HANDY;
             }
        else
            write(sock, in, BUFFER); //gui thong tin tu mang i thong qua lenh truyen tu ham send() hoac ham write()

     }
  
}
    
}
