#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#if 0
int main(int argc, char *argv[])
{
	struct servent *p;

	printf("======getservent!\n");
	while (p = getservent()) {
		printf("%s\t%d/%s\t", p->s_name, ntohs(p->s_port), p->s_proto);	
		while(*p->s_aliases) {
			printf("%s ", *p->s_aliases);
			p->s_aliases += 1;
		}
		printf("\n");
	}
}
#endif

int main(int argc, char *argv[])
{
	struct servent *p;

//	printf("======getserbyname!\n");
//	argv++;
//	while (*argv && *(argv + 1)) {
//		if (p = getservbyname(*argv, *(argv + 1))) {
//			printf("%s\t%d/%s\t", p->s_name, ntohs(p->s_port), p->s_proto);	
//			while(*p->s_aliases) {
//				printf("%s ", *p->s_aliases);
//				p->s_aliases += 1;
//			}
//			printf("\n");
//		}
//		argv += 2;
//	}

	printf("======getservbyport!\n");
	argv++;
	while (*argv && *(argv + 1)) {
		printf ("%d %s\n", atoi(*argv), *(argv+1));
		if (p = getservbyport(htons(atoi(*argv)), *(argv + 1))) {
			printf("%s\t%d/%s\t", p->s_name, ntohs(p->s_port), p->s_proto);	
			while(*p->s_aliases) {
				printf("%s ", *p->s_aliases);
				p->s_aliases += 1;
			}
			printf("\n");
		}
		argv += 2;
	}
}
