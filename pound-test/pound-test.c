/*
 * # 单井号，连接的部分转为字符串
 * ## 双井号，进行标识符连接
 * */
#include <stdio.h>
#include <stdlib.h>

#define CONTACT(a, b) (a##b)
#define STR_CONTACT(a) ("string:"#a)
#define DISPLAY_PRINT(a, conversion) printf(#a" = %"#conversion"\n", (a))
int main(int argc, char **argv)
{
	int a = -1;
	unsigned b = 1;
	long long c = 10000;
	long long d = -10000;
	float f = 100.555;
	char *str = "hahahhahah";

	DISPLAY_PRINT(-1, d);	
	DISPLAY_PRINT(1, u);	
	DISPLAY_PRINT(10000, lld);	
	DISPLAY_PRINT(-10000, lld);	
	DISPLAY_PRINT(3 + 2, d);	
	DISPLAY_PRINT("hello world", s);	
	DISPLAY_PRINT(a, d);	
	DISPLAY_PRINT(b, u);	
	DISPLAY_PRINT(c, lld);	
	DISPLAY_PRINT(d, lld);	
	DISPLAY_PRINT(f, f);	
	DISPLAY_PRINT(str, s);	
}
