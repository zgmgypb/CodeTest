/*
[开发记录]

2016-06-24
1 创建工程用于管理font-print源码和工程
2 下载字库文件加入
3 创建初始版本，实现的功能：打印全部英文和大部分汉字，可实现自定义元素指定、字体大小指定、
	显示颜色指定
4 
*/

#include "font_print.h"
#include "hzk_index.h"
#include "hzk12.h"
#include "hzk14.h"
#include "hzk16.h"
#include "hzk20.h"
#include "hzk24.h"

/* 打印颜色定义 */
#define COLOR_NONE          "\033[0m"  
#define COLOR_RED           "\033[0;31m"  
#define COLOR_LIGHT_RED     "\033[1;31m"  
#define COLOR_GREEN         "\033[0;32m"  
#define COLOR_LIGHT_GREEN   "\033[1;32m"  
#define COLOR_BLUE          "\033[0;34m"  
#define COLOR_LIGHT_BLUE    "\033[1;34m"  
#define COLOR_DARY_GRAY     "\033[1;30m"  
#define COLOR_CYAN          "\033[0;36m"  
#define COLOR_LIGHT_CYAN    "\033[1;36m"  
#define COLOR_PURPLE        "\033[0;35m"  
#define COLOR_LIGHT_PURPLE "\033[1;35m"  
#define COLOR_BROWN         "\033[0;33m"  
#define COLOR_YELLOW        "\033[1;33m"  
#define COLOR_LIGHT_GRAY    "\033[0;37m"  
#define COLOR_WHITE         "\033[1;37m"  

static const struct
{
	int m_ColorIndex;
	char *m_pColorStr;
}sc_ColorMap[FP_COLOR_NUM] = 
{
	{FP_COLOR_AUTO, NULL},
	{FP_COLOR_RED, COLOR_RED},
	{FP_COLOR_LIGHT_RED, COLOR_LIGHT_RED},
	{FP_COLOR_GREEN, COLOR_GREEN},
	{FP_COLOR_LIGHT_GREEN, COLOR_LIGHT_GREEN},
	{FP_COLOR_BLUE, COLOR_BLUE},
	{FP_COLOR_LIGHT_BLUE, COLOR_LIGHT_BLUE},
	{FP_COLOR_DARY_GRAY, COLOR_DARY_GRAY},
	{FP_COLOR_CYAN, COLOR_CYAN},
	{FP_COLOR_LIGHT_CYAN, COLOR_LIGHT_CYAN},
	{FP_COLOR_PURPLE, COLOR_PURPLE},
	{FP_COLOR_LIGHT_PURPLE, COLOR_LIGHT_PURPLE},
	{FP_COLOR_BROWN, COLOR_BROWN},
	{FP_COLOR_YELLOW, COLOR_YELLOW},
	{FP_COLOR_LIGHT_GRAY, COLOR_LIGHT_GRAY},
	{FP_COLOR_WHITE, COLOR_WHITE}
};

static const struct
{
	int m_FontSize;
	int m_Rows;
	const struct font_struct *m_pFontStruct;
}sc_SizeMap[FP_FS_NUM] = 
{
	{FP_FS_12, 12, GUI_FontHZSong_12_CharInfo},
	{FP_FS_14, 14, GUI_FontHZSong_14_CharInfo},
	{FP_FS_16, 16, GUI_FontHZSong_16_CharInfo},
	{FP_FS_20, 20, GUI_FontHZSong_20_CharInfo},
	{FP_FS_24, 24, GUI_FontHZSong_24_CharInfo}
};

void FP_PrintString(char *pString, char Meta, unsigned int WordSpace, FP_FontSize FontSize, FP_FontColor FontColor)
{
	int i, j, k;
	int lRows = 0;
	char *plStr = NULL;
	const struct font_struct * plFontStruct = NULL;

	if (pString == NULL)
		return;

	if (Meta < 0x20)
	{
		Meta = '*'; /* 默认 */
	}

	if (FontSize >= FP_FS_NUM)
	{
		FontSize = FP_FS_12;
	}

	/* 设置颜色 */
	if ((FontColor != FP_COLOR_AUTO) && (FontColor < FP_COLOR_NUM))
	{
		for (i=0; i<FP_COLOR_NUM; i++)
		{
			if (FontColor == sc_ColorMap[i].m_ColorIndex)
				printf (sc_ColorMap[i].m_pColorStr);
		}
	}

	for (i=0; i<FP_FS_NUM; i++)
	{
		if (FontSize == sc_SizeMap[i].m_FontSize)
		{
			lRows = sc_SizeMap[i].m_Rows;
			plFontStruct = sc_SizeMap[i].m_pFontStruct;
		}
	}

	for (i=0; i<lRows; i++)
	{
		plStr = pString;
		while (*plStr != '\0')
		{
			for (j=0; j<plFontStruct[*plStr - 0x20].m_Cols; j++)
			{
				if (plFontStruct[*plStr - 0x20].m_pFontArray[i * plFontStruct[*plStr - 0x20].m_Bytes + j / 8] 
				& (0x80 >> (j % 8)))
					putchar (Meta);
				else
					putchar (' ');
			}	

			plStr++;
			for (k=0; k<WordSpace; k++)
			{
				putchar (' ');
			}
		}
		printf ("\n");
	}

	/* 清除颜色 */
	if ((FontColor != FP_COLOR_AUTO) && (FontColor < FP_COLOR_NUM))
	{
		printf (COLOR_NONE);
	}
}
/* EOF */
