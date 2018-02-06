#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <vector>

#pragma comment(lib, "Ws2_32.lib") //Winsock library

#define PORT 8888
#define BUFLEN 512
#define MAX_CLIENTS 10

//struct sockaddr clients[MAX_CLIENTS];
std::vector<struct sockaddr_in> clients;
int recipientID;
int numClients;

void setupConnection(SOCKET &s, sockaddr_in &serverInfo, WSADATA &wsaData);

int messageType(char mes[]) {
  char* mesToks = strtok(mes, " ");
  //bool equal;
  //int res;
  char regBuf[] = "register";
  char sendBuf[] = "sendto";
  bool sending = false;
  int i = 0;
    while(mesToks) {
        //std::cout << mesToks << '\n';
        if (i==0 && strcmp(mes, regBuf) == 0) {
        	return 0;
        } else if (i==0 && strcmp(mes, sendBuf) != 0) {
        	return -1;
        } else if (i==1) {
        	char temp = mesToks[6];
        	recipientID = temp - '0';
        	//std::cout << "setting recID to " << recipientID << std::endl;
        	return 1;
        }

        i++;
        mesToks = strtok(NULL, " ");
    }
   return -1;
}

int main() {

	SOCKET s;
	struct sockaddr_in serverInfo, otherInfo;
	//struct sockaddr_in clients[MAX_CLIENTS];
	char buf[BUFLEN];
	char recvBuf[BUFLEN];
	int otherLen = (int)sizeof(struct sockaddr_in);
	WSADATA wsaData;

	// Open log file stream
	std::ofstream log_file("ServerLogfile.txt", std::ios_base::out | std::ios_base::trunc);

	// Set up server socket
	setupConnection(s, serverInfo, wsaData);


	log_file << "server started on " << inet_ntoa(serverInfo.sin_addr) << " at port " << PORT << std::endl;



	
	

	/*
  ///

  AcceptSocket = accept(s, NULL, NULL);
  if (AcceptSocket == INVALID_SOCKET) {
  		std::cout << "accept failed with error: " << WSAGetLastError() << std::endl;
      closesocket(s);
      WSACleanup();
      return 1;
  } 
  std::cout << "client connected\n";

	*/


	// Register client
	int clientID = 1;
	numClients = 0;
	//std::cout << "Waiting for client to register...\n";
	


	while(true) {
		memset(buf,'\0', BUFLEN);
		if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *)&otherInfo, &otherLen) < 0) {
				std::cout << "Could not receive datagram.\n";
		    closesocket(s);
		    WSACleanup();
		    exit(0);
		}
		std::string recvStr = buf;
		//std::cout << "recvStr: " << recvStr << std::endl;
		int res = messageType(buf);
		
		if (res == 0) {
			clients.push_back(otherInfo);
			numClients++;
			//std::cout << "registered client" << std::endl;

			std::cout << "client" << numClients << " registered from host " 
			<< inet_ntoa(otherInfo.sin_addr) << " port " << ntohs(otherInfo.sin_port) << std::endl;

			log_file << "client connection from host " << inet_ntoa(otherInfo.sin_addr) 
			<< " at port " << PORT << std::endl;

			log_file << "received register client " << numClients 
			<< " from host " << inet_ntoa(otherInfo.sin_addr) 
			<< " port " << ntohs(otherInfo.sin_port) << std::endl;

			//now reply to the client with 'welcome'
			
			if (numClients == 1) {
					strcpy(recvBuf, "a");
			} else if (numClients == 2) {
					strcpy(recvBuf, "b");
			} else if (numClients == 3) {
					strcpy(recvBuf, "c");
			} else if (numClients == 4) {
					strcpy(recvBuf, "d");
			} else if (numClients == 5) {
					strcpy(recvBuf, "e");
			} else {
				strcpy(recvBuf, "f");
			}
			
			//recvBuf = 'welcome';
		  if (sendto(s, recvBuf, BUFLEN, 0, (struct sockaddr*)&otherInfo, otherLen) == SOCKET_ERROR) {
		      printf("\nsendto() failed with error code : %d" , WSAGetLastError());
		     	exit(EXIT_FAILURE);
		  }

		} else if (res == 1) {
			//std::cout << "da fuq" << std::endl;
			//std::cout << "sendto client" << recipientID << std::endl;
			//std::cout << "numClients" << numClients << std::endl;
			if (recipientID <= numClients) {
				sockaddr_in saddr_in = clients[recipientID-1];

				recvStr.erase(0, 14);
				log_file << "sendto client" << recipientID << " from client#\""
				<< recvStr << "\"" << std::endl;
				recvStr = "recvfrom client# " + recvStr;  
				//const char* sendBuf = recvStr.c_str();

				if (sendto(s, recvStr.c_str(), BUFLEN, 0, (struct sockaddr*)&saddr_in, otherLen) == SOCKET_ERROR)
			  {
			      printf("\nsendto() failed with error code : %d" , WSAGetLastError());
			     	exit(EXIT_FAILURE);
			  }

			  
			  //memset(&sendBuf[0], 0, recvStr.length());
		  } else {
		  	std::cout << "That client doesn't exist yet\n";
		  }
		}
	}

   /////


	

	

	

	/*
	// Wait for data from client
	while(1) {
		std::cout << "Waiting for data...\n";

		memset(buf,'\0', BUFLEN);

		if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *)&otherInfo, &otherLen) < 0) {
			std::cout << "Could not receive datagram.\n";
	    closesocket(s);
	    WSACleanup();
	    exit(0);
		}

		printf("Received packet from %s port %d\n", inet_ntoa(otherInfo.sin_addr), ntohs(otherInfo.sin_port));
    printf("Data: %s\n" , buf);
	}
	*/

	log_file << "terminating server... " << std::endl;
	log_file.close();

  return 0;
}


void setupConnection(SOCKET &s, sockaddr_in &serverInfo, WSADATA &wsaData) {

	// Initialize Winsock
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
			std::cout << "WSAStartup failed: " << WSAGetLastError() << std::endl;
	    exit(EXIT_FAILURE);
	} 
	//std::cout << "Initialized.\n";


	// Create socket
	if((s = socket(AF_INET , SOCK_DGRAM , 0 )) == INVALID_SOCKET) {
		std::cout << "Server could not create socket : " << WSAGetLastError() << std::endl;
	}	
	//printf("Server socket created.\n");


	// Set up server struct
	memset((void *)&serverInfo, '\0', sizeof(struct sockaddr_in));
	serverInfo.sin_addr.s_addr = INADDR_ANY;
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_port = htons(PORT);

	//std::cout << "Localhost IP: " << inet_ntoa(serverInfo.sin_addr) << std::endl;

	// bind
	if( bind(s ,(struct sockaddr *)&serverInfo , sizeof(serverInfo)) == SOCKET_ERROR) {
		std::cout << "Bind failed with error code : " << WSAGetLastError() << std::endl;
		closesocket(s);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
}