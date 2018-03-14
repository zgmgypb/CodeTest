#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

/* 服务器地址和端口 */
#define SERVER_IP (0x78787835) 
#define SERVER_PORT (9001)
#define SERVER_MAX_LISTENQ (16)
#define TCP_MTU (1500)

static int s_SocketFd;
#define MTU (1500)

static char const* allowedCommandNames
= "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE";
static char *servName = "Gospell Rtsp SERVER";

void handleCmd_OPTIONS(char *fResponseBuffer)
{
	snprintf((char*)fResponseBuffer, 256,
			"RTSP/1.0 200 OK\r\n"
			"CSeq: 2\r\n"
			"Public: %s\r\n\r\n",
			allowedCommandNames);
}

void handleCmd_DESCRIBE(char *fResponseBuffer)
{
	snprintf((char*)fResponseBuffer, 256,
			"RTSP/1.0 200 OK\r\n"
			"CSeq: 3\r\n"
			"Content-Base: 127.0.0.1\r\n"
			"Content-Type: application/sdp\r\n"
			"m=video 49170 RTP/AVP 98\r\n"
			"a=rtpmap:98 H264/90000\r\n"
			"a=fmtp:98 profile-level-id=42A01E;"
			"sprop-parameter-sets=Z0IACpZTBYmI,aMljiA==\r\n\r\n");
}
void handleCmd_SETUP(char *fResponseBuffer)
{
	snprintf((char*)fResponseBuffer,256,
			"RTSP/1.0 200 OK\r\n"
			"CSeq: 4\r\n"
			"Transport: RTP/AVP;unicast;port=8980-8981\r\n"
			"Session: 1\r\n\r\n");
}

void handleCmd_PLAY(char *fResponseBuffer)
{
	snprintf((char*)fResponseBuffer,256,
			"RTSP/1.0 200 OK\r\n"
			"CSeq: 5\r\n"
			"Range: npt=0.000-410.134\r\n"
			"User-Agent: yuanzhenhai\r\n\r\n");
}

void handleCmd_bad(char *fResponseBuffer)
{
	snprintf((char*)fResponseBuffer, 256,
			"RTSP/1.0 400 Bad Request\r\nAllow: 8\r\n\r\n",
			allowedCommandNames);
}

#define TS_FILE "./test.ts"
#define BITRATE 50000 // kbps
#define RTP_PACKET_LEN (12)

static void ENC_3531AConstructRtp(char *pDataBuf)
{
	static unsigned RtpSn = 0;
	char plRtp[RTP_PACKET_LEN] = {0x80, 0xa1, 0x00, 0x00, 0x0a, 0xa2, 0xe9, 0x0e,
		0x54, 0x48, 0x00, 0x10};

	memcpy(pDataBuf, plRtp, RTP_PACKET_LEN);
	RtpSn++;
	pDataBuf[2] = (RtpSn >> 8) & 0xFF;
	pDataBuf[3] = RtpSn & 0xFF;
}

int rtp_send_file(int sockfd)
{
	FILE *pfd;
	char pbuf[MTU];
	int ret;
	struct timeval time_val;

	pfd = fopen(TS_FILE, "r");
	if (pfd) {
		while (fread(&pbuf[RTP_PACKET_LEN], 1, 1316, pfd)) {
			ENC_3531AConstructRtp(pbuf);
			send(sockfd, pbuf, 1316 + RTP_PACKET_LEN, MSG_DONTWAIT);
 			time_val.tv_sec = 0;
			time_val.tv_usec = (1316 * 8 * 1000) / BITRATE;
			select(0, NULL, NULL, NULL, &time_val);
		}
	}
	
	return 0;	
}

