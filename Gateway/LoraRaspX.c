
//#define Testdoxa
/* Standard library */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <strings.h>
#define LOW 0
#define HIGH 1
#define MAXBUF 256

/* Gpio Raspberry */

#include <wiringPi.h>
#include <wiringSerial.h>
#define AUXpin		18
#define M0pin		24
#define M1pin		23
#define TIMEOUT_RET	2


#define HEADER          0xC0
#define GW_AddressH		0x05
#define GW_AddressL 	0x01
#define SPEED           0x18
#define GW_Channel		0x17
#define OPTION          0xC7

uint8_t NODE_Address_H	= 0;
uint8_t NODE_Address_L	= 0;


int fd;
int ReadAUX();
int WaitAUX_H();

/* Thread */
#include <pthread.h>	
#include <semaphore.h>
#define BUFFER 	1024
void *Interface_GatewayServer(void *arg);
void *Interface_LoraGateway(void *arg);
uint8_t valueSensor = 0;
uint8_t LedState = 0;
void *DataSending(void *arg);
	char in[BUFFER];
	char out[BUFFER];
	char message[1000] , server_reply[2000];
void *DataReceiving(void *arg);
	int len;
//sem_t bin_sem1;
//sem_t bin_sem2;	


/*Networking Library*/
#include "sys/socket.h"
#include "sys/types.h"
#include "netinet/in.h"
#include "error.h"
#include "strings.h"
#include "unistd.h"
#include "arpa/inet.h"
#include "ctype.h"
int sock;
struct sockaddr_in serv;
#define FRAMEgateway	"gateway\n"	
uint8_t SENSORDATA = 0;	//default. do not send data first!


