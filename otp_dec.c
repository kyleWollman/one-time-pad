/************************************************************
* Author: Written by Kyle Wollman (wollmank@oregonstate.edu)
* Date: 03/17/2018
* Class: CS344 400 W2018
* Description: Program 4 - otp_dec.c: Creates a client socket
*				and calls otp_dec_d to decrypt a message from
*				the file passed in at the command line using
*				the key file passed in at the command line.
**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/ioctl.h>
#include <fcntl.h>

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead, plainFD, keyFD, i = 0, j = 0;
	char c;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[200000], key[200000], encID[6], length[20], message[500000];
    memset(buffer, '\0', sizeof(buffer));
	memset(key, '\0', sizeof(key));
	memset(message, '\0', sizeof(message));
	memset(length, '\0', sizeof(length));
	strcpy(encID, "decode");
	
	if (argc < 4) { fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]); exit(0); } // Check usage & args
	
	plainFD = open(argv[1], O_RDONLY);
	
	//get first char of plaintext
	read(plainFD, &buffer[i], 1);

	c = buffer[i];
	
	//get rest of message
	while(c != '\n')
	{
		i++;
		read(plainFD, &buffer[i], 1);
		c = buffer[i];
		//check for illegal characters
		if(c != '\n' && c != ' ')
		{
			if(c > 'Z' || c < 'A')
			{
					fprintf(stderr, "otp_dec error: input contains bad characters\n");
					exit(1);
			}
		}
	}
	
	buffer[strcspn(buffer, "\n")] = '\0'; //remove trailing \n
	
	
	keyFD = open(argv[2], O_RDONLY);
	
	//get first char of key
	read(keyFD, &key[j], 1);
	c = key[j];
	
	for(j = 1; j < i; j++)
	{
		read(keyFD, &key[j], 1);
		c = key[j];
		if(c == '\0')
		{
			fprintf(stderr, "Error: key '%s' is too short\n", argv[2]);
			exit(1);
		}
	}
	
	//convert length of message to string
	sprintf(length, "%d", j);
	
	
	strcat(message, encID);
	strcat(message, length);
	strcat(message, buffer);
	strcat(message, key);
	
	
	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	// Send message to server
	charsWritten = send(socketFD, message, sizeof(message), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if(charsWritten < j) 
	{
		fprintf(stderr, "CLIENT: WARNING: Not all data written to socket!\n");
	}
	
	int checkSend = -5;  // Holds amount of bytes remaining in send buffer
	do
	{
		ioctl(socketFD, TIOCOUTQ, &checkSend);  // Check the send buffer for this socket
	}
	while (checkSend > 0);  // Loop forever until send buffer for this socket is empty
	
	if (checkSend < 0) error("ioctl error");  // Check if we actually stopped the loop because of an error
	
	// Get return message from server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, j, MSG_WAITALL); // Read data from the socket waiting for entire message
	
	//check if client rejected
	if((strcmp(buffer, "rejected")) == 0)
	{
		fprintf(stderr, "Error: could not contact otp_enc_d on port %d\n", atoi(argv[3]));
		exit(2);
	}
	
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
	printf("%s\n", buffer);

	close(socketFD); // Close the socket
	
	return 0;
}

