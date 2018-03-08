#ifndef WAVEFILE_H
#define WAVEFILE_H
#include <stdio.h>
 #include "lib-mw-capture.h"

#pragma pack(push)
#pragma pack(1)
typedef struct {
    WORD 	wFormatTag; //0x0001
    WORD 	wChannels;
    DWORD	dwSamplesPerSec;
    DWORD	dwAvgBytesPerSec;
    WORD	wBlockAlign;
    WORD	wBitsPerSample;
}PCMWAVEFORMAT;

typedef struct {
    DWORD dwRiffFlag; // 'RIFF'
    DWORD dwFileSize;
    DWORD dwWaveFlag; // 'WAVE'
    DWORD dwFmtFlag;  // 'fmt'
    DWORD dwFmtSize;
    PCMWAVEFORMAT pcmFormat;
    DWORD dwDataFlag; // 'data'
    DWORD dwDataSize;
}WAVE_FILE_HEADER ;

static FILE *g_pFile = NULL;
static WAVE_FILE_HEADER g_wfHeader = {0};

#define FOURCC(ch0, ch1, ch2, ch3)											\
        ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |							\
        ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24))


static BOOL WaveFileInit(const char * pszFilePath, int nSamplesPerSec, int nChannels, WORD wBitsPerSample)
    {
        g_wfHeader.dwRiffFlag = FOURCC('R','I','F','F'); //'FFIR' FOURCC('F', 'F', 'I','R')
        g_wfHeader.dwFileSize = sizeof(WAVE_FILE_HEADER) - 12;

        g_wfHeader.dwWaveFlag = FOURCC('W','A','V','E');//'EVAW' FOURCC('E','V','A','W')
        g_wfHeader.dwFmtFlag = FOURCC('f','m','t',' ');//' tmf' FOURCC(' ','t','m','f')
        g_wfHeader.dwFmtSize = sizeof(PCMWAVEFORMAT);

        g_wfHeader.pcmFormat.wBitsPerSample = wBitsPerSample;
        g_wfHeader.pcmFormat.wFormatTag = 0x0001;
        g_wfHeader.pcmFormat.wChannels = nChannels;
        g_wfHeader.pcmFormat.dwSamplesPerSec = nSamplesPerSec;
        g_wfHeader.pcmFormat.wBlockAlign = g_wfHeader.pcmFormat.wBitsPerSample * nChannels / 8;
        g_wfHeader.pcmFormat.dwAvgBytesPerSec =  g_wfHeader.pcmFormat.wBlockAlign * nSamplesPerSec;

        g_wfHeader.dwDataFlag = FOURCC('d','a','t','a');//'atad' FOURCC('a','t','a','d')
        g_wfHeader.dwDataSize = 0;

        //printf("size of header: %d\n", sizeof(WAVE_FILE_HEADER));

        g_pFile =fopen(pszFilePath, "wb");
        if (NULL == g_pFile)
            return FALSE;

        if (fwrite(&g_wfHeader, sizeof(WAVE_FILE_HEADER), 1, g_pFile) != 1) {
            fclose(g_pFile);
            return FALSE;
        }
        return TRUE;
    }



static void WaveFileExit()
{
    if (g_pFile) {
        g_wfHeader.dwFileSize = g_wfHeader.dwDataSize + sizeof(WAVE_FILE_HEADER) - 12;

        fseek(g_pFile, 0, SEEK_SET);
        fwrite(&g_wfHeader, sizeof(g_wfHeader), 1, g_pFile);

        fclose(g_pFile);
        g_pFile = NULL;
    }
}

static BOOL WaveFileWrite(const BYTE * pData, int nSize)
{
    if (fwrite(pData, nSize,1,  g_pFile) != 1)
        return FALSE;

    g_wfHeader.dwDataSize += nSize;
    return TRUE;
}

static BOOL WaveFileIsOpen() {
        return (g_pFile != NULL);
    }

#pragma pack(pop)
#endif // WAVEFILE_H
