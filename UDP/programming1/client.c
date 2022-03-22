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
	unsigned char return_buffer[10];

	//create socket
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	fcntl(sock, F_SETFL, O_NONBLOCK); // make the socket non-blocking
	if (sock<0)
	{
		error("Error: socket");
	}

	// specify an address for the socket
	server.sin_family = AF_INET; //IPv4, Internet
	server.sin_port = htons(59002); //port number
	server.sin_addr.s_addr = INADDR_ANY; //connect local machine

	// configure packet
	unsigned char buffer[264];
	int seg_num;
	// start of packet id
	buffer[0] = 0xFF; 
	buffer[1] = 0xFF; 
	// end of packet id
 	int endID;
 	printf("Enter end of packet identifier: ");
 	scanf("%X", &endID); 
 	buffer[262] = 0xFF;
	buffer[263] = endID;
	// client id set to 100
	buffer[2] = 100; 
	// packet type: DATA
	buffer[3] = 0xFF; 
	buffer[4] = 0xF1; 
	// segment number
	printf("Enter segment number: ");
	scanf("%d", &seg_num); 
	buffer[5] = seg_num;
	// payload
	unsigned char payload[255];
	printf("Please enter the message: ");
	scanf("%s", payload);
	// load payload to buffer
	for (int i = 0; i < 255; i++)
	{
		buffer[i + 7] = payload [i];
	}
	// payload length
	int payload_len;
	printf("Enter payload length: ");
	scanf("%d", &payload_len); 
	buffer[6] = payload_len; 

	// send
	length = sizeof(server);
	sendMsg=sendto(sock, buffer, 264, 0, (struct sockaddr *)&server,length);
	if (sendMsg<0)
	{
		error("Error: sendto");
	}
	usleep(3000);
	
	//receive
	receiveMsg = -1; // hold return value of recvfrom
	int counter = 0; //ACK counter
	while(receiveMsg == -1 && counter < 4)
	{	
		// send again if didn't receive, won't execute at first iteration
		if(counter > 0 && counter < 3 && receiveMsg == -1)
		{	
			sleep(3); // ACK_timer 3 seconds
			if(receiveMsg == -1)
			{
				printf("ACK not received, packet resent\n");
				sendMsg=sendto(sock, buffer, 264, 0, (struct sockaddr *)&server,length);
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
		receiveMsg = recvfrom(sock, return_buffer, 10, 0, (struct sockaddr *)&from, (socklen_t *)&length);
		// proceed if return packet received
		if (receiveMsg != -1)
		{
			printf("Sent from server: ");
			//determine return packet
			if ((int)return_buffer[4] == 0xF2)
			{
				printf("ACK\n");
			}
			else if((int)return_buffer[6] == 0xF4)
			{
				printf("REJECT: Out of sequence\n");
			}
			else if((int)return_buffer[6] == 0xF5)
			{
				printf("REJECT: Length mismatch\n");
			}
			else if((int)return_buffer[6] == 0xF6)
			{
				printf("REJECT: End of packet missing\n");
			}
			else if((int)return_buffer[6] == 0xF7)
			{
				printf("REJECT: Duplicate packet\n");
			}
			return 0;
		}
		counter++; //counter increment
	}
	return 0;
}
