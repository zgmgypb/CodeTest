#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void strdup_test(void)
{
	char *str="hello world!";

	printf("strdup(\"hello\"):%s\n", strdup("hello"));
}

char *str_invert(char *str)
{
	char *p;

	if (!str) {
		return NULL;
	}

	p = (char *)malloc(strlen(str) + 1);
	p = &p[strlen(str)];
	*p = '\0';
	while (*str != '\0') {
		*(--p) = *(str++);
	}

	return p;
}

unsigned char bit_reverse(unsigned char c)
{
	unsigned char ret = 0;
	int i;

	for (i = 0; i < 8; i++) {
		ret = (ret << 1) | ((c >> i) & 0x01);
	}
	
	return ret;
}

/* 写一个程序, 要求功能：求出用1，2，5这三个数不同个数组合的和为100的组合个数。
 * 如：100个1是一个组合，5个1加19个5是一个组合。。。。
 * */
void count_xyz(void)
{
	int x,y,z;
	int sum = 100;
	int count = 0;

	for (x = 0; x <= sum / 5; x++)
		for (y = 0; y <= (sum - x * 5) / 2; y++) {
			z = sum - x * 5 - 2 * y;
			printf("[5]: %d [2]: %d [1]: %d\n", x, y, z);
			count ++;
		}
	printf("statistics: %d\n", count);
}

void Matrix(int m,int n)  //顺时针
{
	int i,j,a=1;
	int s[100][100];
	int small = (m<n)?m:n;
	int k=small/2;

	for(i=0;i<k;i++)
	{
		for(j=i;j<n-i-1;j++)
			s[i][j]=a++;
		for(j=i;j<m-i-1;j++)
			s[j][n-i-1]=a++;
		for(j=n-i-1;j>i;j--)
			s[m-i-1][j]=a++;
		for(j=m-i-1;j>i;j--)
			s[j][i]=a++;
	}
	if(small & 1)
	{
		if(m<n)
			for(i=k;i<n-k;++i)
				s[k][i]=a++;
		else
			for(i=k;i<m-k;++i)
				s[i][k]=a++;
	}
	for(i=0;i<m;i++)
	{
		for(j=0;j<n;j++)
			printf("%02d ",s[i][j]);
		printf("\n");
	}
}

void findMaxNumOfStr()  
{
	char input[100];  
	char output[100] = {0};
	int  count = 0, maxlen = 0, i = 0;
	char *in=input, *out=output,*temp=NULL,*final=NULL;

	printf("Please input string(length under 100):\n");
	scanf("%s", input);
	printf("Input string is %s\n", input);

	while(*in!='\0')
	{
		if(*in>='0'&&*in<='9')
		{
			count=0;
			temp=in;
			for(;(*in>='0')&&(*in<='9');in++)
				count++;
			if (maxlen<count)
			{
				maxlen=count;
				final=temp;   
			} 
		} 
		in++;
	} 

	for(i=0; i<maxlen; i++)
		*(out++) = *(final++);
	*out='\0';

	printf("Maxlen is %d\n", maxlen);
	printf("Output is %s\n", output);
}

void selfAddTest() 
{ 
	char *p1="name"; 
	char *p2; 
	char *p3;
	p2=(char*)malloc(20); 
	memset (p2, 0, 20); 
	p3 = p2;
	while(*p2++ = *p1++); 
	//while ((*p2++ = *p1++) != '\0');
	//while (*p2 = *p1) {
	//	p2++;
	//	p1++;
	//}
	printf("%s\n", p3); 
} 

int main(int argc, char *argv[])
{
#if 1
	selfAddTest();
#endif
//#error compile test
//#warning hahahaha
#if 0
	findMaxNumOfStr();
#endif

#if 0
	Matrix(10, 7);
#endif

//#define xx 1
#if xx /* 如果 xx 不存在，这里等同于 #if 0 */
	count_xyz();	
#endif
#if 0
	strdup_test();
#endif
#if 0
	int a = 5, b = 7, c;
	const int *d = &b;

	printf("7 bit reverse:%d\n", bit_reverse(7));
	printf("nihao str_invert:%s\n", str_invert("nihao"));
	{
		int i = 3;
	}

	{
		int j = 5;
	}
#endif
#if 0
	//*d = 200;
	printf("d=%p\n", d);
	d = &a;
	printf("d=%p\n", d);
	//*d = 20;
	printf("*d=%d\n", *d);

	c = a+++b;
	printf("a = %d, b = %d, c = %d\n", a, b, c);
	if ('\0') {
		printf("\'\\0\' = %d\n", '\0');
	}
	else {
		printf("NULL!! \'\\0\' = %d\n", '\0');
	}
#endif
}
