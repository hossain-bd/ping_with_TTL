/* 
 * Mohammad Nazmul Hossain
 * Communication Systems and Networks
 * Martikel No.: 11106776
 * E-mail: nazmul.hossain@engineer.com
 *
 *
 * ####################################################
 *
 * USAGE Example: ./Lab_Final 172.217.22.36 0
 *                ./Lab_Final 172.217.23.132 443
 * 
 *
 * ####################################################
 * 
 * 
 * This script is written for the Linux environment
 * This program will take the destination IP address and port number from the user
 * and then send a UDP packet to that IP.
 * 
 * The TTL parameter is set to '1' so that the packet can travel only one hop
 * After the 1st hop the TTL decreased to zero and the hop send back an ICMP Time Exceeded replay
 * 
 * The output shows sending and recieving ICMP packet types and code.
 * A Time to live Exceeded message will be generated in the output terminal by the responding L3 device
 *
 *
 *
 * USAGE:   ./name_executable DestinationIP DestinationPort
 *          
 *
 *
 *          Before execution this script need to be compile
 *          To compile # make <Filename>
 *
*/



#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/ip.h>
#include<netinet/ip_icmp.h>

#define SA (struct sockaddr*)
#define buf_size	1024
#define TTL		1


/*
 *  Generic checksum calculation function
 *  
*/

unsigned short csum (unsigned short *pckt,int nbytes) {
	unsigned short *ptr = pckt;
    	register long sum;
    	unsigned short oddbyte;
    	register short answer;
 
 	sum=0;
 	while(nbytes>1) {
 		sum+=*ptr++;
 	       	nbytes-=2;
 	}
 	if(nbytes==1) {
 	       oddbyte=0;
 	       *((u_char*)&oddbyte)=*(u_char*)ptr;
 	       sum+=oddbyte;
 	}
	sum = (sum>>16)+(sum & 0xffff);
	sum = sum + (sum>>16);
	answer=(short)~sum;
    	return(answer);
}

int main (int argc, char *argv[]) {

	if (argc != 3) {
      		printf ("Required arguments: <dst IP> <port>\n");
      	exit (0);
    	}

	int sc = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP);	//create a socket
  	char buf1[buf_size] = { 0 }, buf2[buf_size] = { 0 };

	struct sockaddr_in dst;			//destination ddress from the user input 
	struct sockaddr_in hop_addr;		//hop addresses
  	int ttl = TTL, hop = 1, one = 1;
  	const int *val = &one;


 	if (setsockopt (sc, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
    		printf ("Cannot set HDRINCL!\n");
  	setsockopt(sc, SOL_IP, IP_TTL, &ttl, sizeof(ttl));

  	dst.sin_port = htons (*argv[2]);
  	dst.sin_family = AF_INET;
  	inet_pton (AF_INET, argv[1], &(dst.sin_addr));
  	//dst.sin_addr.s_addr = inet_addr ("172.217.22.68");
  	//inet_pton (AF_INET, argv[1], &(dst.sin_addr));


  	while (1) {
      	if(hop == 2)
		goto last;

	// Manually configuring IP Header parameters
	struct ip *ip_hdr = (struct ip *) buf1;
      	ip_hdr->ip_hl = 5;		/* header length */
      	ip_hdr->ip_v = 4;		/* version */
     	ip_hdr->ip_tos = 0;		/* type of service */
      	ip_hdr->ip_len = 20 + 8;	/* total length */
      	ip_hdr->ip_id = 10000;		/* identification */
      	ip_hdr->ip_off = 0;		/* fragment offset field */
      	ip_hdr->ip_ttl = hop;		/* time to live */
      	ip_hdr->ip_p = IPPROTO_ICMP;	/* protocol */
      	inet_pton (AF_INET, "localhost", &(ip_hdr->ip_src));	/* source address */
      	ip_hdr->ip_dst = dst.sin_addr;				/* dest address */
      	//inet_pton (AF_INET, argv[1], &(ip_hdr->ip_dst));
      	ip_hdr->ip_sum = csum ((unsigned short *) buf1, 9);	/* checksum */

	// Manually configuring ICMP Header parameters
      	struct icmphdr *icmphd = (struct icmphdr *) (buf1 + 20);	//initializing sending icmp header
      	icmphd->type = ICMP_ECHO;		/* message type */
      	icmphd->code = 0;			/* type sub-code */
      	icmphd->checksum = 0;			/* checksum */
      	icmphd->un.echo.id = 0;			/* identification */
      	icmphd->un.echo.sequence = 0;		/* Secuence Number */	
      	icmphd->checksum = csum ((unsigned short *) (buf1 + 20), 4);
      
	//Format: sendto (sc, buf1, 28, 0, (struct sockaddr*) & dst, sizeof dst);
	//Send Packet too dst
      	sendto (sc, buf1, sizeof(struct ip) + sizeof(struct icmphdr), 0, (struct sockaddr*) & dst, sizeof dst);

      	socklen_t len = sizeof (struct sockaddr_in);		//get size of socket
      	
	//recvfrom (sc, buf2, 28, 0, (struct sockaddr*) & hop_addr, &len);
        recvfrom (sc, buf2, sizeof(buf2), 0, (struct sockaddr*) & hop_addr, &len);	//recieve the server response in buf2[]
        
	
	struct icmphdr *icmphd2 = (struct icmphdr *) (buf2 + 20);	//initializing recieving icmp header
      

/* 	The following conditions are checked and responsed by printf("") according to the ICMP types
 *	
 *	Ref. https://www.ibm.com/support/knowledgecenter/en/SS42VS_7.3.1/com.ibm.qradar.doc/c_DefAppCfg_guide_ICMP_intro.html
 *
*/

      	if (icmphd->type == 8)		// type 8 is Echo Request
		printf("\nSending ICMP Echo Request to : %s\n\n", inet_ntoa(dst.sin_addr));

	printf("ICMP sent type: %d\n", icmphd->type);
      	printf("ICMP recieved type: %d\n", icmphd2->type);
	printf("ICMP recieved code: %d\n", icmphd2->code);

	if (icmphd2->type == 11 && icmphd2->code == 0)	//type 11 code 0 is Time to Live exceeded in transit message
							//type 11 code 1 is Fragment reassembly time exceeded
		printf("\nReply from hop no. %d Address: %s Time to live Exceeded\n\n",hop, inet_ntoa (hop_addr.sin_addr));
	else
      	{
      	printf ("\nReached destination:%s with hop limit:%d\n\n",inet_ntoa (hop_addr.sin_addr), hop);	//else assumed type 0 will be recieved
									//normally type 0 has been reieved in this code other than 11 & 8

      	exit (0);

      	}
      	last:	//Level for goto from begining of while loop
      	if (hop == ttl) 
		goto out;
      	hop++;

    	}
  	out:	//level for goto if hop count reaches to given TTL value at the defination	
  	return 0;
}