void rtsp_response(int sockfd, char *pbuf)
{
	int send_number;
	char fResponseBuffer[256];

	if(strstr(pbuf, "OPTIONS"))
	{
		handleCmd_OPTIONS(fResponseBuffer);
		send_number = send(sockfd, fResponseBuffer, strlen(fResponseBuffer), MSG_DONTWAIT);
		if(-1 == send_number)
		{
			perror("option sendto error");
		}
		else
		{
			printf("fResponseBuffer:%s\n",fResponseBuffer);
			printf("option send number = %d\n",send_number);
			printf("option send successful!\n");
		}

	}
	else if(strstr(pbuf, "DESCRIBE"))
	{
		handleCmd_DESCRIBE(fResponseBuffer);
		send_number = send(sockfd, fResponseBuffer, strlen(fResponseBuffer), MSG_DONTWAIT);
		if(-1 == send_number)
		{
			perror("describe sendto error");
		}
		else
		{
			printf("fResponseBuffer:%s\n",fResponseBuffer);
			printf("describe sendto number = %d\n",send_number);
			printf("descrebe sendto successful!\n");
		}
	}
	else if(strstr(pbuf, "SETUP"))
	{
		printf("setup!\n");
		handleCmd_SETUP(fResponseBuffer);
		send_number = send(sockfd, fResponseBuffer, strlen(fResponseBuffer), MSG_DONTWAIT);
		if(send_number == -1)
		{
			perror("setup sendto error");
		}
		else
		{
			printf("fResponseBuffer:%s\n",fResponseBuffer);
			printf("setup sendto number = %d\n",send_number);
			printf("setup sendto successful!\n");
		}

	}
	else if(strstr(pbuf, "PLAY"))
	{
		printf("play!\n");
		handleCmd_PLAY(fResponseBuffer);
		send_number = send(sockfd, fResponseBuffer, strlen(fResponseBuffer), MSG_DONTWAIT);
		if(-1 == send_number)
		{
			perror("play sendto error");
		}
		else
		{
			int rtp_flag = rtp_send_file(sockfd);
			printf("rtp_flag = %d",rtp_flag);
			printf("fResponseBuffer:%s\n",fResponseBuffer);
			printf("play sendto number = %d\n",send_number);
			printf("play sendto successful!\n");
		}

	}
	else
	{
		printf("bad request!\n");
		handleCmd_bad(fResponseBuffer);
		send_number = send(sockfd, fResponseBuffer, strlen(fResponseBuffer), MSG_DONTWAIT);
		if(-1 == send_number)
		{
			perror("bad reqest sendto error");
		}
		else
		{
			printf("fResponseBuffer:%s\n",fResponseBuffer);
			printf("bad request sendto number = %d\n",send_number);
			printf("bad reqrest successful!\n");
		}

	}
}

void rtsp_process(int sockfd, struct sockaddr_in *psockaddr)
{
	char pbuf[MTU];

	while(recv(sockfd, pbuf, MTU, MSG_WAITALL)) {
		printf("tcp recv: %s\n", pbuf);		
		rtsp_response(sockfd, pbuf);
	}	
}

int main(int argc, char *argv[])
{
	struct sockaddr_in lServerAddr;
	struct sockaddr_in lCliAddr;
	int lReUse = 1; // 可重用地址

	//signal(SIGINT, SigHandler);
	//signal(SIGQUIT, SigHandler);
	//signal(SIGABRT, SigHandler);
	//signal(SIGCLD, SigCldHandler);

	if ((s_SocketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket error");
	}
	if (setsockopt(s_SocketFd, SOL_SOCKET, SO_REUSEADDR, &lReUse, sizeof(int)) < 0) { // 为了可以立即重启使用端口
		close(s_SocketFd);
		perror("setsockopt error");
	}

	bzero(&lServerAddr, sizeof(lServerAddr));
	lServerAddr.sin_family = AF_INET;
	lServerAddr.sin_addr.s_addr = htonl(SERVER_IP);
	lServerAddr.sin_port = htons(SERVER_PORT);
	if (bind(s_SocketFd, (struct sockaddr *)&lServerAddr, sizeof(struct sockaddr_in)) < 0) {
		perror("bind error");
	}

	if (listen(s_SocketFd, SERVER_MAX_LISTENQ) < 0) {
		perror("listen error");
	}

	while (1) {
		int lConnFd;
		socklen_t lAlen = sizeof(lCliAddr);

		lConnFd = accept(s_SocketFd, (struct sockaddr *)&lCliAddr, &lAlen);
		if (lConnFd < 0) {
			perror("accept error");
		}
		else {
			pid_t lPid;

			printf("Connect Client is: %s-%d\n", inet_ntoa(lCliAddr.sin_addr), ntohs(lCliAddr.sin_port));
			rtsp_process(lConnFd, &lCliAddr);

			//lPid = fork();
			//if (lPid < 0) {
			//	perror("fork error");
			//}
			//else if (lPid == 0) {
			//	close(s_SocketFd);
			//	//GS_TCP_SERV_ClientComm(lConnFd, &lCliAddr);
			//	exit(EXIT_FAILURE);
			//}
			//else {
			//	printf("New Process Is: PID[%d]\n", lPid);
			//	close(lConnFd);
			//}
		}
	}

	return 0;
}
