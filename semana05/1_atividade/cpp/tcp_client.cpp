// TCP/IP - CLIENT
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Aux.h"

using namespace std;


int main(int argc, char *argv[]) 
{
    if (argc < 3 || argc > 4) // Test for correct number of arguments
    DieWithUserMessage("Parameter(s)",
            "<Server Address> <Input file name> [<Server Port>]");

    char *servIP = argv[1];     // First arg: server IP address (dotted quad)
    char *inPathFile = argv[2]; // Second arg: string to echo

    // Third arg (optional): server port (numeric).  7 is well-known echo port
    in_port_t servPort = (argc == 4) ? atoi(argv[3]) : 7;

    // Create a reliable, stream socket using TCP
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
        DieWithSystemMessage("socket() failed");
    else
    	printf("Socket successfully created...\n");
    // Construct the server address structure
    struct sockaddr_in servAddr;            // Server address
    memset(&servAddr, 0, sizeof(servAddr)); // Zero out structure
    servAddr.sin_family = AF_INET;          // IPv4 address family
    
    
    // Convert address
    int rtnVal = inet_pton(AF_INET, servIP, &servAddr.sin_addr.s_addr);
    if (rtnVal == 0)
        DieWithUserMessage("inet_pton() failed", "invalid address string");
    else if (rtnVal < 0)
        DieWithSystemMessage("inet_pton() failed");
    else
    	printf("Address successfully converted...\n");
    servAddr.sin_port = htons(servPort);    // Server port

    // Establish the connection to the echo server
    if (connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        DieWithSystemMessage("connect() failed");
    else
    	printf("Connected to the server...\n");
    
    // Send the string to the server
    ifstream inFile{inPathFile};
    static constexpr std::size_t buffsizeFile{BUFSIZE};
    char buffer[buffsizeFile]; 
    char bufferRecv[BUFSIZE]; 
    ssize_t numBytes;
    while(inFile.read(buffer, buffsizeFile)){
	    numBytes = send(sock, buffer, buffsizeFile, 0);
	    if (numBytes < 0)
		DieWithSystemMessage("send() failed");
	    else
	    	printf("The file was sent successfully...\n");
    
	    // Receive the message confirm back from the server
	    fputs("Received from Server: ", stdout);     // Setup to print the echoed string
	    /* Receive up to the buffer size (minus 1 to leave space for
	       a null terminator) bytes from the sender */
	    numBytes = recv(sock, bufferRecv, BUFSIZE, 0);
	    if (numBytes < 0)
		DieWithSystemMessage("recv() failed");
	    else if (numBytes == 0)
		DieWithUserMessage("recv()", "connection closed prematurely");
	    bufferRecv[numBytes] = '\0';    // Terminate the string!
	    fputs(bufferRecv, stdout);      // Print the echo buffer
	    fputc('\n', stdout); // Print a final linefeed
    };
 
    numBytes = send(sock, buffer, inFile.gcount(), 0);
    if (numBytes < 0)
	DieWithSystemMessage("send() failed");
    else
    	printf("The file last part was sent successfully...\n");

    // Receive the message confirm back from the server
    fputs("Received from Server: ", stdout);     // Setup to print the echoed string
    /* Receive up to the buffer size (minus 1 to leave space for
       a null terminator) bytes from the sender */
    numBytes = recv(sock, buffer, BUFSIZE - 1, 0);
    if (numBytes < 0)
	DieWithSystemMessage("recv() failed");
    else if (numBytes == 0)
	DieWithUserMessage("recv()", "connection closed prematurely");
    buffer[numBytes] = '\0';    // Terminate the string!
    fputs(buffer, stdout);      // Print the echo buffer
    
    fputc('\n', stdout); // Print a final linefeed
    inFile.close();
    close(sock);
    exit(0);
}
