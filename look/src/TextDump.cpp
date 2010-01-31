/*--------------------------------------------------------------------------------*
 * Name:	TextDump
 * Description:
 * This function will dump the text file from the last position for the number of
 * lines required.
 *
 * Author:	Peter Antoine.
 * Date:	16th July 2001
 *
 *--------------------------------------------------------------------------------*/

#ifdef _WIN32
	#include <io.h>
	#define	STDOUT	_fileno(stdout)
#else
	#include <unistd.h>
	#define	STDOUT	fileno(stdout)
#endif

#include <stdio.h>
#include <ctype.h>

extern unsigned int	gLastDisplayPoint;	// used for minus gotos

bool	TextDump(int infile,int numLines)
{
	bool	finished(false),eof(false);
	char	buffer[1024],*lineStart;
	int		totalCount(0),count(0),track,lineCount = 0,bytesRead,lineStartPos;
	
	track = lseek(infile,0,SEEK_CUR);
	gLastDisplayPoint = track;
	
	do{
		/* read the data */
		if ((bytesRead = read(infile,buffer,1024)) != 1024)
		{
			printf("Here\n");
			finished = true;
			eof = true;
		}

		lineStart = buffer;
		lineStartPos = 0;
		
		totalCount += count;
		count = 0;

		/* dump all the lines */
		while(lineCount < numLines && count < bytesRead)
		{
			/* remove the bells */
			if (buffer[count] == 0x07)
				buffer[count] = '.';
			
			switch (buffer[count++])
			{
				case 0x0a:		/* unix */
				case 0x0d:		/* dos */
				{
					/* output the line to stdout */
					write(STDOUT,lineStart,count-lineStartPos);

					/* update the pointers */
					lineStart = &buffer[count];
					lineStartPos = count;
					lineCount++;

					if (buffer[count] == 0x0a && buffer[count-1] != 0x0a)
						count++;
				}
				break;
			}
		}

		if (count >= bytesRead && !finished)
		{
			/* output the partial line to stdout */
			write(STDOUT,lineStart,bytesRead-lineStartPos);
		}

		if (lineCount == numLines)
		{
			/* exit and set the file to the last char displayed */
			finished = true;
			lseek(infile,track+totalCount+lineStartPos,SEEK_SET);
		}
	} 
	while(!finished);

	write(STDOUT,"\n",1);
	
	/* if we have not displayed the whole buffer -- dont tell user it is eof */
	if (count < bytesRead)
		eof = false;
	
	return eof;
}

