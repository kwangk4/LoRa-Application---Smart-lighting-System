#include "lib_server.h"
// Khai bao ham
void *clientgw();
void *clientapp();
void Displayrowclient(); //Ham hien thi 
void Get_realtime();//Ham doc thoi gian thuc
void DATA_ACCESS();
void DELETE_DATA();
int Get_data_socket_app(char data[MAX_DATA]);
int Get_data_socket_gateway(char data[MAX_DATA]);
int DATA_STORE();
int sosanhchuoi(char *s1, char *s2);

struct ThreadArgs
{
    int clntSock;                      /* Socket descriptor for client */
};



int main(int argc, char **argv)
{
    uint8_t m = 0;
    pthread_t threadID_1,threadID_2;              /* Thread ID from pthread_create() */
    struct ThreadArgs *threadArgs;
/*
    sem_init(&bin_sem1, 0, 0);//---------------------------------------------------------------------------------------2: initialize semaphore
    sem_post(&bin_sem1); // Post 1 first
    sem_init(&bin_sem2, 0, 0);
*/
    if( pthread_mutex_init(&a_mutex, NULL) !=0 )
    {
        perror( "Mutex create error" );
        exit(-1);
    }

      
    
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == ERROR) //gia tri sock duoc tra ve boi ham socket 
    {
        perror("server socket: ");
        exit(-1);
    }
     
    // Construct local address structure
    memset(&server, 0, sizeof(server));       /* Zero out structure */
    server.sin_family = AF_INET;                /* Internet address family */
    server.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    server.sin_port = htons(atoi(argv[1]));              /* Local port */

    // Bind to the local address
    if (bind(sock, (struct sockaddr *) &server, sizeof(server)) == ERROR)
    {
        perror("bind : ");
        exit(-1);
    }
    // Mark the socket so it will listen for incoming connections 
    if (listen(sock,  MAX_CLIENTS) == ERROR)
    {
        perror("listen");
        exit(-1);
    }



printf("\nThe TCPServer Waiting for client on port %d\n",ntohs(server.sin_port));  //ntohs dung de nhan du lieu cung theo trat tu gui di boi trinh chu
        fflush(stdout);
    
    while(1)
    {
        
        // gateway.client = 0,1: used to checking--- gateway.client = 2: used for handling
        uint8_t gateway = 1, client = 1;

        // Wait for a client to connect
        if( (new = accept(sock, (struct sockaddr*) &server, &sockaddr_len)) == ERROR)
            perror("accept() failed !");
        else
        {
            getpeername(new, (struct sockaddr *) &server, &sockaddr_len);
            printf("================NEW CONNECTION==================\n");
            printf("+ New client[%d][Addr:%s]\n", new-3, inet_ntoa(server.sin_addr));
            while(1)
            {
                if(recv(new,data_app,MAX_DATA,0) > 0)  //Cho frame dau tien
                {
                printf("Check 1st frame: Gateway or Client?\n");
                // So sanh do dai chuoi voi frame gateway. Neu bang nhau thi xet
                if (  (strlen(data_app)-1) == (strlen(FrameGateway_Checking))  ) 
                    {
                        for (i = 0; i < (strlen(data_app)-1); ++i)
                        {   
                            if (data_app[i] != FrameGateway_Checking[i])
                            {
                            gateway = 0;
                            break;
                            }

                        }
                        if (gateway == 1) 
                        {
                        //Neu co ket noi thi tao thread ung voi tinh chat cua client (Gateway)
                   
                            /* Create separate memory for client argument */
                            if ((threadArgs = (struct ThreadArgs *) malloc(sizeof(struct ThreadArgs))) == NULL)
                            perror("malloc() failed");

                            threadArgs -> clntSock = new;

                            if (pthread_create(&threadID_1, NULL, &clientgw, (void *) threadArgs) != 0)
                            perror("pthread_create() failed");
                            break;
                        }
                    }
                
                // So sanh do dai chuoi voi frame client. Neu bang nhau thi xet. Luu y xet lan luot
                if (  (strlen(data_app)-1) == (strlen(FrameClient_Checking))  ) 
                    {
                        for (i = 0; i < (strlen(data_app)-1); ++i)
                        {
                            if (data_app[i] != FrameClient_Checking[i])
                            {
                            client = 0;
                            break;
                            }

                        }
                
                        if (client == 1) 
                        {
                        //Neu co ket noi thi tao thread ung voi tinh chat cua client (APP)                    
                    
                            /* Create separate memory for client argument */
                            if ((threadArgs = (struct ThreadArgs *) malloc(sizeof(struct ThreadArgs))) == NULL)
                            perror("malloc() failed");

                            threadArgs -> clntSock = new;

                            if (pthread_create(&threadID_2, NULL, &clientapp, (void *) threadArgs) != 0)
                            perror("pthread_create() failed");
                            break;
                        }
                    }

                printf("-----------Please Input again!---------\n");                  
                                 
                }
            }
        }
    }
    close(sock);
    return 0; 
}

