/*--------------------------------------------------------------------------------*
 * Name:	DriveHexDump
 * 
 * This function will hex dump the drive from the current location for n lines.
 * It will also read forward on the disk from the current disk sector.
 *
 * Author:	Peter Antoine.
 * Date:	16th July 2001
 *--------------------------------------------------------------------------------*/

#ifdef _WIN32
	#include <io.h>
	#include "ScsiData.h"
	#define	STDOUT	_fileno(stdout)
#else
	#include <unistd.h>
	#define	STDOUT	fileno(stdout)
#endif

#include "look.h"
#include <stdio.h>
#include <ctype.h>


extern	unsigned long	gFileSize;
extern	unsigned int	gLastLine;
extern	unsigned long	gDiskSector;
extern 	unsigned char	gSectorBuffer[512];

void	DisplayHexLines(unsigned char* inBuffer,int position,int bytesRead,int width);

bool	DriveHexDump(int numLines,int width)
{
	int	count,filePos,lineCount,bytesRead;

	for (count=0;count<numLines && gDiskSector < gFileSize;count++)
	{
		if ((gLastLine % 32) == 0)
		{
			/* read the new sector */
			gLastLine = 0;

			if (!Read(gDiskSector,(char*)gSectorBuffer,3))
			{
				printf("Failed to read sector: %d (%x)\n",gDiskSector,gDiskSector);
				break;
			}else{
				printf("Sector: %d (%x)\n",gDiskSector,gDiskSector);
			}
			
			gDiskSector++;
		}
			
		DisplayHexLines(gSectorBuffer+gLastLine*16,(gDiskSector-1)*512+gLastLine*16,16,width);
		gLastLine++;
	}

	return (gFileSize<gDiskSector);
}
