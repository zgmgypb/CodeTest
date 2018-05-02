#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>

#define TSCTRL_GET_INFO_PSI_LOCK	0x00000001
#define TSCTRL_GET_INFO_VIDEO_LOCK	0x00000002
#define TSCTRL_GET_INFO_AUDIO_LOCK	0x00000004
#define TSCTRL_GET_INFO_PCR_LOCK	0x00000008
#define TSCTRL_GET_INFO_VIDEO_CC_ERR	0x00000010
#define TSCTRL_GET_INFO_AUDIO_CC_ERR	0x00000020
#define TSCTRL_GET_INFO_TS_OUTPUT	0x00000040
#define TSCTRL_GET_INFO_VIDEO_OUTPUT	0x00000080
#define TSCTRL_GET_INFO_AUDIO_OUTPUT	0x00000100
#define TSCTRL_GET_INFO_VIDEO_SYNTAX_ERROR	0x00000200
#define TSCTRL_GET_INFO_VIDEO_TIMESTAMP_ERROR	0x00000400
#define TSCTRL_GET_INFO_DISCONTINUOUS_TIMESTAMP	0x00000800
#define TSCTRL_ALL (TSCTRL_GET_INFO_PSI_LOCK | TSCTRL_GET_INFO_VIDEO_LOCK | TSCTRL_GET_INFO_AUDIO_LOCK | \
		TSCTRL_GET_INFO_PCR_LOCK | TSCTRL_GET_INFO_VIDEO_CC_ERR | TSCTRL_GET_INFO_AUDIO_CC_ERR | \
		TSCTRL_GET_INFO_TS_OUTPUT | TSCTRL_GET_INFO_VIDEO_OUTPUT | TSCTRL_GET_INFO_AUDIO_OUTPUT | \
		TSCTRL_GET_INFO_VIDEO_SYNTAX_ERROR | TSCTRL_GET_INFO_VIDEO_TIMESTAMP_ERROR | TSCTRL_GET_INFO_DISCONTINUOUS_TIMESTAMP)

unsigned int get_status_flag = 0;
void signal_handler(int sig)
{
	int status;

	if (sig == SIGCHLD)
	{
		if (wait(&status) > 0)
			printf ("child process terminated! return status = %d\n", status);
	}
}

void get_param(char *param, char *param_start, char *param_end)
{
	while (*param_end == ' ' && param_start != param_end) param_end --; /* 跳过后面的空格  */
	
	memcpy(param, param_start, param_end - param_start + 1);
	param[param_end - param_start + 1] = '\0';
}

void get_value(int *value, char *string)
{
	*value = strtol(string, NULL, 0);	
}