/*==============================================================*/
int main(int argc, char **argv){

	/*GPIO init*/
	wiringPiSetupGpio();
	pinMode(M0pin, OUTPUT);	delay(500);
	pinMode(M1pin, OUTPUT);	delay(500);
	pinMode(AUXpin,INPUT);	delay(500);


	/*serial init*/
	if((fd = serialOpen ("/dev/ttyAMA0", 9600)) < 0 ){
		fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
	};

	/*Setup Lora E32 Module - Send command through UART*/
	digitalWrite(M0pin,HIGH);	//MODE 3 - SLEEP
	digitalWrite(M1pin,HIGH);
	printf("Mode 3 - Sleep\n");
	delay(10);
	if(WaitAUX_H() == TIMEOUT_RET) {exit(0);}	//if timeout
	

	/*Write CMD for LORA configuration*/
	//Header | Address High | Address Low | SPEED | Channel | Option
	uint8_t CMD[6] = {HEADER, GW_AddressH, GW_AddressL, SPEED, GW_Channel, OPTION};
	write(fd, CMD, 6);

	WaitAUX_H();
	delay(1200);
	printf("Setting Configure has been sent!\n");

	/*Read Set configure*/
	while(serialDataAvail(fd)){	/*Clean Uart Buffer*/
		serialGetchar(fd);
	}
	uint8_t READCMD[3] = {0xC1, 0xC1, 0xC1};	/*Send 3 C1 to read*/
	write(fd, READCMD, 3);
	WaitAUX_H();
	delay(50);
	printf("Reading Configure Command has been sent!\n");

	uint8_t readbuf[6];
	uint8_t Readcount, idx, buff;
	Readcount = serialDataAvail(fd);
	if (Readcount == 6){
		printf("Setting Configure is:  ");
		for(idx = 0; idx < 6; idx++){
			readbuf[idx] = serialGetchar(fd);
			printf("%x ",0xFF & readbuf[idx]);
			delay(10);
		}
	}
	printf("\n");
	printf("---------------SETTING DONE-------------\n");


	/*Thread for lora - rasp*/
		//Create Thread for lora interface with raspberry
		int res;
		pthread_t  LoraRas;
		res = pthread_create(&LoraRas, NULL, Interface_LoraGateway, 0);
			if (res != 0){
			perror("Thread creation failed");
			exit(EXIT_FAILURE);
		}
		sleep(1);



	/* Initialise Socket */
		

	/*Socket Client Configure*/
		
	// [sock]: global var
	
	
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket");
		exit(-1);
	}

	serv.sin_family = AF_INET;
	serv.sin_port = htons(atoi(argv[2]));
	serv.sin_addr.s_addr = inet_addr(argv[1]);

	printf("Gateway IP: %d \n",ntohs(serv.sin_port));
	fflush(stdout);


	/*Connect to server*/
	if((connect(sock, (struct sockaddr *)&serv, sizeof(struct sockaddr_in))) == -1){
		perror("connect error!\n");
		exit(-1);
	}


	/*Thread creation for Data Sending to server task*/
	pthread_t DataSend_task;
	res = pthread_create(&DataSend_task, NULL, DataSending, 0);
	if (res != 0) {
		perror("Data Sending Threading creation is failed!\n");
		exit(EXIT_FAILURE);
	}


	/*Thread creation for Receiving Data from server Task*/
	pthread_t DataReceive_task;
	res = pthread_create(&DataReceive_task, NULL, DataReceiving, 0);
	if (res != 0) {
		perror("Data Sending Threading creation is failed!\n");
		exit(EXIT_FAILURE);
	}


	/*Loop*/
	while(1){
		//[len] global var
		len = recv(sock,server_reply, 1024, 0);
		if(len > 0){
			printf("Message from server: %s\n", server_reply);
			fflush(stdout);

		if(sosanhchuoi("handy", server_reply)){

			uint8_t Senda[4] = {NODE_Address_H, NODE_Address_L, 0x17, 0xAA};
			write(fd,Senda,4);
			printf("HANDY Mode sent to NODE\n");
			write(sock,"ACK from GATEWAY\n",BUFFER);
			delay(10);
		}
		if (sosanhchuoi("auto", server_reply)){
			uint8_t Senda[4] = {NODE_Address_H, NODE_Address_L, 0x17, 0xFF};
			write(fd,Senda,4);
			printf("AUTO Mode sent to NODE\n");
			write(sock,"ACK from GATEWAY\n",BUFFER);
			delay(10);
		}
		if (sosanhchuoi("on", server_reply)){
			uint8_t Senda[4] = {NODE_Address_H, NODE_Address_L, 0x17, 0x11};	//light on frame
			write(fd,Senda,4);
			printf("Light ON sent to NODE\n");
			write(sock,"ACK from GATEWAY\n",BUFFER);
			delay(10);
		}
		if (sosanhchuoi("off", server_reply)){
			uint8_t Senda[4] = {NODE_Address_H, NODE_Address_L, 0x17, 0x22};
			write(fd,Senda,4);
			printf("Light OFF sent to NODE \n");
			write(sock,"ACK from GATEWAY\n",BUFFER);
			delay(10);
		}

		}
	}

	close(sock);
	return(0);
}


/*Function GPIO: Check whether AUX high or not? */
int ReadAUX(){
	int AUX_HL;
	if(!digitalRead(AUXpin)){
		AUX_HL = LOW;
	}
	else{
		AUX_HL = HIGH;
	}
	return(AUX_HL);
}

/*Function GPIO: Wait until AUX high, initialise TIMEOUT*/
int WaitAUX_H(){
	int ret_var;
	uint8_t cnt = 0;

	while ((ReadAUX() == LOW) && (cnt++ < 100)){
		printf(".");
		delay(100);
	}
	printf("\n");	

	if (cnt >= 100 ) {
		ret_var = TIMEOUT_RET;
		printf("Timeout!\n");
	}
	
	return ret_var;
}


/*Thread GatewayServer: (root) Thread for Gateway interfaces with server*/
void *Interface_GatewayServer(void *arg){
	
}

