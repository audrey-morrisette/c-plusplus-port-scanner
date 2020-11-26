#define _WIN32_WINNT 0x501

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>

// Structure to hold port data
struct port{
    int portNumber;
    bool status;
    port(int portNumber){
        this->portNumber = portNumber;
        status = false; // port defaults to closed
    }
    port *next;
};

// TODO - add functionality to scan common ports of interest

int main(int argc, char **argv){

	WSADATA wsaData;
	SOCKET connectSocket = INVALID_SOCKET;
	addrinfo *result, *ptr = NULL;
	addrinfo hints;

	int iResult;
    int highPort;
    int lowPort;

    // port struct to hold head of linked list of ports
    port *headPort = NULL;

	// if no address was supplied, close with options
	if (argc != 5) {
		std::cout << "Usage: PortScanner [address] [low port] [high port] [filename to store results]" << std::endl;
		return 0;
	}
    std::istringstream low(argv[2]);
    std::istringstream high(argv[3]);
    if (!(low >> lowPort && high >> highPort) || lowPort >= highPort || lowPort <= 0) {
        std::cout << "Invalid Arguments" << '\n';
        return 0;
    }

    // creating linked list to hold ports and their respective statuses
    headPort = new port(lowPort);
    port *tempPort;
    tempPort = headPort;
    for(int i = lowPort+1; i <= highPort; i++){
        tempPort->next = new port(i);
        tempPort->next->next = NULL;
        tempPort = tempPort->next;
    }

	// initialize winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(iResult != 0){
        std::cout << "WSAStartup failed with error: " << iResult << std::endl;
	}

	// Resolve the server address and port
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	for(tempPort = headPort; tempPort != NULL; tempPort = tempPort->next){
        if(!tempPort) break;
        std::ostringstream ss;
        ss << tempPort->portNumber;
        // zero out memory
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        iResult = getaddrinfo(argv[1], ss.str().c_str(), &hints, &result);
        if (iResult != 0) {
            std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
            return 1;
        }else{
            connectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
            if(connectSocket == INVALID_SOCKET){
                std::cout << "socket failed with error: " << WSAGetLastError() << std::endl;
                return 1;
            }
            iResult = connect( connectSocket, result->ai_addr, (int)result->ai_addrlen);
            if (iResult == SOCKET_ERROR) {
                closesocket(connectSocket);
                connectSocket = INVALID_SOCKET;
                tempPort->status = false;
                continue;
            }else{
                // connection succeeded
                closesocket(connectSocket);
                connectSocket = INVALID_SOCKET;
                tempPort->status = true;
            }
        }// end if/else
	}

	// will export results to .csv file
	std::ofstream outputFile;
	outputFile.open(argv[4]);
    outputFile << "Port,Status" << std::endl;
	// Displaying the results of the scan
	std::cout << "--------------------------------------" << std::endl;
	std::cout << "|            Scan Results            |" << std::endl;
	std::cout << "--------------------------------------" << std::endl;
	for(tempPort = headPort; tempPort != NULL; tempPort = tempPort->next){
        if(tempPort->status){
            SetConsoleTextAttribute(hConsole, 15);
            std::cout << "Port: " << tempPort->portNumber << std::setw(20) << "\tStatus: ";
            SetConsoleTextAttribute(hConsole, 10);
            std::cout << "Open" << std::endl;
            SetConsoleTextAttribute(hConsole, 15);
        }else{
            SetConsoleTextAttribute(hConsole, 15);
            std::cout << "Port: " << tempPort->portNumber << std::setw(20) << "\tStatus: ";
            SetConsoleTextAttribute(hConsole, 12);
            std::cout << "Closed" << std::endl;
            SetConsoleTextAttribute(hConsole, 15);
        }
        outputFile << tempPort->portNumber << "," << (tempPort->status ? "Open":"Closed") << std::endl;
	}
    std::cout << "--------------------------------------" << std::endl;
	// freeing memory
	while(headPort != NULL){
        tempPort = headPort;
        headPort = headPort->next;
        delete tempPort;
	}

    return 0;
}// end main








