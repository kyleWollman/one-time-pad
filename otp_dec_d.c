/************************************************************
* Author: Written by Kyle Wollman (wollmank@oregonstate.edu)
* Date: 03/17/2018
* Class: CS344 400 W2018
* Description: Program 4 - otp_dec_d.c: Creates a server that
*				receives data from the client containing a 
*				message, key, id word, and the length of the 
*				message. It breaks each of these into its own
*				array and uses the key to decrypt the message
*				using one time pad method. It then sends the 
*				decrypted message back to the client.
**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/time.h>


void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, i, j, c, messLen;
	socklen_t sizeOfClientInfo;
	char buffer[500000], encID[6], length[10], message[200000], key[200000], encMess[500000];
	memset(buffer, '\0', sizeof(buffer));
	memset(encID, '\0', sizeof(encID));
	memset(length, '\0', sizeof(length));
	memset(message, '\0', sizeof(message));
	memset(key, '\0', sizeof(key));
	memset(encMess, '\0', sizeof(encMess));
	struct sockaddr_in serverAddress, clientAddress;
	pid_t spawnPid = -5;
	

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args
	
	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	while(1)
	{
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		
		if (establishedConnectionFD < 0) error("ERROR on accept");
		
		//fork off child process
		spawnPid = fork();
		switch(spawnPid)
		{
			case -1: { perror("Error spawning\n"); exit(1); break; }
			case 0: 
			{
				// Get the message from the client
				memset(buffer, '\0', sizeof(buffer));
				charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer) - 1, MSG_WAITALL); // Read the client's message from the socket
				if (charsRead < 0) error("ERROR reading from socket");

				//retrieve client id
				for(i = 0; i < 6; i++)
				{
					encID[i] = buffer[i];
				}
				
				//check client id
				if(strcmp(encID, "decode"))
				{
					//if not decode reject the client
					strcpy(buffer, "rejected");
					charsRead = send(establishedConnectionFD, buffer, sizeof(buffer), 0);
					if (charsRead < 0) error("ERROR writing to socket");
				}
			
				else
				{
					//get message legnth
					j = 0;
					for(i = 6; i < strlen(buffer); i++)
					{
						if(buffer[i] >= '0' && buffer[i] <= '9')
						{
							length[j++] = buffer[i];
						}
					}
					
					//convert length to int
					messLen = atoi(length);
					
					j = 6 + strlen(length); //encode id + lenght of message
					
					//get message from buffer
					for(i = 0; i < messLen; i++)
					{
						message[i] = buffer[j++];
					}
					
					//get key from buffer
					for(i = 0; i < messLen; i++)
					{
						key[i] = buffer[j++];
					}
					
					//decode the message
					for(i = 0; i < messLen; i++)
					{
						if(message[i] == ' ')//put space at position 27 i.e. just after 'Z'
						{
							message[i] = 91;
						}
						if(key[i] == ' ')//do same for spaces in key
						{
							key[i] = 91;
						}
						c = (message[i] - 65) - (key[i] - 65); //convert ascii to alphabet range
						
						if(c < 0)
							c += 54; //if negative convert to value to use mod operator
						c %= 27;

						c += 65;

						if(c == 91)
						{
							c = 32;
						}
		
						encMess[i] = c;
					}
					
					// Send a decoded message back to the client
					charsRead = send(establishedConnectionFD, encMess, sizeof(encMess), 0); // Send success back
					if (charsRead < 0) error("ERROR writing to socket");
					if(charsRead < messLen)
					{
						fprintf(stderr, "SERVER: WARNING: Not all data written to socket!\n");
					}
					
					int checkSend = -5;  // Holds amount of bytes remaining in send buffer
					do
					{
						ioctl(establishedConnectionFD, TIOCOUTQ, &checkSend);  // Check the send buffer for this socket
					}
					while (checkSend > 0);  // Loop forever until send buffer for this socket is empty
					if (checkSend < 0) error("ioctl error");  // Check if we actually stopped the loop because of an error
					
					close(establishedConnectionFD); // Close the existing socket which is connected to the client
					break;
				}
			}
			
			default:
			{
				struct sigaction SIGCHLD_action = {0};
				SIGCHLD_action.sa_handler = SIG_IGN; //reap children
				sigaction(SIGCHLD, &SIGCHLD_action, NULL);
				break;
			}
		}
		
	}
	
	close(listenSocketFD);
	
	return 0; 
}

