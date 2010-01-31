/*--------------------------------------------------------------------------------*
 * Name:	FindString
 * Description:
 *
 * This function will find a string either in the file or on the disk. This 
 * function was cut out of look to make id easyier to read.
 *
 * Author:	Peter Antoine.
 * Date:	16th July 2001
 *
 *--------------------------------------------------------------------------------*/

#ifdef _WIN32
	#include <io.h>
	#define	STDOUT	_fileno(stdout)
	#include "ScsiData.h"
#else
	#include <unistd.h>
	#define	STDOUT	fileno(stdout)
#endif


#include <errno.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>

#include "look.h"

extern unsigned int	gDriveMode;
extern unsigned long	gDiskSector;
extern unsigned int	gLastLine;
extern unsigned int	gLastSearchPos;
	
static char	lgSearchBuffer[K_64];

bool	FindString(int infile,unsigned char* token,int tokenSize,bool fromStart,bool fromLastSearch)
{
	int	readOffset = 0,count1,bytesRead,filePos;
	int	count(0),currPos,offset,searchBlock(0);
	bool	stop(false),found(false),failed(false);
	
	if (!gDriveMode)
	{
		currPos = lseek(infile,0,SEEK_CUR);

		if (!fromStart)
		{
			if (fromLastSearch)
			{
				filePos = gLastSearchPos;
				currPos = filePos;
				lseek(infile,filePos,SEEK_SET);
			}else{
				filePos = currPos;
			}

			searchBlock = gDiskSector;
		}
	}
	
	stop = false;
	found = false;

	/* do the find here */
	for (offset = 0;!stop && !found ;offset++)
	{
		if (gDriveMode)
		{
			stop = !Read(searchBlock,lgSearchBuffer,K_64_IN_SECTORS);
			searchBlock += K_64_IN_SECTORS; 
			bytesRead = K_64;
		}else{
			if((bytesRead = read(infile,lgSearchBuffer,K_64)) != K_64)
				stop = true;
		}

		/* Do a cheap search --- There are better ways... */
		for (count=0;count<bytesRead && !found;count++)
		{
			if (lgSearchBuffer[count] == token[readOffset])
			{
				for (count1=count+1;count1<bytesRead && count1 < count+tokenSize && !failed;count1++)
				{
					if (lgSearchBuffer[count1] != token[readOffset+count1-count])
						failed = true;
				}

				if (!failed)
				{	
					if (count1 == bytesRead)
					{
						/* we have run out of the current buffer so start searching
						 * for the token from where we left off.
						 */
						readOffset = count1-count;
					}else{
						/* We have found it */
						found = true;

						if (gDriveMode)
						{
							gDiskSector = (searchBlock - K_64_IN_SECTORS) + count/512;
							gLastLine = 0;
						}else{
							filePos = (currPos + (offset*K_64) + count);
							gLastSearchPos = filePos+1;
							lseek(infile,filePos,SEEK_SET);
						}
					}
				}else{
					readOffset = 0;
				}

				failed = false;
			}
		}
	}

	return found;
}


bool	FindMaskedString(int infile,unsigned char* token,unsigned char* mask, int tokenSize,bool fromStart,bool fromLastSearch)
{
	int	readOffset = 0,count1,bytesRead,filePos;
	int	count(0),currPos,offset,searchBlock(0);
	bool	stop(false),found(false),failed(false);
	
	if (!gDriveMode)
		currPos = lseek(infile,0,SEEK_CUR);

	if (!fromStart)
	{
		if (fromLastSearch)
		{
			filePos = gLastSearchPos;
			currPos = filePos;
			lseek(infile,filePos,SEEK_SET);
		}else{
			filePos = currPos;
		}

		searchBlock = gDiskSector;
	}
	
	stop = false;
	found = false;

	/* save some cpu -- mask the token */
	for (count=0;count<tokenSize;count++)
		token[count] = token[count] & mask[count];
	
	/* do the find here */
	for (offset = 0;!stop && !found ;offset++)
	{
		if (gDriveMode)
		{
			stop = !Read(searchBlock,lgSearchBuffer,K_64_IN_SECTORS);
			searchBlock += K_64_IN_SECTORS; 
			bytesRead = K_64;
		}else{
			if((bytesRead = read(infile,lgSearchBuffer,K_64)) != K_64)
				stop = true;
		}

		/* Do a cheap search --- There are better ways... */
		for (count=0;count<bytesRead && !found;count++)
		{
			if ((lgSearchBuffer[count] & mask[readOffset]) == token[readOffset])
			{
				for (count1=count+1;count1<bytesRead && count1 < count+tokenSize && !failed;count1++)
				{
					if ((lgSearchBuffer[count1] & mask[readOffset+count1-count]) != token[readOffset+count1-count])
						failed = true;
				}

				if (!failed)
				{	
					if (count1 == bytesRead)
					{
						/* we have run out of the current buffer so start searching
						 * for the token from where we left off.
						 */
						readOffset = count1-count;
					}else{
						/* We have found it */
						found = true;

						if (gDriveMode)
						{
							gDiskSector = (searchBlock - K_64_IN_SECTORS) + count/512;
							gLastLine = 0;
						}else{
							filePos = (currPos + (offset*K_64) + count);
							gLastSearchPos = filePos+1;
							lseek(infile,filePos,SEEK_SET);
						}
					}
				}else{
					readOffset = 0;
				}

				failed = false;
			}
		}
	}

	return found;
}


/* this will fail if the token is greater than 64K, but if you can type that in then
 * you are a f**king genius and please send me how you managed to do than on a console.
 *
 * (and why you would do something that supid!)
 */
bool	FindBlockedMaskString(int infile,unsigned char* token,unsigned char* mask, int tokenSize,int block_size,bool fromStart)
{
	int		count,currPos,bytesRead;
	bool	found(false),file_end(false),failed(false);

	if (fromStart)
		/* goto the start of the file */
		currPos = lseek(infile,0,SEEK_SET);
	else{
		/* round the file position up to the next block */
		currPos = gLastSearchPos;
		currPos = (((currPos + block_size - 1) / block_size)+1) * block_size;
		currPos = lseek(infile,currPos,SEEK_SET);
	}

	/* save some cpu -- mask the token 
	 * should copy before amending but I know it is olny
	 * used once.
	 */
	for (count=0;count<tokenSize;count++)
	{
		token[count] = token[count] & mask[count];

		printf("%02x%02x\n",(unsigned char)token[count],(unsigned char)mask[count]);
	}

	/* now try and find it */	
	while (!found && !file_end)
	{
		if((bytesRead = read(infile,lgSearchBuffer,tokenSize)) != tokenSize)
			file_end = true;
		else{
			/* check to see if the string is at the start of the block */
			failed = false;
			
			for (count=0;count<tokenSize && !failed;count++)
			{
				if ((lgSearchBuffer[count] & mask[count]) != token[count])
					failed = true;
			}
				
			if (failed)
			{
				currPos += block_size;
			}else{
				found = true;
				gLastSearchPos = currPos;
			}
			
			lseek(infile,currPos,SEEK_SET);
		}
	}

	return found;
}
	
