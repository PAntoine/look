/*--------------------------------------------------------------------------------*
 * Name:	DumpHexToFile
 * Description:
 * This function will dump the file passed into for the number of lines requested.
 * It will return true if the file has reached its physical end.
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

#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <memory.h>
#include <sys/stat.h>

#include "ConvTools.h"

bool	DumpHexToFile(int infile,int start,int end,int width,char* filename)
{
	int				outfile,number,numLines,old_pos,numBytes;
	int				count,filePos,lineCount,bytesRead,multiplier;
	bool			eof(false);
	unsigned char 	buffer[16];
	unsigned char	line[80];

	multiplier = (2 << width) + 1;

	numLines = (end - start + 15) / 16;

	numBytes = (end - start + 1);	
	
	if (numLines*16 != (end - start))
		numLines++;
	
	/* first be rude not to try and open the file first */
	if ((outfile = open(filename,O_CREAT|O_TRUNC|O_WRONLY,S_IWRITE|S_IREAD)) != -1)
	{
		line[0] = '\n';
		count = 0;
	
		/* need to position the file at the right place - after saving the old location */
		old_pos = lseek(infile,0,SEEK_CUR);
		lseek(infile,start,SEEK_SET);
		
		for(lineCount=0;lineCount<numLines && !eof && numBytes > 0;lineCount++)
		{
			/* write the last line */
			write(outfile,line,count * multiplier+1);
			
			/* we read 16 bytes across the page */
			if (numBytes > 16)
			{
				if ((bytesRead = read(infile,buffer,16)) != 16)
					eof = true;
			} else {
				if ((bytesRead = read(infile,buffer,numBytes)) != numBytes)
					eof = true;
			}
			
			numBytes -= 16;

			switch(width)
			{
				case 0:		/* byte wide */
					for (count=0;count<bytesRead;count++)
					{
						GenUpperHex(buffer[count],&line[count*multiplier],&line[count*multiplier+1]);
						line[(count+1)*multiplier-1] = ',';
					}

					break;

				case 1:		/* word wide */
					number = (bytesRead + 1) >> 1;

					for (count=0;count<number;count++)
					{
						GenHexWord(*((unsigned short*)&buffer[count*2]),&line[count*multiplier]);
						line[(count+1)*multiplier-1] = ',';
					}
					
					break;

				case 2:		/* dword wide */
					number = (bytesRead + 3) >> 2;
					
					for (count=0;count<number;count++)
					{
						GenHexDWord(*((unsigned int*)&buffer[count*4]),&line[count*multiplier]);
						line[(count+1)*multiplier-1] = ',';
					}
					
					break;
			}
			
			line[count*multiplier] = '\n';
		}
			
		line[count*multiplier-1] = ' ';
		
		write(outfile,line,count * multiplier);
		close(outfile);

		/* postion the infile to the old location */
		lseek(infile,old_pos,SEEK_SET);
		
	}

	return eof;
}

#define OFFSET	(4)
#define ENTRY	(5)

static unsigned char preamble[] = "static unsigned char ";
static unsigned char bit_middle[] = "[] = \n{\n";
static unsigned char postamble[] = "};\n\nstatic unsigned int ";
static unsigned char postamble2[] = "_size = sizeof(";
static unsigned char postamble3[] = ");\n\n";

bool	DumpHexToCHeaderFile(int infile,int start,int end,char* filename)
{
	int				outfile,number,numLines,old_pos,numBytes;
	int				struct_count,count,filePos,lineCount,bytesRead,multiplier;
	bool			eof(false);
	unsigned char 	buffer[16];
	unsigned char	struct_name[33];
	unsigned char	out_buffer[80];
	unsigned char	line[] = "\t\t0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00  ";

	numLines = (end - start + 15) / 16;
	numBytes = (end - start + 1);	
	
	if (numLines*16 != (end - start))
		numLines++;
	
	for (struct_count=0;struct_count<32 && filename[struct_count] != '.' && filename[struct_count] != '\0';struct_count++)
	{
		struct_name[struct_count] = filename[struct_count];
	}

	/* first be rude not to try and open the file first */
	if ((outfile = open(filename,O_CREAT|O_TRUNC|O_WRONLY,S_IWRITE|S_IREAD)) != -1)
	{
		/* need to position the file at the right place - after saving the old location */
		old_pos = lseek(infile,0,SEEK_CUR);
		lseek(infile,start,SEEK_SET);
		
		write(outfile,preamble,sizeof(preamble)-1);
		write(outfile,struct_name,struct_count);
		write(outfile,bit_middle,sizeof(bit_middle)-1);

		for(lineCount=0;lineCount<numLines && !eof && numBytes > 0;lineCount++)
		{
			/* we read 16 bytes across the page */
			if ((bytesRead = read(infile,buffer,16)) != 16)
			{
				eof = true;
			}

			for (count=0;count<bytesRead;count++)
			{
				GenUpperHex(buffer[count],&line[OFFSET+(count*ENTRY)],&line[OFFSET+(count*ENTRY+1)]);
			}

			numBytes -= 16;

			if (eof || numBytes <= 1)
			{
				line[(count*ENTRY)+1] 	= ' ';
			}
			else
			{
				line[(count*ENTRY)+1] 	= ',';
			}

			line[(count*ENTRY)+2] 	= '\n';
			line[(count*ENTRY)+3] 	= 0;

			/* write the last line */
			write(outfile,line,(OFFSET + ((count-1)) * ENTRY) + 4);
		}

		write(outfile,postamble,sizeof(postamble)-1);
		write(outfile,struct_name,struct_count);
		write(outfile,postamble2,sizeof(postamble2)-1);
		write(outfile,struct_name,struct_count);
		write(outfile,postamble3,sizeof(postamble3)-1);

		close(outfile);

		/* postion the infile to the old location */
		lseek(infile,old_pos,SEEK_SET);
	}

	return eof;
}


