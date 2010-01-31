/*--------------------------------------------------------------------------------*
 * Name:	DisplayHexLine
 * Description:
 *
 * This function will take in s buffer pointer and display the contents of the 
 * buffer to the screen. It will display 16 bytes in the format of choice.
 *
 * Author:	Peter Antoine.
 * Date:	28th November 2002
 *--------------------------------------------------------------------------------*/

#include <stdio.h>
#include <ctype.h>
#include <memory.h>

#include "ConvTools.h"

void	DisplayHexLines(unsigned char* inBuffer,int position,int bytesRead,int width)
{
	int				number,count;
	unsigned char	line[80];
	unsigned char	*buffer,*hexybit;
			
	buffer = line + 60;
	hexybit = line + 11;
	line[76] = '\0';
	memset(line,' ',74);

	switch(width)
	{
		case 0:		/* byte wide */
			for (count=0;count<bytesRead;count++)
			{
				GenUpperHex(inBuffer[count],&hexybit[count*3],&hexybit[count*3+1]);
			}
			break;

		case 1:		/* word wide */
			number = (bytesRead + 1) >> 1;

			for (count=0;count<number;count++)
			{
				GenHexWord(*((unsigned short*)&inBuffer[count*2]),&hexybit[count*6]);
			}
			break;

		case 2:		/* dword wide */
			number = (bytesRead + 3) >> 2;
			
			for (count=0;count<number;count++)
			{
				GenHexDWord(*((unsigned int*)&inBuffer[count*4]),&hexybit[count*10]);
			}
			break;
	}

	/* remove all the non-prints from the texty bit */
	for (count=0;count<bytesRead;count++)
		if (isprint(inBuffer[count]))
			buffer[count] = inBuffer[count];
		else
			buffer[count] = '.';

	for (count=bytesRead;count<16;count++)
		buffer[count] = ' ';

	/* generate line offset */
	*((int*)&line[0]) = 0x20202020;
	*((int*)&line[4]) = 0x30302020;
	line[8] = '|';
	
	for (count=6;count>-1 && position != 0;count -= 2)
	{
		GenUpperHex((char)(position & 0xff),&line[count],&line[count+1]);
		position >>= 8;
	}

	printf("%s\n",line);
}

