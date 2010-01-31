/*********************************************************************************
 * Name:	BitStream Display
 * Desc:	This function will take a buffer and display it as a bit stream.
 * 		The length of the display is the nubmer of bytes in the buffer.
 * 		The flag gBigEnd or gLittleEnd will define if the low bit or the
 * 		high bit will be displayed first.
 *
 * 		first version will write to stdout.
 *
 *
 *********************************************************************************/

#ifdef _WIN32
	#include <io.h>
	#define	STDOUT	_fileno(stdout)
#else
	#include <unistd.h>
	#define	STDOUT	fileno(stdout)
#endif

#include <stdio.h>
#include <ctype.h>

#include "ConvTools.h"

extern unsigned int	gBigEndian;

void	BitStreamDisplay(unsigned char *buffer,int bytesRead,int BigEndian,int position);

bool	BinDump(int infile,int numLines)
{
	int				count,filePos,lineCount,bytesRead;
	bool			eof(false);
	unsigned char 	buffer[8];

	for(lineCount=0;lineCount<numLines && !eof;lineCount++)
	{
		filePos = lseek(infile,0,SEEK_CUR);

		/* we read 8 bytes across the page */
		if ((bytesRead = read(infile,buffer,8)) != 8)
			eof = true;
		
		BitStreamDisplay((unsigned char*)buffer,bytesRead,gBigEndian,filePos);
	}

	return eof;
}

void	BitStreamDisplay(unsigned char *buffer,int length,int bigEndian,int position)
{
	int 			i,j,bytenum=0,linepos=9,count;
	unsigned char	byte;
	unsigned char	line[80];
	
	if (buffer != NULL)
	{
		for (i=0;i<length;i++)
		{
			byte = buffer[i];
			
			for (j=0;j<8;j++)
			{
				if (bigEndian)
				{
					if (byte & 0x80)
						line[linepos++] = '1';
					else
						line[linepos++] = '0';

					byte <<= 1;
				}
				else
				{	
					if (byte & 0x01)
						line[linepos++] = '1';
					else
						line[linepos++] = '0';

					byte >>= 1;
				}
			}

			if (bytenum == 7)
			{
				*((int*)&line[0]) = 0x20202020;
				*((int*)&line[4]) = 0x30302020;

				if (bigEndian)
					line[8] = '>';
				else
					line[8] = '<';
				
				
				for (count=6;count>-1 && position != 0;count -= 2)
				{
					GenUpperHex((position & 0xff),&line[count],&line[count+1]);
					position >>= 8;
				}

				line[80] = '\0';
				printf("%s\n",line);
				bytenum = 0;
			}else{
				line[linepos++] = ' ';
				bytenum++;
			}
		}
	}
}
	
