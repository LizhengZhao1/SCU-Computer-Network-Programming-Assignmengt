// Author: Lizheng Zhao
// Student ID: W1608646
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// socket headers
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
// structure for address info
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
// header for ACK timer
#include <unistd.h>

// error method to display errors during connection
void error(char *msg){
	perror(msg);
	exit(0);
}

int main(){
	int sock, sendMsg, length, receiveMsg;
	struct sockaddr_in server, from;
	struct hostent *hp;

	//create socket
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	fcntl(sock, F_SETFL, O_NONBLOCK); // make the socket non-blocking
	if (sock<0)
	{
		error("Error: socket");
	}

	// specify an address for the socket
	server.sin_family = AF_INET; //Internet, IPv4
	server.sin_port = htons(59003); //port number
	server.sin_addr.s_addr = INADDR_ANY; //connect local machine

	// configure packet
	unsigned char buffer[14];
	int seg_num, tech_num;
	// start of packet id
	buffer[0] = 0xFF; 
	buffer[1] = 0xFF; 
	// end of packet id
	buffer[12] = 0xFF; 
	buffer[13] = 0xFF; 
	// client id set to 100
	buffer[2] = 100; 
	// access permission
	buffer[3] = 0xFF; 
	buffer[4] = 0xF8; 
	// segment number
	buffer[5] = 1;
	// ask user to input user subscriber number
	unsigned long usrNum;
	printf("Enter subscriber number: ");
	scanf("%lu", &usrNum); 
  	// shift a long and store them in four bytes
	buffer [8] = usrNum >> 24; 
	buffer [9] = usrNum >> 16; 
	buffer [10] = usrNum >> 8; 
	buffer [11] = usrNum >> 0; 
	//technology
	printf("Choose your subscribed technology: \n");
	printf("2G.........02\n");
	printf("3G.........03\n");
	printf("4G.........04\n");
	printf("5G.........05\n");
	printf("Enter technology: ");
	scanf("%d", &tech_num); 
	buffer[7] = tech_num;
	// length
	// create a new string to check payload length
	char payload[5];
	for(int i = 0; i < 5; i++)
	{
		payload[i] = buffer[i + 7];
	}
	buffer[6] = strlen(payload); 
	
	// send
	length = sizeof(server);
	sendMsg=sendto(sock, buffer, 14, 0, (struct sockaddr *)&server,length);
	if (sendMsg<0)
	{
		error("Error: sendto");
	}
	usleep(3000);
	
	// receive 
	receiveMsg = -1; // to hold return value of recvfrom
	int counter = 0; // declare ACK counter
	while(receiveMsg == -1 && counter < 4)
	{
		// send again if didn't receive, won't execute at first iteration
		if(counter > 0 && counter < 3 && receiveMsg == -1)
		{	
			sleep(3); // ACK_timer 3 seconds
			if(receiveMsg == -1)
			{
				printf("ACK not received, packet resent\n");
				sendMsg=sendto(sock, buffer, 14, 0, (struct sockaddr *)&server,length);
			}
		}
		// last check for receive packet
		if(counter == 3 && receiveMsg == -1)
		{	
			sleep(3);// ACK_timer 3 seconds
			if(receiveMsg == -1)
			{
				printf("Server does not respond\n"); // print error if not receive after three counter resets
				return 0;
			}
		}
		// receive packet
		receiveMsg = recvfrom(sock, buffer, 14, 0, (struct sockaddr *)&from, (socklen_t *)&length); // receive datagram
		if (receiveMsg != -1)
		{
			printf("Sent from server: ");
			if((int)buffer[4] == 0xFB)
			{
				printf("Access granted\n");
			}
			else if((int)buffer[4] == 0xF9)
			{
				printf("Access denied: not paid\n");
			}
			else if((int)buffer[4] == 0xFA)
			{
				printf("Access denied: user does not exist\n");
			}
			else
			{
				printf("Access denied: unknown error, contact administrator\n");
			}	
			return 0; // exit when received
		}
		counter++; //counter increment
	}
	printf("Server does not respond"); // print error if not receive after three counter resets
	return 0;
}
