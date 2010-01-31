/*--------------------------------------------------------------------------------*
 * Name:	FindPrevTextPage
 * Description:
 * This function will read though the text file backwards until it finds the the
 * start of the previous text page. That is it searches for x * 2 <cr> characters.
 * It then set the file pointer to point to this point in the file.
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

void	FindPrevTextPage(int infile,int numLines,bool atEnd)
{
	int		buffPos = 0,track,lineCount,bytesRead;
	bool	finished(false);
	char	buffer[1024];
	
	track = lseek(infile,0,SEEK_CUR);
	track -= 1024;

	if (!atEnd)
		lineCount = (numLines * 2) + 1;
	else
		lineCount = numLines + 1;

	
	while(track > 0 && !finished && lineCount > 0)
	{
		track = lseek(infile,track,SEEK_SET);
		
		/* read the data */
		if ((bytesRead = read(infile,buffer,1024)) != 1024)
		{
			finished = true;
		}else{
			buffPos = bytesRead;
			
			while(lineCount > 0 && buffPos > 0)
			{
				if (buffer[buffPos--] == '\r')
					lineCount--;
			}
		}

		if (lineCount > 0)
			track -= 1024;
	}

	buffPos+= 2;

	if (buffer[buffPos] == '\n')
		buffPos++;
	
	if (lineCount == 0)
		lseek(infile,track+buffPos,SEEK_SET);
	else if (track < 0)
		lseek(infile,0,SEEK_SET);
}

