#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>

#define SRC_SEND_PORT 1234
#define SRC_RECV_PORT 6000
#define SRC_ADDRESS "120.120.120.50"
#define DEST_PORT 1025
#define DEST_ADDRESS "120.120.120.51"
#define TS_PATH "./1848_H264_720P_AC3_5994_2M.ts"
#define SAVE_PATH "./audio"	

#define TRANSPORT_SYNC_BYTE   0x47
#define TRANSPORT_PACKET_SIZE 188
#define PACKET_LEN (TRANSPORT_PACKET_SIZE*7)

int open_send_socket(void)
{
	int sockfd;
	struct in_addr addr;
	struct sockaddr_in src_addr;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){ /* create socket object (IPv4,UDP) */
		perror("socket");
		exit(1);
	}

	bzero(&src_addr, sizeof(src_addr));
	src_addr.sin_family = AF_INET; /* addrress family */
	src_addr.sin_port = htons(SRC_SEND_PORT); /* port number */
	if (inet_aton(SRC_ADDRESS, &addr) == 0) {
		perror("inet_aton");
		exit(1);	
	}
	src_addr.sin_addr = addr; /* IPv4 address */
	if (bind(sockfd, (struct sockaddr*)&src_addr, sizeof(src_addr)) < 0) {
		perror("bind");
		exit(1);
	}

	return sockfd;
}

int open_recv_socket(void)
{
	int sockfd;
	struct in_addr addr;
	struct sockaddr_in local_addr;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){ /* create socket object (IPv4,UDP) */
		perror("socket");
		exit(1);
	}

	bzero(&local_addr, sizeof(local_addr));
	local_addr.sin_family = AF_INET; /* addrress family */
	local_addr.sin_port = htons(SRC_RECV_PORT); /* port number */
	if (inet_aton(SRC_ADDRESS, &addr) == 0) {
		perror("inet_aton");
		exit(1);	
	}
	local_addr.sin_addr = addr; /* IPv4 address */
	if (bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
		perror("bind");
		exit(1);
	}

	return sockfd;
}

void close_socket(int sockfd)
{
	if (close(sockfd) < 0) {
		perror("close");
		exit(1);
	}
}

void print_packet(char *packet, int packet_len)
{
	int i;

	for (i=0; i<packet_len; i++) {
		printf("%02x  ", packet[i] & 0xff);
		if ((i + 1) % 16 == 0) {
			printf ("\n");
		}
	}
	printf ("\n");
}

void sig_int(int signo)
{
	printf ("interrupt!!!\n");
}
#if 0
int isPCR(const char *packet, long *pclock)
{
	if (packet[0] != TRANSPORT_SYNC_BYTE) {
		printf ("Missing sync byte!\n");
		return 0;
	}

	adaptation_field_control = (packet[3] & 0x30) >> 4;
	if (adaptation_field_control != 2 && adaptation_field_control != 3) {
		// there's no adaptation_field
		return 0;
	}
	
	adaptation_field_length = packet[4];
	if (adaptation_field_length == 0) {
		return FALSE;
	}

	discontinuity_indicator = packet[5] & 0x80;
	pcrFlag = packet[5] & 0x10;
	if (pcrFlag == 0) {	 	
		return FALSE; // no PCR
	}

	// There's a PCR.  Get it, and the PID:
	pcrBaseHigh = (packet[6] << 24)|(packet[7] << 16)|(packet[8] << 8)|packet[9];
	clock = pcrBaseHigh / 45000;
	if ((packet[10] & 0x80) != 0) clock += 1/90000; // add in low-bit (if set) 
	pcrExt = ((packet[10] & 0x01) << 8) | packet[11];
	clock += pcrExt/27000000;
	pid = ((packet[1] & 0x1F) << 8) | packet[2];
	
	*pclock = clock;
	return 1;
}
#endif
int main(void)
{
	int sockfd, recvSockFd;
	struct sockaddr_in src_addr;
	struct sockaddr_in dest_addr;
	struct in_addr addr;
	char buffer[PACKET_LEN];
	FILE* fd;
	struct timeval tv;
	int n, i, counter = 0, Satis = 0;
	char recvBuff[1500];

//	if (signal(SIGINT, sig_int) == SIG_ERR) {
//		perror("signal");
//	}

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){ /* create socket object (IPv4,UDP) */
		perror("socket");
		exit(1);
	}

	bzero(&src_addr, sizeof(src_addr));
	src_addr.sin_family = AF_INET; /* addrress family */
	src_addr.sin_port = htons(SRC_SEND_PORT); /* port number */
	if (inet_aton(SRC_ADDRESS, &addr) == 0) {
		perror("inet_aton");
		exit(1);	
	}
	src_addr.sin_addr = addr; /* IPv4 address */
	if (bind(sockfd, (struct sockaddr*)&src_addr, sizeof(src_addr)) < 0) {
		perror("connect");
		exit(1);
	}
	
	bzero(&dest_addr, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET; /* addrress family */
	dest_addr.sin_port = htons(DEST_PORT); /* port number */
	if (inet_aton(DEST_ADDRESS, &addr) == 0) {
		perror("inet_aton");
		exit(1);	
	}
	dest_addr.sin_addr = addr; /* IPv4 address */	
	
	{	
		char plPathName[32];
		int lInterVal;
		int lStartPos;
		int lChar;
		int lCathBytes;

		recvSockFd = open_recv_socket();
		while(1) {
			printf ("Please Input [Receive Frame Size, CathBytes, StartPos, Interval]: ");
			scanf("%d, %d, %d, %d", &lChar, &lCathBytes, &lStartPos, &lInterVal);
			if (lChar == 0) continue;
			counter = 0;
			gettimeofday(&tv, NULL);
			sprintf (plPathName, SAVE_PATH"%d.pcm", tv.tv_sec);
			if ((fd = fopen(plPathName, "w+")) == NULL) {
				perror("fopen");
				exit(1);
			} 
			printf ("[****] create file : %s\n", plPathName);
			while(1) {
				if ((n = recv(recvSockFd, recvBuff, sizeof(recvBuff), MSG_TRUNC)) > 0)
				{
					counter ++;
					for (i = lCathBytes * lStartPos; i<n; i = i + lCathBytes * (lInterVal + 1))
						fwrite(&recvBuff[i], 1, lCathBytes, fd);
					if (counter % 10 == 0)
						printf ("receive data packet: %d \n", counter);
					if (counter % lChar == 0)
					{
						fclose(fd);
						break;
					}
	
				}
			}
		}
	}
/*	while (1) {
		if ((fd = fopen(TS_PATH, "r")) == NULL) {
			perror("fopen");
			exit(1);
		}
		fseek(fd, 0, SEEK_SET);
		while (feof(fd) == 0) {
			fread(buffer, sizeof(char), PACKET_LEN, fd);
			print_packet(buffer, PACKET_LEN);
			sendto (sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr_in));
		}
		fclose (fd);
		sleep(10);
	}
*/
//	while ((n = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
//		//print_packet(buffer, PACKET_LEN);
//		if (sendto (sockfd, buffer, n, 0, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr_in)) != n) {
//			perror("sendto");
//			exit(1);
//		}
//	}

	return 0;
}
