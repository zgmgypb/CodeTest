#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>

void timer_thread(sigval_t sigv)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	printf ("timer_id: 0x%x current time: %d s %d us\n", *(timer_t *)sigv.sival_ptr, tv.tv_sec, tv.tv_usec);
}

int main(int argc, char *argv[])
{
	timer_t timer_id;
	struct sigevent evp;
	struct itimerspec its;

	if (!argv[1])
		exit(0);

	memset(&evp, 0, sizeof(struct sigevent));
	evp.sigev_notify = SIGEV_THREAD;
	evp.sigev_notify_function = timer_thread;
	evp.sigev_value.sival_ptr = &timer_id;

	if (timer_create(CLOCK_REALTIME, &evp, &timer_id) == -1)
	{
		perror("timer_create error");
		exit(-1);
	}
	printf ("timer create success: 0x%x\n", timer_id);
	printf ("timer interval is : %d ms\n", atoi(argv[1]));

	its.it_value.tv_sec = atoi(argv[1]) / 1000;
	its.it_value.tv_nsec = atoi(argv[1]) % 1000 * 1000000;
	its.it_interval.tv_sec = its.it_value.tv_sec;
	its.it_interval.tv_nsec = its.it_value.tv_nsec;
	if (timer_settime(timer_id, 0, &its, NULL) == -1)
	{
		perror("timer_settime error");
		exit(-1);
	}

	while (1) sleep(1);
	return 0;

}
