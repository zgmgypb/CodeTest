/* 
 * ** showipandprot.c  
 * */  
#include <stdio.h>  
#include <string.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netdb.h>  
#include <arpa/inet.h>  
#include <netinet/in.h>  

int main(int argc, char *argv)
{
	struct addrinfo hints, *res, *p;
	int status;
	char ipstr[INET6_ADDRSTRLEN];
	uint16_t port;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = 0;

	if ((status = getaddrinfo("127.0.0.1", "80000", &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 2;
	}

	printf("IP address for :\n\n");
	for (p = res; p != NULL; p = p->ai_next) {
		void *addr;
		char *ipver;

		if (p->ai_family == AF_INET) {
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;

			addr = &(ipv4->sin_addr);
			ipver = "IPV4";
			port = ntohs(ipv4->sin_port);
		} else {
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			addr = &(ipv6->sin6_addr);
			ipver = "IPV6";
			port = ntohs(ipv6->sin6_port);
		}
		
		inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
		printf("%s: %s, port is %u\n", ipver, ipstr, port);
	}

	freeaddrinfo(res);

	return 0;
}
