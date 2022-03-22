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
	int sock, bind_result, sign, length, fromlen;
	struct sockaddr_in server;
	struct sockaddr_in from;
	unsigned char buf[14];
	
	// create socket
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock<0)
	{
		error("Error: socket");
	}

	// define server address
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET; // Internet, IPv4
	server.sin_port = htons(59003); // give a port
	server.sin_addr.s_addr = INADDR_ANY; // get IP

	//bind
	length = sizeof(server);
	bind_result = bind(sock, (struct sockaddr*)&server, length);
	if (bind_result <0)
	{
		error("Error: bind");
	}
	
	//read numbers from database
	// declare varibles to hold user number, technology, status
	unsigned long db_usr[3];
	int db_tech[3], db_stat[3];
	int db_counter = -1;
	char file[100];
	//open file
	FILE *fp = fopen("Verification_Database.txt", "r");
	if(fp == NULL)
	{
		printf("Unable to open file");
		return 0;
	}
	//read number and store accordingly
	while(db_counter < 3)
	{
		// get numbers from database
		fgets(file, sizeof(file), fp);
		if(db_counter > -1) //ignore first row
		{
			db_usr[db_counter] = strtoul(file, NULL, 10); //read user number
		 	db_tech[db_counter]= strtoul(&file[11], NULL, 10); //read technology
			db_stat[db_counter] = strtoul(&file[14], NULL, 10); //read status	
		}
		db_counter++;
	}
 	//close file
	fclose(fp);

	//receive and send
	fromlen = sizeof(struct sockaddr_in);
	bzero(buf, 14); // clean up buffer for nest packet
	while(1)
	{
		// receive datagram
		sign = recvfrom(sock,buf,14,0,(struct sockaddr *)&from, (socklen_t *)&fromlen);
		// calculate true subscriber number from 4 chars, 2^0 2^8 2^16 2^24
		unsigned long usr = (int)buf[11] * 1 + (int)buf[10] * 256 + (long)buf[9] * 65536 + (long)buf[8] * 16777216;	
		// declare a indicator, turns to 1 when user found in database
		int cond_ind = 0;
		// determine which response to give back
		for (int i = 0; i < 3; i++)
		{
			if( usr == db_usr[i] && (int)buf[7] == db_tech[i] && db_stat[i] == 1)
			{
				// subscriber number found, technology correct, paid
				buf[4] = 0xFB;
				sign = sendto(sock, buf, 14, 0, (struct sockaddr*)&from, fromlen);
				cond_ind = 1;
				break;
			}
			else if( usr == db_usr[i] && (int)buf[7] == db_tech[i] && db_stat[i] != 1)
			{
				// subscriber number found, technology correct, not paid
				buf[4] = 0xF9;
				sign = sendto(sock, buf, 14, 0, (struct sockaddr*)&from, fromlen);
				cond_ind = 1;
				break;
			}
		}
		if(cond_ind == 0)
		{	
			// subscriber number not found or technology incorrect
			buf[4] = 0xFA;
			sign = sendto(sock, buf, 14, 0, (struct sockaddr*)&from, fromlen);
		}
		bzero(buf, 14); // clean up buffer for nest packet
	}
	return 0;
}
