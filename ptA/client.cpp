/*
    UDP client
*/
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <pthread.h>
 
#pragma comment(lib,"ws2_32.lib") //Winsock Library
 
#define SERVER "127.0.0.1"  //ip address of udp server
#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data

int myID;

void setupConnection(SOCKET &s, sockaddr_in &otherInfo, WSADATA &wsaData);

struct Thread_data {
	SOCKET s;
	sockaddr_in oI;
};

void *threadFuncSend(void *threadArg) {
	struct Thread_data * t_data;
	t_data = (struct Thread_data*) threadArg;

	struct sockaddr_in otherInfo = t_data->oI;
	SOCKET s = t_data->s;

	int otherLen = (int)sizeof(struct sockaddr_in);

	while (true) {
		std::string input;
		getline(std::cin, input);
		if (input.compare("exit") == 0) {
			std::cout << "client" << myID << " exit\n";
			return NULL;
		}
		const char *cstr = input.c_str();
		
		if (sendto(s, cstr, strlen(cstr) , 0 , (struct sockaddr *) &otherInfo, otherLen) 
			== SOCKET_ERROR) {
			printf("sendto() failed with error code : %d" , WSAGetLastError());
			exit(EXIT_FAILURE);
		} //std::cout << "sent message\n";
	}
}

void *threadFuncRecv(void *threadArg) {
	struct Thread_data * t_data;
	t_data = (struct Thread_data*) threadArg;
	struct sockaddr_in otherInfo = t_data->oI;
	SOCKET s = t_data->s;

	char recvBuf[BUFLEN];
	int otherLen = (int)sizeof(struct sockaddr_in);

	while (true) {
		memset(recvBuf, 0, sizeof(recvBuf));
		if (recvfrom(s, recvBuf, BUFLEN, 0, (struct sockaddr *)&otherInfo, &otherLen) < 0) {
			std::cout << "Could not receive datagram.\n";
		    closesocket(s);
		    WSACleanup();
		    exit(0);
		} std::cout << "client" << myID << " " << recvBuf << std::endl;
	}
}

int main(void) {

	SOCKET s;
	struct sockaddr_in otherInfo;
	char buf[BUFLEN];
	WSADATA wsaData;


	// open log file stream
	std::ofstream log_file("ClientLogfile.txt", std::ios_base::out | std::ios_base::trunc);

	// 
	setupConnection(s, otherInfo, wsaData);

    // communicate
    //send the message
    int otherLen = (int)sizeof(struct sockaddr_in);

    log_file << "connecting to server " << SERVER << " at port " << PORT << std::endl;
    log_file << "sending register message client" << myID << std::endl;

    char message[BUFLEN] = "register client";
    char recvBuf[BUFLEN];
    if (sendto(s, message, strlen(message) , 0 , (struct sockaddr *) &otherInfo, otherLen) 
    	== SOCKET_ERROR) {
		printf("sendto() failed with error code : %d" , WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	if (recvfrom(s, recvBuf, BUFLEN, 0, (struct sockaddr *)&otherInfo, &otherLen) < 0) {
		std::cout << "Could not receive datagram.\n";
	    closesocket(s);
	    WSACleanup();
	    exit(0);
	}
	//std::cout << recvBuf << std::endl;
	if(recvBuf[0] == 'a') {
		myID = 1;
	} else if (recvBuf[0] == 'b') {
		myID = 2;
	} else if (recvBuf[0] == 'c') {
		myID = 3;
	} else if (recvBuf[0] == 'd') {
		myID = 4;
	} else if (recvBuf[0] == 'e') {
		myID = 5;
	} else if (recvBuf[0] == 'f') {
		myID = 6;
	} else {
		myID = 7;
	}

	std::cout << "client" << myID << " connected to server and registered\n";
	std::cout << "client" << myID << " waiting for messages..\n";
	log_file << "received welcome" << std::endl;


	pthread_t *clientThreads = new pthread_t[2];

	struct Thread_data threadData;
	threadData.s = s;
	threadData.oI = otherInfo; // questionable


	int rc;
	rc = pthread_create(&clientThreads[0], NULL, threadFuncSend, (void*) &threadData); 
	if (rc) {
		std::cout << "creating error" <<"\n";
		exit(-1);
	}
	rc = pthread_create(&clientThreads[1], NULL, threadFuncRecv, (void*) &threadData); 
	if (rc) {
		std::cout << "creating error" <<"\n";
		exit(-1);
	}

	pthread_join(clientThreads[0], NULL);

	log_file << "terminating client..." << std::endl;
	exit(0);

	
}


void setupConnection(SOCKET &s, sockaddr_in &otherInfo, WSADATA &wsaData) {
	// Initialize Winsock
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
	    std::cout << "WSAStartup failed with error : " << WSAGetLastError() << std::endl;
	    exit(EXIT_FAILURE);
	} 
		//std::cout << "Initialized.\n";


	// Create socket
	if((s = socket(AF_INET , SOCK_DGRAM , 0 )) == INVALID_SOCKET)
	{
		std::cout << "Client could not create socket : " << WSAGetLastError() << std::endl;
		exit(EXIT_FAILURE);
	}
		//printf("Client socket created.\n");

	// Set up 'other' struct
	memset((void *)&otherInfo, '\0', sizeof(struct sockaddr_in));
	otherInfo.sin_family = AF_INET;
    otherInfo.sin_port = htons(PORT);
    otherInfo.sin_addr.S_un.S_addr = inet_addr(SERVER);    
}