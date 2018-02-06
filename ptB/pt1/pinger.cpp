#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "conio.h"

#define ICMP_ECHO       8
#define ICMP_DEST_UNREACH 3
#define ICMP_TTL_EXPIRE 11
#define ICMP_ECHO_REQUEST 8

// ICMP header
struct ICMPHeader {
    BYTE type;          // ICMP packet type
    BYTE code;          // Type sub code
    USHORT checksum;
    USHORT id;
    USHORT seq;
    ULONG timestamp;    // not part of ICMP, but we need it
};

// The IP header
struct IPHeader {
    BYTE h_len:4;           // Length of the header in dwords
    BYTE version:4;         // Version of IP
    BYTE tos;               // Type of service
    USHORT total_len;       // Length of the packet in dwords
    USHORT ident;           // unique identifier
    USHORT flags;           // Flags
    BYTE ttl;               // Time to live
    BYTE proto;             // Protocol number (TCP, UDP etc)
    USHORT checksum;        // IP checksum
    ULONG source_ip;
    ULONG dest_ip;
};


unsigned short in_cksum(unsigned short *ptr, int nbytes)
{
    register long sum;
    u_short oddbyte;
    register u_short answer;
 
    sum = 0;
    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }
 
    if (nbytes == 1) {
        oddbyte = 0;
        *((u_char *) & oddbyte) = *(u_char *) ptr;
        sum += oddbyte;
    }
 
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
 
    return (answer);
}

void setUpRawSocket(SOCKET &s, int ttl);

void allocPackets(ICMPHeader* &sendPacket, IPHeader* &recvPacket, int packet_size);

void setupICMPHeader(ICMPHeader* &sendPacket, std::string &payloadStr, int packet_size);

void packetFinalPrep(ICMPHeader* sendPacket, int packet_size);

int sendPing(SOCKET s, sockaddr_in& dest, ICMPHeader* sendPacket, 
        int packet_size);

int recvPing(SOCKET s, sockaddr_in& source, IPHeader* recvPacket, 
        int packet_size);


