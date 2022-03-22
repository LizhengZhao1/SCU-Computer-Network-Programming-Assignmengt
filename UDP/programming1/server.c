// Author: Lizheng Zhao
// Student ID: W1608646
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// socket headers
#include <sys/types.h>
#include <sys/socket.h>
// structure for address info
#include <netinet/in.h>
#include <netdb.h>

// error method to display errors during connection
void error(char *msg){
	perror(msg);
	exit(0);
}

int main(){
	int sock, bind_result, msg, length, fromlen;
	struct sockaddr_in server;
	struct sockaddr_in from;
	unsigned char buf[264];
	char payload[255];
	unsigned char return_buf[10];
	int seg_index = -1;
	int current_seg = -1;
	
	// create socket
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock<0)
	{
		error("Error: socket");
	}

	// define server address
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET; // IPv4, Internet
	server.sin_port = htons(59002); // give a port
	server.sin_addr.s_addr = INADDR_ANY; // get server IP

	// bind
	length = sizeof(server);
	bind_result = bind(sock, (struct sockaddr*)&server, length);
	if (bind_result <0)
	{
		error("Error: bind");
	}

	// receive and send
	fromlen = sizeof(struct sockaddr_in);
	bzero(buf, 264); // clean up buffer for next packet
	while(1)
	{
		// receive datagram
		msg = recvfrom(sock,buf,1024,0,(struct sockaddr *)&from, (socklen_t *)&fromlen);

		// put payload into a separate string
		for (int i = 0; i < 255; i++)     
		{
			payload [i] = buf[i + 7];
		}
		bzero(return_buf, 10); //clean the buffer that hold return values
		
		// configure return packet(reject)
		// start of packet id
		return_buf[0] = buf[0]; 
		return_buf[1] = buf[1];
		//client ID
		return_buf[2] = buf[2];
		// REJECT
		return_buf[3] = 0xFF;
		return_buf[4] = 0xF3;
		// reject sub code, first part
		return_buf[5] = 0xFF;
		// received seg num
		return_buf[7] = buf[5];
		// end of packet id
		return_buf[8] = 0xFF;
		return_buf[9] = 0xFF;
		
		// test for faulty conditions
		if( current_seg != -1 && buf[5] ==  current_seg) //duplicated packet
		{
			// reject sub code, second part
			return_buf[6] = 0xF7;
			msg = sendto(sock, return_buf, 10, 0, (struct sockaddr*)&from, fromlen);
		}
		else if((int)buf[5] != seg_index && seg_index != -1)  //out of sequence
		{	
			// reject sub code, second part
			return_buf[6] = 0xF4;
			msg = sendto(sock, return_buf, 10, 0, (struct sockaddr*)&from, fromlen);
		}
		else if( (int)buf[6] != strlen(payload)) //length mismatch
		{
			// reject sub code, second part
			return_buf[6] = 0xF5;
			msg = sendto(sock, return_buf, 10, 0, (struct sockaddr*)&from, fromlen);
		}
		else if( (int)buf[263] != 0xFF || (int)buf[262] != 0xFF) //end of packet missing
		{
			// reject sub code, second part
			return_buf[6] = 0xF6;
			msg = sendto(sock, return_buf, 10, 0, (struct sockaddr*)&from, fromlen);
		}
		else
		{	
			printf("Received a datagram: ");
			printf("%s\n", payload);
			// modify return buffer for ACK
			// ACK
			return_buf[4] = 0xF2;
			// received seg number
			return_buf[5] = buf[5];
			// end of packet
			return_buf[6] = 0xFF;
			return_buf[7] = 0xFF;
			// clean up rest of return buffer
			return_buf[8] = '\0';
			return_buf[9] = '\0';
			msg = sendto(sock, return_buf, 10, 0, (struct sockaddr*)&from, fromlen);
			seg_index = buf[5] + 1; // incremnet sequence number expected for next incoming packet
			current_seg = buf[5]; // record current sequence number
		}
		bzero(buf, 264); // clean up buffer for next packet
	}
	return 0;
}
