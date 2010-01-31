#ifdef _WIN32
	#include <io.h>
	#define	STDOUT	_fileno(stdout)
#else
	#include <unistd.h>
	#define	STDOUT	fileno(stdout)
#endif

#include <stdio.h>
#include <ctype.h>

void	DisplayHexLines(unsigned char* inBuffer,int position,int bytesRead,int width);


/*--------------------------------------------------------------------------------*
 * Name:	HexBlockDump
 * Description:
 *  This function will dump a file in HEX at block size. It will dump blockSize
 *  chars.
 *
 * Author:	Peter Antoine.
 *--------------------------------------------------------------------------------*/
bool	HexBlockDump(int infile,int numBytes,int width)
{
	int				count,numLines,leftOver,bytesRead,filePos;
	bool			eof = false;
	unsigned char	buffer[16];


	numLines = numBytes / 16;
	leftOver = numBytes % 16;
	filePos  = lseek(infile,0,SEEK_CUR);

	printf("block: %d\n", (filePos / numBytes));

	/* write the whole lines */
	for (count=0;count<numLines;count++)
	{
		bytesRead = read(infile,buffer,16);

		if (bytesRead != 16)
			eof = true;

		DisplayHexLines(buffer,filePos,bytesRead,width);
		filePos += bytesRead;
	}

	if (leftOver != 0)
	{
		/* now handle the life overs */
		bytesRead = read(infile,buffer,leftOver);
		DisplayHexLines(buffer,filePos,bytesRead,width);
		filePos += bytesRead;

		if (bytesRead != leftOver)
			eof = true;
	}

	return eof;
}

/*--------------------------------------------------------------------------------*
 * Name:	HexDump
 * Description:
 * This function will dump the file passed into for the number of lines requested.
 * It will return true if the file has reached its physical end.
 *
 * Author:	Peter Antoine.
 * Date:	16th July 2001
 *
 *--------------------------------------------------------------------------------*/
bool	HexDump(int infile,int numLines,int width)
{
	int				count,filePos,lineCount,bytesRead;
	bool			eof(false);
	unsigned char 	buffer[16];

	for(lineCount=0;lineCount<numLines && !eof;lineCount++)
	{
		filePos = lseek(infile,0,SEEK_CUR);

		/* we read 16 bytes across the page */
		if ((bytesRead = read(infile,buffer,16)) != 16)
			eof = true;

		DisplayHexLines(buffer,filePos,bytesRead,width);

	}

	return eof;
}

