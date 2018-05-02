#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

pthread_mutex_t mutex_1;
pthread_mutex_t mutex_2;

void *child1(void*arg)
{
	while(1) {
		pthread_mutex_lock(&mutex_1);
		sleep(3);
		pthread_mutex_lock(&mutex_2);
		printf("thread 1 get running\n");
		pthread_mutex_unlock(&mutex_2);
		pthread_mutex_unlock(&mutex_1);
		sleep(5);
	}
}

void *child2(void*arg)
{
	while(1) {
		pthread_mutex_lock(&mutex_2);
		pthread_mutex_lock(&mutex_1);
		printf("thread 2 get running\n");
		pthread_mutex_unlock(&mutex_1);
		pthread_mutex_unlock(&mutex_2);
		sleep(5);
	}
}

#define LONGLONG long long 
long long test_64ret(void)
{
	long a, b;

	a = 2147483640UL; 
	b = 5;
	return (LONGLONG)a * 3 + b;
}

#define join(x, y) x##y
int main(int argc,char *argv[])
{
//	pthread_t tid;
//
//	pthread_create(&tid, NULL, child1, NULL);
//	pthread_create(&tid, NULL, child2, NULL);
//
//	while (1);
	int hhh1=5;
	int i;
	long long xx;

	i = 1;
	printf("%d\n", join(hhh, 1));
	xx = test_64ret();
	printf("%lld\n", xx);

}
