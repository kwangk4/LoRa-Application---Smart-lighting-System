
#ifndef INCLUDE_H_
#define INCLUDE_H_


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

#include <time.h>
#include <mysql.h>

//----------------------------
//MULTI THREAD
void *thread1_function(void *arg);
void *thread2_function(void *arg);
//sem_t bin_sem2;
pthread_mutex_t a_mutex;		
volatile int GloVar = 0;
char message[] = "Hello World";

//----------------------------
//SOCKET SERVER


#define ERROR    	  -1
#define MAX_CLIENTS    5
#define MAX_DATA    1024
    struct sockaddr_in server;
    struct sockaddr_in client;
    int sock;
    int  new,i;
    int new_gw[5];
    int new_app[5];
    int sockaddr_len = sizeof(struct sockaddr_in);
    int data_len;
    //char frame_st[MAX_DATA];
    char data_app[MAX_DATA];
    char data_gw[MAX_DATA];
    char temp_app[MAX_DATA];
    uint8_t gw = 0, app = 0;
    char temp_gw[MAX_DATA];
    


 //----------------------------
 //MYSQL DATABASE
MYSQL my_connection;
MYSQL_RES *res_ptr;
MYSQL_ROW sqlrow;
 
 typedef enum 
{ 
    ALL,
    ROW_10
}READ_DATA;

READ_DATA r;
//----------------------------
//GET_DATA_SOCKET
char s3[255], s4[255];
typedef enum 
{   
    GROUP_NAK,
    GROUP_ACK,
	GROUP_DATA,
	GROUP_CONTROL,
	GROUP_QUERY,
    GROUP_FAIL
}GROUP_ORDER;

GROUP_ORDER t_app, t_gw, t;
 #endif

//-------------------------
char FrameGateway_Checking[] = "gateway";
char FrameClient_Checking[] = "client";
