#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int Gs_GetProcessPIDByName(char *pName, long *pPIDBuf, int PIDBufLen)
{
	char plBuf[512], *plTmpBuf;
	FILE *plFP;
	int lRead;
	int lRet = 0;

	if (!pName)
	{
		return -1;
	}

	plTmpBuf = strtok(pName, " "); /* 只支持一个命令的 PID 查询 */
	if (!plTmpBuf)
		return -1;

	snprintf(plBuf, sizeof(plBuf), "pidof %s", plTmpBuf);
	plFP = popen(plBuf, "r"); /* 管道方式运行 pidof 命令 */
	if (!plFP)
		exit (-1);	

	if (plFP != NULL) 
	{
		lRead = fread(plBuf, sizeof(char), sizeof(plBuf) - 1, plFP);
		if (lRead > 1)
		{
			plBuf[lRead] = 0;

			plTmpBuf = strtok(plBuf, " ");
			while(plTmpBuf)
			{
				if (lRet < PIDBufLen)
				{
					pPIDBuf[lRet] = strtol(plTmpBuf, NULL, 10);
					lRet++;
				}
				else
					break;

				plTmpBuf = strtok(NULL, " ");
			}
		}
		pclose(plFP);
	}

	return lRet;
}

int main(int argc, char *args[])
{
	long pBuf[15];
	int i, j;

	i = Gs_GetProcessPIDByName(args[1], pBuf, 15);
	for (j=0; j<i; j++)
		printf ("%ld\n", pBuf[j]);
}