int main(int argc, char const *argv[])
{
	bool writeToLog = false;
	std::string payloadStr;
	int count;
    std::string destIpStr;
    struct sockaddr_in destIP;
    struct sockaddr_in source;

    struct IPHeader *recvPacket = NULL;
    struct ICMPHeader *sendPacket = NULL;
    SOCKET s;

    int payload_size = 5;
    int packet_size = sizeof(ICMPHeader) + payload_size;
   	int recvPacketSize = 1024;
    int ttl = 30;

    std::cout << sizeof(struct ICMPHeader) << std::endl;

	//parse cmd line arguments
	// set: writeToLof, payloadStr, count & destIpStr
	if (argc < 7) { // Check the value of argc. If not enough parameters have been passed, inform user and exit.
        std::cout << "Usage is -in <infile> -out <outdir>\n"; // Inform the user of how to use the program
        std::cin.get();
        exit(0);
    } else { // if we got enough parameters...
        for (int i = 1; i < argc; ) { /* We will iterate over argv[] to get the parameters stored inside.
                                          * Note that we're starting on 1 because we don't need to know the 
                                          * path of the program, which is stored in argv[0] */
            //std::cout << argv[1] << std::endl;
            if (i + 1 != argc) { // Check that we haven't finished parsing already
                //std::cout << "check" << std::endl;
                if (argv[i] == "-l") {
                    writeToLog = true;
                    i++;
                } else if (strcmp(argv[i],"-p") == 0) {
                	//std::cout << "checkP" << std::endl;
                    payloadStr = argv[i + 1];
                    i+=2;
                } else if (strcmp(argv[i],"-c") == 0) {
                	//std::cout << "checkC" << std::endl;
                    count = atoi(argv[i + 1]);
                    i+=2;
                } else if (strcmp(argv[i],"-d") == 0) {
                	//std::cout << "checkD" << std::endl;
                    destIpStr = argv[i + 1];
                    i+=2;
                } else {
                    std::cout << "Not enough or invalid arguments, please try again.\n";
                    exit(0);
                }
            } else {
            	i++;
            }
            //std::cout << argv[i] << " ";
        }
    }

    //set destAddr to the IP address passed as cmd argument
    destIP.sin_family = AF_INET;
    destIP.sin_addr.s_addr = inet_addr(destIpStr.c_str());

    //Initialise Winsock
    ////////////////////////////////////////////////
	WSADATA wsock;
	//std::cout << "\nInitialising Winsock...\n";
	if (WSAStartup(MAKEWORD(2,2),&wsock) != 0) 
	{
		fprintf(stderr,"WSAStartup() failed");
		exit(EXIT_FAILURE);
	}
	//std::cout << "Initialised successfully.\n";
	////////////////////////////////////////////////

	//Open RAW socket and set options
	setUpRawSocket(s, ttl);
	//

	//allocates the send and receive packets
	allocPackets(sendPacket, recvPacket, packet_size);
	//

	//Sets up ICMP header info
	setupICMPHeader(sendPacket, payloadStr, packet_size);


    //Required output
    std::cout << "Pinging " << destIpStr << " with " << payload_size << " bytes of data \"" << payloadStr << "\"\n"; 

	//send packet
    for (int i=0; i<count; i++) {
        packetFinalPrep(sendPacket, packet_size);
        sendPing(s, destIP, sendPacket, packet_size);
    }


    //Statistics variables//
    ULONG totalTime = 0;
    int numReceived = 0;
    int min = 500;
    int max = 0;
    ////////////////////////
    int res;
    for (int i=0; i<count; i++) {
	//while(1) {
        res = recvPing(s, source, recvPacket, recvPacketSize);
        if (res == -1) {
            break;
        } else if (res) {
            int headerLen = recvPacket->h_len*4;
            ICMPHeader* ih = (ICMPHeader*)((char*)recvPacket + headerLen);

            if (ih->type != ICMP_ECHO) {
                ULONG elapsedTime = GetTickCount() - ih->timestamp;
                /// stats ///
                totalTime += elapsedTime;
                numReceived++;
                if (elapsedTime<min) {min = elapsedTime;}
                if (elapsedTime>max) {max = elapsedTime;}
                /////////////
                int _ttl = recvPacket->ttl;
                //required output
                std::cout << "Reply from " << destIpStr << ": bytes=" << payload_size << " time=" 
                    << elapsedTime << " TTL=" << _ttl << std::endl;
            } else if (ih->type == ICMP_TTL_EXPIRE) {
                std::cout << "TTL expired.\n";
                exit(-1);
            } else if (ih->type == ICMP_DEST_UNREACH) {
                std::cout << "Destination unreachable.\n";
                exit(-1);
            } else {
                std::cout << "Unknown packet type.\n";
                exit(-1);
            }
                    
        // If "TTL expired", fall through.  Next test will fail if we
        // try it, so we need a way past it.
        } else {
            exit(-1);
        }
	}

    //Print statistics 
    std::cout << "Ping statistics for " << destIpStr << ":\n";
    std::cout << "Packets: Sent = " << count << ", Received = " << numReceived
        << ", Lost=" << count-numReceived << "(" << ((float)(count-numReceived)/count)*100 
            << "%% loss),\n";
    if (res>0) {
        std::cout << "Approximate RTT in milli-seconds:\n";
        std::cout << "Minimum = " << min << "ms, Maximum = " << max << "ms, Average = "
            << totalTime/numReceived << "ms\n";
    }

    //socket::close(s);
    return 0;
}

