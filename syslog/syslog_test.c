#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

int main(int argc, char *argv[])
{
	openlog("[LOG_TEST]", LOG_PERROR, LOG_SYSLOG);
	syslog(LOG_SYSLOG | LOG_CRIT, "syslog test!!\n");
	closelog();
}