//--------------------------------------------------------------------
//Tuyen doc thong tin tu client app
void *clientapp(void *threadArgs)
{
    app++;
    printf("Okay, Welcome you, App[%d]!\n",app);
    new_app[app] = ((struct ThreadArgs *) threadArgs) -> clntSock;
    int recvMsgSize;
    while(1)
    { 
        recvMsgSize = recv(new_app[app],data_app,MAX_DATA,0);
        pthread_mutex_lock(&a_mutex);
        if (recvMsgSize < 0) 
            perror("ERROR reading from socket");
        else if(recvMsgSize>0)
        {
             printf("-----------------------app-running--------------------\n");
            //printf(".\n Client[%d]: %s",newsock-3,data_app);
             fflush(stdin);
            t = Get_data_socket_app(data_app);
            
            switch(t)
            {
                       
            case GROUP_CONTROL:
                write(new_gw[gw], data_app, MAX_DATA);
                printf("sent mesg to GATEWAY: %s\n",data_app);
               break;

            case GROUP_QUERY:
                DATA_ACCESS(new_app[app]);
                break;

            default:
                strcpy(temp_app,"SEND FAIL!");
                printf("sent mesg to client: %s\n",temp_app);
                write(new_app[app], temp_app, MAX_DATA);
                break;
            }

        }
        else
        {
            printf("- Client[%d]: disconnected !\n",new_app[app]-3);
            break;
        } 
       pthread_mutex_unlock(&a_mutex);   
    }
    close(new_app[app]);
}
    //------------------------------gateway-
void *clientgw(void *threadArgs)
{
    gw++;
    printf("Okay, Welcome you, Gateway[%d]!\n",gw);
    new_gw[gw] = ((struct ThreadArgs *) threadArgs) -> clntSock;
    int recvMsgSize;
    uint8_t flag = 1;
    while(1)
    {
        recvMsgSize = recv(new_gw[gw],data_gw,MAX_DATA,0);
        pthread_mutex_lock(&a_mutex);
        if (recvMsgSize < 0) 
            perror("ERROR reading from socket");
        else if(recvMsgSize>0)
        {
             printf("-----------------------gw-running--------------------\n");
            //printf(".\n Client[%d]: %s\n",new_gw[gw]-3,data_gw);
             fflush(stdin);
            t = Get_data_socket_gateway(data_gw);
            
            switch(t)
            {
            case GROUP_ACK:
                write(new_app[app], temp_app, MAX_DATA);//Thacmac!
                printf("sent mesg to APP: %s\n", temp_app);
                break;
            case GROUP_NAK:
                strcpy(temp_gw,"REQUEST FAIL!");
                printf("sent mesg to APP: %s\n",temp_gw);
                break;
            case GROUP_DATA:
                flag = DATA_STORE(flag);
                sleep(1);
                if(flag)
                {
                    strcpy(data_gw,"insert success\n");
                    write(new_gw[gw], data_gw, MAX_DATA);
                    printf("Sent mesg to GATEWAY: %s\n", data_gw);
                }
                else
                {
                    strcpy(data_gw,"insert failed\n");
                    write(new_gw[gw], data_gw, MAX_DATA);
                    printf("Sent mesg to GATEWAY: %s\n", data_gw);
                }
                break;
            default:
                strcpy(temp_gw,"SEND FAIL!");
                printf("sent mesg to GATEWAY: %s\n",temp_gw);
                write(new_gw[gw], temp_gw, MAX_DATA);
                break;
            }
        
        }
        else
        {
            printf("- Gateway[%d]: disconnected !\n",new_gw[gw]-3);
            break;
        }  
        pthread_mutex_unlock(&a_mutex);    
    }
    close(new_gw[gw]);
}
//---------------------------------------------------------------------------------------------
//Cai dat ham luu tru du lieu len database
int DATA_STORE(   )
{
        //khoi tao cau truc
   static int sum_id = 0;
    int res;
    mysql_init(&my_connection);
    uint8_t flag;
    if( mysql_real_connect( &my_connection, "localhost", "Tai", "tai123", "DO_AN", 0, NULL, 0) )
        {

        printf("Database Connected success\n");
            

        // luu du lieu vao bang
    
        char s1[255] = "insert into CO_SO_DU_LIEU(BRIGHT,ON_OFF) values(";
        //char s2[255] = "'2018-03-20 15:15:20',";
        
        //strcat(s1,s2);
        strcat(s1,s3);
        strcat(s1,s4);
        
        res = mysql_query( &my_connection, s1);
        if(res)
            { printf("insert error %d:%s\n",mysql_errno(&my_connection),mysql_error(&my_connection));
            flag = 0; }
        else
            { 
              flag = 1;
              if( sum_id < 100 )
                 sum_id ++;
              else
                 DELETE_DATA(); 
                mysql_close(&my_connection);
            }  

        }   
    else
        {   
            flag = 0;
            printf("Connection failed\n");
            if(mysql_errno(&my_connection))
            {
                printf("connection error %d:%s\n",mysql_errno(&my_connection),mysql_error(&my_connection));
            }   
        }   
    return flag;
}