void parse_childparams(char *buffer, int size)
{
	int i = 0;
	char line[256] = {0};
	char param[128] = {0};
	int value;
	char *p;

	while (1)
	{
		while (isspace(*buffer))
			buffer ++;

		if (*buffer == '\0')
			break;

		i = 0;
		while (1)
		{
			if (*buffer == '\n' || *buffer == '\0' || i > 254)
			{
				line[i] = '\0';
				printf ("__Line__: %s\n", line);
				break;
			}
			else
				line[i++] = *buffer++;
		}
		p = strchr(line, '=');
		if (!p)
		{
			continue;
		}
		get_param(param, line, p - 1);
		if (!strcmp(param, "TS Input PSI Lock"))
		{
			get_value(&value, p + 1); /* 1 for '=' */
			printf ("%s = %d\n", param, value);
			get_status_flag |= TSCTRL_GET_INFO_PSI_LOCK;
		}
		else if (!strcmp(param, "TS Input Video Lock"))
		{
			get_value(&value, p + 1); /* 1 for '=' */
			printf ("%s = %d\n", param, value);
			get_status_flag |= TSCTRL_GET_INFO_VIDEO_LOCK;
	
		}
		else if (!strcmp(param, "TS Input Audio Lock"))
		{
			get_value(&value, p + 1); /* 1 for '=' */
			printf ("%s = %d\n", param, value);
			get_status_flag |= TSCTRL_GET_INFO_AUDIO_LOCK;
	
		}
		else if (!strcmp(param, "TS Input PCR Lock"))
		{
			get_value(&value, p + 1); /* 1 for '=' */
			printf ("%s = %d\n", param, value);
			get_status_flag |= TSCTRL_GET_INFO_PCR_LOCK;
		}
		else if (!strcmp(param, "TS Input Video CC Error"))
		{
			get_value(&value, p + 1); /* 1 for '=' */
			printf ("%s = %d\n", param, value);
			get_status_flag |= TSCTRL_GET_INFO_VIDEO_CC_ERR;
		}
		else if (!strcmp(param, "TS Input Audio CC Error"))
		{
	 		get_value(&value, p + 1); /* 1 for '=' */
			printf ("%s = %d\n", param, value);
			get_status_flag |= TSCTRL_GET_INFO_AUDIO_CC_ERR;
		}
		else if (!strcmp(param, "TS Output"))
		{
			if (strstr(p+1, "YES"))
				value = 1;
			else
				value = 0;
			printf ("%s = %d\n", param, value);
			get_status_flag |= TSCTRL_GET_INFO_TS_OUTPUT;
		}
		else if (!strcmp(param, "TS Video Syntax Error"))
		{
			get_value(&value, p + 1); /* 1 for '=' */
			printf ("%s = %d\n", param, value);
			get_status_flag |= TSCTRL_GET_INFO_VIDEO_SYNTAX_ERROR;
		}
		else if (!strcmp(param, "TS Video Timestamp Error"))
		{
			get_value(&value, p + 1); /* 1 for '=' */
			printf ("%s = %d\n", param, value);
			get_status_flag |= TSCTRL_GET_INFO_VIDEO_TIMESTAMP_ERROR;
		}
		else if (!strcmp(param, "TS Discontinuous Timestamp"))
		{
			get_value(&value, p + 1); /* 1 for '=' */
			printf ("%s = %d\n", param, value);
			get_status_flag |= TSCTRL_GET_INFO_DISCONTINUOUS_TIMESTAMP;
		}
		else if (!strcmp(param, "Video Output"))
		{
			if (strstr(p+1, "YES"))
				value = 1;
			else
				value = 0;
			printf ("%s = %d\n", param, value);
			get_status_flag |= TSCTRL_GET_INFO_VIDEO_OUTPUT;
		}
		else if (!strcmp(param, "Audio Output"))
		{
			if (strstr(p+1, "YES"))
				value = 1;
			else
				value = 0;
			printf ("%s = %d\n", param, value);
			get_status_flag |= TSCTRL_GET_INFO_AUDIO_OUTPUT;
		}
		else
		{
			printf ("Status Param:%s Unrecognized!\n", param);
		}
	}
	printf ("[%s][%d]\n", __FUNCTION__, __LINE__);
}

int main(int argc, char *argv[])
{
	pid_t pid = -1;
	int fd;
	int pipe_fd[2];
	char read_buffer[1024] = {0};
	int ret;

	signal(SIGCHLD, signal_handler);

	if (pipe(pipe_fd) < 0)
		perror("pipe failed!\n");


	pid = fork();
	if (pid > 0) /* parent process */
	{
		close(pipe_fd[1]);
		while (1)
		{
			if ((ret = read(pipe_fd[0], read_buffer, sizeof(read_buffer) - 1)) > 0)
			{
				read_buffer[ret] = '\0';
				printf ("read pipe: %s\n", read_buffer);
				parse_childparams(read_buffer, ret);
			}
			if (get_status_flag == TSCTRL_ALL)
			{
				get_status_flag = 0;
				printf ("__Get Status Complete__!\n");
			}
			sleep(1);
		}
	}
	else if (pid == 0) /* child process */
	{
		fd = open("/dev/null", O_RDWR);
		if (fd < 0)
		{
			fprintf (stderr, "open error!\n");
			exit (-1);
		}
		setvbuf (stdout, NULL, _IONBF, 0);
		dup2(pipe_fd[1], STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		close(pipe_fd[0]);
		close(pipe_fd[1]);
		close(fd);
		if (execl("./child_process", "./child_process", NULL) == -1)
			fprintf (stderr, "execl faild!\n");
		exit(-1);
	}
	else
	{
		fprintf (stderr, "fork failed! Errno: %d\n", errno);
	}

	return 0;
}

