#include <stdio.h>
#include <winsock2.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_BUFFER_SIZE 1024
//--- change the port ---
#define SERVER_PORT 8888
//-----------------------

char* executeCommand(const char* command) {

    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES saAttr;
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFOA siStartInfo;
    static char szOutput[MAX_BUFFER_SIZE];
    DWORD dwRead;

    // Create a pipe for the child process's STDOUT
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0)) {
        printf("Failed to create pipe.\n");
        return NULL;
    }

    // Set up members of the PROCESS_INFORMATION structure
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    // Set up members of the STARTUPINFOA structure
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFOA));
    siStartInfo.cb = sizeof(STARTUPINFOA);
    siStartInfo.hStdOutput = hWritePipe;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    // Create the child process
    //TODO: Handle bad commands so it doesn't get stuck
    if (!CreateProcessA(NULL, (LPSTR)command, NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo)) {
        printf("Failed to create process.\n");
        return NULL;
    }

    // Wait for the child process to exit
    WaitForSingleObject(piProcInfo.hProcess, INFINITE);

    // Close the write end of the pipe, so the ReadFile will finish
    CloseHandle(hWritePipe);

    // Read the command output from the read end of the pipe
    if (!ReadFile(hReadPipe, szOutput, MAX_BUFFER_SIZE - 1, &dwRead, NULL)) {
        printf("Failed to read pipe.\n");
        return NULL;
    }

    // Null-terminate the output string
    szOutput[dwRead] = '\0';

    // Clean up resources
    CloseHandle(hReadPipe);
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);

    return szOutput;

}

void HandleClient(void* clientSocketPtr) {

    SOCKET clientSocket = *(SOCKET*)clientSocketPtr;
    char buffer[MAX_BUFFER_SIZE];
    int bytesRead, totalBytesRead = 0;

    // Receive message from the client
    while (1) {
        bytesRead = recv(clientSocket, buffer + totalBytesRead, MAX_BUFFER_SIZE - totalBytesRead, 0);
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

    // Null-terminate the received message
    buffer[totalBytesRead] = '\0';

    printf("Received message from client: %s\n", buffer);

    // Send response back to the client
    char* response = executeCommand(buffer);
    if (send(clientSocket, response, strlen(response), 0) == SOCKET_ERROR) {
        printf("Failed to send response to the client.\n");
        closesocket(clientSocket);
        return;
    }

    printf("Response sent to client.\n");

    // Cleanup and close the socket
    closesocket(clientSocket);
    _endthread();
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    int clientAddressSize;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initialize winsock.\n");
        return 1;
    }

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == INVALID_SOCKET) {
        printf("Failed to create socket.\n");
        WSACleanup();
        return 1;
    }

    // Bind socket to an address and port
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(SERVER_PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        printf("Failed to bind socket.\n");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Error while listening for connections.\n");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Server started. Listening for incoming connections...\n");

    while (1) {
        // Accept a connection from a client
        clientAddressSize = sizeof(clientAddress);
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);
        if (clientSocket == INVALID_SOCKET) {
            printf("Failed to accept client connection.\n");
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        printf("Client connected. Waiting for message...\n");

        // Create a new thread to handle the client
        _beginthread(HandleClient, 0, &clientSocket);
    }

    // Close the server socket
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}