//------------------------------------------------------------------------------------------------
//Cai dat ham truy van du lieu tu database
void DATA_ACCESS( int sock_cli)
 {
        //khoi tao cau truc
    int res;
    mysql_init(&my_connection);
    if( mysql_real_connect( &my_connection, "localhost", "Tai", "tai123", "DO_AN", 0, NULL, 0) )
    {
        printf("Connect database success\n");
        //xuat du lieu tu database
        if( r == ALL)
            res = mysql_query( &my_connection, "select ID, TIME, BRIGHT, ON_OFF from CO_SO_DU_LIEU ");
        else if( r == ROW_10)   
            res = mysql_query( &my_connection, "select ID, TIME, BRIGHT, ON_OFF from CO_SO_DU_LIEU ORDER BY ID DESC LIMIT 0, 10");
        if( res )
            printf("SELECT ERROR:%s\n",mysql_error(&my_connection) );
        else
        {
            res_ptr = mysql_use_result(&my_connection);
            if( res_ptr )
            {
                while((sqlrow = mysql_fetch_row(res_ptr)))
                    {   
                        printf("get data... \n");
                        Displayrowclient(sock_cli);
                    }
            
            if (mysql_errno(&my_connection))
            
                printf("retrive error: %s\n",mysql_error(&my_connection) );
            
            }
        }

        //giai phong tai nguyen
        mysql_free_result(res_ptr);
        mysql_close(&my_connection);
    }
    else
        {   
            printf("Connection failed\n");
            if(mysql_errno(&my_connection))
            {
                printf("connection error %d:%s\n",mysql_errno(&my_connection),mysql_error(&my_connection));
            }   
        }
        mysql_close( &my_connection );
}



//------------------------------------------------------------------------------------
//cai dat ham hien thi tren client

