#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstring>
#include "pcap.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CAPTURE 2048
#define ETHER_ADDR_LEN	6
#define SIZE_ETHERNET 14

/* Ethernet header */
struct sniff_ethernet {
	u_char ether_dhost[ETHER_ADDR_LEN]; /* Destination host address */
	u_char ether_shost[ETHER_ADDR_LEN]; /* Source host address */
	u_short ether_type; /* IP? ARP? RARP? etc */
};

/* IP header */
	struct sniff_ip {
		u_char ip_vhl;		/* version << 4 | header length >> 2 */
		u_char ip_tos;		/* type of service */
		u_short ip_len;		/* total length */
		u_short ip_id;		/* identification */
		u_short ip_off;		/* fragment offset field */
	#define IP_RF 0x8000		/* reserved fragment flag */
	#define IP_DF 0x4000		/* dont fragment flag */
	#define IP_MF 0x2000		/* more fragments flag */
	#define IP_OFFMASK 0x1fff	/* mask for fragmenting bits */
		u_char ip_ttl;		/* time to live */
		u_char ip_p;		/* protocol */
		u_short ip_sum;		/* checksum */
		struct in_addr ip_src,ip_dst; /* source and dest address */
	};
	#define IP_HL(ip)		(((ip)->ip_vhl) & 0x0f)
	#define IP_V(ip)		(((ip)->ip_vhl) >> 4)

/* ICMP header */
struct sniff_icmp {
    int8_t type;          // ICMP packet type
    int8_t code;          // Type sub code
    unsigned short checksum;
    unsigned short id;
    unsigned short seq;
    unsigned long timestamp;    // not part of ICMP, but we need it
};


// packet handler function
void got_packet(u_char *args, const struct pcap_pkthdr *header,
	    const u_char *packet);

int main(int argc, char const *argv[])
{
	bool writeToLog = false;
	bool readPcapFile = false;
	std::string pcapFileStr;
	int count;
	std::string _dev;


	//parse cmd line arguments
	// set: interface, readFile, count, writeToLog
	if (argc < 5) { // Check the value of argc. If not enough parameters have been passed, inform user and exit.
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
                } else if (strcmp(argv[i],"-r") == 0) {
                	readPcapFile = true;
                	pcapFileStr = argv[i + 1];
                    i+=2;
                } else if (strcmp(argv[i],"-c") == 0) {
                    count = atoi(argv[i + 1]);
                    i+=2;
                } else if (strcmp(argv[i],"-i") == 0) {
                	_dev = argv[i + 1];
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

    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE];
    //const char *dev = _dev.c_str();
    const char *dev = pcap_lookupdev(errbuf);
    if (dev==NULL) {
    	std::cout << "shit\n";
    }

    if (!readPcapFile) {

	    //Open the device for sniffing
	    if ((handle = pcap_open_live(dev, MAX_CAPTURE, 0, 1000,
		    errbuf)) == NULL) {
	    	std::cout << "Unable to open device.\n";
	    	return -1;
	    }

	    struct bpf_program filterStruct;
    	int optimize = 0;
    	char filter_exp[] = "ip proto icmp";

	    if (pcap_compile(handle, &filterStruct, filter_exp, optimize, PCAP_NETMASK_UNKNOWN) < 0){
	   		std::cout << "Failed to compile for filter.\n";
	   	}

	   	if ((pcap_setfilter(handle, &filterStruct))<0) {
	   		std::cout<< "Failed to set filter.\n";
   		}

	   	//begin sniffing
	   	//cnt set to -1 -> will sniff until error
	   	pcap_loop(handle, -1, got_packet, NULL);
	   	std::cout << "hmm\n";

    } else {
    	//Read offline pcap file
    	const unsigned char *packet;
    	struct pcap_pkthdr header;

    	handle = pcap_open_offline(pcapFileStr.c_str(), errbuf);
    	if (handle == NULL)
		{
			fprintf(stderr, "error reading pcap file: %s\n", errbuf);
			exit(1);
		}

		std::cout << "Feature not fully implemented. Exiting. \n";

		while ((packet = pcap_next(handle, &header)) != NULL) {
			//do something
			//dump_UDP_packet(packet, header.ts, header.caplen);
		}
    }

	return 0;
}


void got_packet(u_char *args, const struct pcap_pkthdr *header,
	    const u_char *packet) {

	const struct sniff_ethernet *ethernet; /* The ethernet header */
	const struct sniff_ip *ip; /* The IP header */
	const struct sniff_icmp *icmp; /* The TCP header */
	char *payload; /* Packet payload */

	u_int size_ip;
	u_int size_icmp;

	//////////////////////////////////////////////////////////////
	// typecasting
	//////////////////////////////////////////////////////////////
	ethernet = (struct sniff_ethernet*)(packet);
	ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
	size_ip = IP_HL(ip)*4;
	if (size_ip < 20) {
		printf("   * Invalid IP header length: %u int8_ts\n", size_ip);
		return;
	}
	icmp = (struct sniff_icmp*)(packet + SIZE_ETHERNET + size_ip);
	//
	size_icmp = 4;
	/*if (size_icmp < 20) {
		printf("   * Invalid ICMP header length: %u int8_ts\n", size_icmp);
		return;
	}*/
	payload = (char *)(packet + SIZE_ETHERNET + size_ip + size_icmp);
	//////////////////////////////////////////////////////////////
	int id = icmp->id;
	int seq = icmp->seq;
	//int length = 


	std::cout << "ICMP echo request/reply, id " << id << ", seq " << seq << std::endl;	




}