/*Thread GatewayServer: (children) Sending data to server TASK*/
void *DataSending(void *arg){
		printf("---------Send GATEWAY FRAME for server to recognize first-------\n");
		strcpy(in, FRAMEgateway);
        printf("Send to Server: %s\n",in );
        write(sock, in, BUFFER);


	while(1){
		if (SENSORDATA){ 	   
			SENSORDATA = 0;
			printf("Sending Sensor Data to server:\n");
			if (valueSensor == 0 && LedState == 0)  write(sock,"dark_off\n",BUFFER);
			if (valueSensor == 0 && LedState == 1)  write(sock,"dark_on\n",BUFFER);
			if (valueSensor == 1 && LedState == 0)  write(sock,"light_off\n",BUFFER);
			if (valueSensor == 1 && LedState == 1)  write(sock,"light_on\n",BUFFER);
			//uint8_t Sendit[4] = {valueSensor, LedState};
			//write(sock,Sendit,BUFFER);
          	//in[4] = {NODE_Address_H, NODE_Address_L, valueSensor, LedState};
        	//write(sock, in, BUFFER);
	}
		//if (){

		//}

}
}
/*Thread GatewayServer: (children) Receiving data from server TASK*/
void *DataReceiving(void *arg){
	//Loop in main do this
}

/*Thread LoraGateway (root)*/
void *Interface_LoraGateway(void *arg){
	
	
	uint8_t Pack_Count = 0;
	float PRR;
	//Receive Data From Lora Node
	
	//Setting Mode first
	digitalWrite(M0pin,LOW);	//MODE 0 - NORMAL
	digitalWrite(M1pin,LOW);
	printf("Mode 0 - NORMAL\n");
	delay(10);
	if(WaitAUX_H() == TIMEOUT_RET) {exit(0);}	//if timeout
	int SequenceNum_Received = 0;

	//Receive task
	while(1){
		uint8_t data_buf[MAXBUF], data_len;
		uint8_t count;

		
		
		data_len = serialDataAvail(fd);

		#ifdef Testdoxa
		if (data_len > 0){	//*Already Get data - Sequence number = data_buf[0]
			for (count = 0; count < data_len; count++)		{
			data_buf[count] = serialGetchar(fd);
			}
		printf("Receive %d bytes from Lora NodeID: %d%d\n",data_len, data_buf[0], data_buf[1]);
		NODE_Address_H = data_buf[0];
		NODE_Address_L = data_buf[1];
		printf("Value: %x      ",0xFF & data_buf[2] );
		printf("LED state: %x      ",0xFF & data_buf[3] );
		printf("Sequence Number (PACKET) %d      ",data_buf[4]);
		SequenceNum_Received++;
		printf("Received --%d-- Packet\n",SequenceNum_Received);
		if(data_buf[4] == 100){	//Sequence Number Packet = 100
			float PRR = (float)SequenceNum_Received / (float)data_buf[4];
			printf("\n");
			printf("-------------PRR-------------\n");
			printf("PRR = %lf\n", PRR);
			printf("-----------------------------\n");
		}


		WaitAUX_H();
		delay(10);
		WaitAUX_H();

		//Send to Node ACK
		uint8_t Senda[4] = {NODE_Address_H, NODE_Address_L, 0x17, data_buf[4]};
		write(fd,Senda,4);
		printf("ACK has been sent!, Sq_num = %d\n",data_buf[4]);
		delay(10);

		if (data_buf[4] == 100){
			exit(0);
		}
		}



		#else //system
		if (data_len > 0){	//Already Get data - Sequence number = data_buf[0]
			for (count = 0; count < data_len; count++)		{
			data_buf[count] = serialGetchar(fd);
			}
		printf("Receive %d bytes from Lora NodeID: %d%d\n",data_len, data_buf[0], data_buf[1]);
		NODE_Address_H = data_buf[0];
		NODE_Address_L = data_buf[1];
		printf("Value: %x      ",0xFF & data_buf[2] );
		valueSensor = data_buf[2];
		printf("LED state: %x      ",0xFF & data_buf[3] );
		LedState = data_buf[3];

		SENSORDATA = 1;
		
	}
		//need to send Mode
		//Some code
	#endif
}
}

int sosanhchuoi(char *s1,char *s2)
{     
  int n, m;
  n=strlen(s1);m=strlen(s2);  

  
    while (n==(m-1))    
      { 

            
            while (*s1++ == *s2++)
                             
              {
                if(*s1 == 0)
                return 1;
                else
                continue;
              }
                             
          
          return 0;
        
        
        }
        return 0;
}