void Displayrowclient(int sock_cli)
{
    unsigned int field_count;
    field_count = 0;
    while(field_count < mysql_field_count(&my_connection))
    {
        switch(field_count)
        {
          case 0:printf("ID: %s\n",sqlrow[field_count]);
            strcpy(s3, "ID: "); 
            strcat(s3,sqlrow[field_count]);
            write(sock_cli, s3, MAX_DATA);
            field_count++;
            break;
          case 1:printf("TIME: %s\n",sqlrow[field_count]);
            strcpy(s3, "TIME: "); 
            strcat(s3,sqlrow[field_count]);
            write(sock_cli, s3, MAX_DATA);
            field_count++;
            break;
          case 2:printf("BRIGHT: %s\n",sqlrow[field_count]);
            strcpy(s3, "BRIGHT: "); 
            strcat(s3,sqlrow[field_count]);
            write(sock_cli, s3, MAX_DATA);
            field_count++;
            break;
          default:printf("MODE: %s\n",sqlrow[field_count]);
            strcpy(s3, "MODE: "); 
            strcat(s3,sqlrow[field_count]);
            write(sock_cli, s3, MAX_DATA);
            write(sock_cli, "------\n", MAX_DATA);
            field_count++;
        }
        
    } 
}
//------------------------------------------------------------------------------------------
//cai dat ham doc thoi gian thuc
void Get_realtime()
{
    struct tm *tm_ptr;
    time_t the_time;
    (void) time(&the_time);
    tm_ptr = gmtime(&the_time);
    printf("datetime: 20%02d-%02d-%02d %02d:%02d:%02d\n", tm_ptr->tm_year-100, tm_ptr->tm_mon+1, 
    tm_ptr->tm_mday, tm_ptr->tm_hour+7, tm_ptr->tm_min, tm_ptr->tm_sec);
}
//-----------------------------------------------------------------------------------------------
//Xoa du lieu trong database
void DELETE_DATA()
{
    printf("delete success\n");
    int res;
    res = mysql_query( &my_connection, "TRUNCATE CO_SO_DU_LIEU");
    if(res)
            { printf("delete error %d:%s\n",mysql_errno(&my_connection),mysql_error(&my_connection));}
       
}

//-----------------------------------------------------------------------------------------------
// Cai dat ham doc du lieu truyen tu client app
int Get_data_socket_app( char data[MAX_DATA])
{   
    
    
    uint8_t x1 = sosanhchuoi("auto",data);
    uint8_t x2 = sosanhchuoi("handy",data);
    uint8_t x3 = sosanhchuoi("on",data);
    uint8_t x4 = sosanhchuoi("off",data);
    uint8_t x5 = sosanhchuoi("read_data",data);
    uint8_t x6 = sosanhchuoi("read_all_data",data);
            

    if (x1)
        { strcpy(temp_app,"now is mode auto");
          t_app = GROUP_CONTROL;}//khong dung lenh gan '=' de gan chuoi vao bien temp.
    else if (x2)
        { strcpy(temp_app,"now is mode handy");
           t_app = GROUP_CONTROL;}
    else if (x3)
        { strcpy(temp_app,"the light is on");
          t_app = GROUP_CONTROL;}
    else if (x4)
        { strcpy(temp_app,"the light is off");
          t_app = GROUP_CONTROL;}
    else if (x5)
        {  r = ROW_10;
          t_app = GROUP_QUERY;}
    else if (x6)
        {  r = ALL;
          t_app = GROUP_QUERY;}
    else t_app = GROUP_FAIL;
    return t_app;
}
//-----------------------------------------------------------------------------------------------
// Cai dat ham doc du lieu truyen tu client gateway
int Get_data_socket_gateway( char data[MAX_DATA])
{   
    
    uint8_t x1 = sosanhchuoi("dark_on",data);
    uint8_t x2 = sosanhchuoi("dark_off",data);
    uint8_t x3 = sosanhchuoi("light_on",data);
    uint8_t x4 = sosanhchuoi("light_off",data);
    uint8_t x5 = sosanhchuoi("ACK from GATEWAY",data);
    //uint8_t x6 = sosanhchuoi("NAK",data);
            
          
     if (x1)
        { strcpy(s3, " 'dark', ");
          strcpy(s4, " 'on') ");  
         t_gw = GROUP_DATA;}
    else if (x2)
        { strcpy(s3, " 'dark', ");
          strcpy(s4, " 'off') ");
           t_gw = GROUP_DATA; }
    else if (x3)
        { strcpy(s3, " 'light', ");
          strcpy(s4, " 'on') ");
            t_gw = GROUP_DATA;}
    else if (x4)
        { strcpy(s3, "'light',");
          strcpy(s4, " 'off') ");
          t_gw = GROUP_DATA; }
    else if (x5)
        { 
          t_gw = GROUP_ACK;}//khong dung lenh gan '=' de gan chuoi vao bien temp.
    else
        t_gw = GROUP_FAIL;
    return t_gw;
}


//----------------------------------------------------------
//chuong trinh so sanh chuoi tu ban phim
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

//-------------------------------------------------------------------------------------------------------------------