void setUpRawSocket(SOCKET &s, int ttl) {
	//std::cout << "Setting up RAW Socket...\n";
	if ((s = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, 0, 0, WSA_FLAG_OVERLAPPED)) == SOCKET_ERROR)
	{
        printf("Failed to create raw icmp packet");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(s, IPPROTO_IP, IP_TTL, (const char*)&ttl, 
            sizeof(ttl)) == SOCKET_ERROR) {
        std::cout << "setsockopt failed.\n";
        exit(EXIT_FAILURE);
    }
    DWORD timeout = 2000;
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, 
            sizeof(struct timeval)) == SOCKET_ERROR) {
        std::cout << "setsockopt failed.\n";
        exit(EXIT_FAILURE);
    }
    //std::cout << "Set up RAW Socket.\n";
}

void allocPackets(ICMPHeader* &sendPacket, IPHeader* &recvPacket, int packet_size) {
	//std::cout << "Allocating packets...\n";
	// First the send buffer
    sendPacket = (ICMPHeader*)new char[packet_size];  
    if (sendPacket == 0) {
        std::cout << "Failed to allocate send packet.\n";
        exit(-1);
    }
    //std::cout << "sendPacket size: " << packet_size << std::endl;

    memset (sendPacket, 0, packet_size);

    // And then the receive buffer
    recvPacket = (IPHeader*)new char[1024];
    if (recvPacket == 0) {
        std::cout << "Failed to allocate recv packet.\n";
        exit(-1);
    }


    memset (recvPacket, 0, 1024);
    //std::cout << "recPacket size: " << sizeof(*recvPacket) << std::endl;
    //std::cout << "Allocated.\n";
}

void setupICMPHeader(ICMPHeader* &sendPacket, std::string &payloadStr, int packet_size) {
	//std::cout << "Setting up ICMP Header...\n";
	sendPacket->type = ICMP_ECHO;
    sendPacket->code = 0;
    sendPacket->seq = rand();
    sendPacket->id = rand();
    char* datapart = (char*)sendPacket + sizeof(struct ICMPHeader);
    int spaceLeft = packet_size - sizeof(struct ICMPHeader);
    memcpy(datapart, &payloadStr, std::min(int(sizeof(payloadStr)), spaceLeft));
    //spaceLeft -= sizeof(payloadStr);
    //datapart += sizeof(payloadStr);
    //memset(datapart, 1, spaceLeft);

    //checksum
    //sendPacket->timestamp = GetTickCount();
    //sendPacket->checksum = 0;
    //sendPacket->checksum = in_cksum((unsigned short *)sendPacket, packet_size);
    //std::cout << "Set up header.\n";
}

void packetFinalPrep(ICMPHeader* sendPacket, int packet_size) {
    sendPacket->timestamp = GetTickCount();
    sendPacket->checksum = 0;
    sendPacket->checksum = in_cksum((unsigned short *)sendPacket, packet_size); 
}

int sendPing(SOCKET s, sockaddr_in& dest, ICMPHeader* sendPacket, int packet_size) {
	//std::cout << "Sending...\n";
	int bytes = sendto(s, (char*)sendPacket, packet_size, 0, (sockaddr *)&dest, sizeof(sockaddr_in));
    if(bytes < 0)
    {
        printf("Failed to send to receiver\n");
        exit(1);
    }
    //std::cout << "Sent!\n";
    return 0;
}

int recvPing(SOCKET s, sockaddr_in& source, IPHeader* recvPacket, int packet_size) {
	std::cout << "Receiving...\n";
    //std::cout << "recPacket size: " << sizeof(recvPacket) << std::endl;
	int len = sizeof(source);
	int bytes = recvfrom(s, (char*)recvPacket, 1024, 0, (struct sockaddr*)&source, (socklen_t *)&len);
	//std::cout << bytes << std::endl;
    if(bytes < 0)
    {
        if (WSAGetLastError() == 10060) {
            return -1;
        }
        std::cout << "closesocket failed with error " << WSAGetLastError() << std::endl;
        exit(1);
    }
    std::cout << "Received!\n";
    return 1;
}	