/*--------------------------------------------------------------------------------*
 * Name:	Look
 * Description:
 * 	This program is a simple file viewer for large files. It will fill a 
 * 	buffer and display the contents. It will accept a small sub command
 * 	set the describes the way that the data is displayed on the screen.
 *
 * Author:	Peter Antoine.
 * Date:	16th July 2001
 *--------------------------------------------------------------------------------*
 *{{{
 * version  Author         Change
 * -------  -------------  --------------------------------------------------
 * v2.0     P.Antoine      Added the + and - commands, and fixed crap error
 *                         when suppling bad command entries.
 * v2.1     P.Antoine      Added 'b' and 'e' commands. This is so I can look
 *                         at MPEG video that is stream based and both big and
 *                         little endian at the same time. Well it is stream
 *                         based.
 * v2.2     P.Antoine      Added the '.' command. This will continue a search 
 *                         from the end of the current search. Useful for binary
 *                         searches as this will find occurs on the displayed
 *                         page as well.
 * v2.3		P.Antoine      Added 'E' command that sets the viewer at the EOF.
 * v2.4		P.Antoine      Added 'd' command to dump a region in hex to a file.
 * v2.5		P.Antoine      Now attempts to auto detect to see if the file is hex
 *                         or text. Also does not display the bell char.
 * v2.6		P.Antoine      Fixed simple problems with the dump. 'E' or 'e' now
 *                         means the end of the file.
 * v3.0     P.Antoine      Now cross-platform Linux/Win32.
 * v3.1     P.Antoine      Adding search that has a mask.As this beasty is starting
 *                         to get a little on the complicated side, have added
 *                         a (slightly) more informative help message.
 * v3.2     P.Antoine      Added 'r' function to allow for the start of current 
 *                         block of block_size to be found. This is useful for 
 *                         block oriented file types (MPEG TS - 188 byte blocks).
 * v3.3     P.Antoine      Added 'O' and 'o' functions for blocked Masked search.
 *                         It will find a masked string at the offset specified 
 *                         thought out the file.
 * v3.4     P.Antoine      Added 'D' command. This the same as the 'd' command
 *                         except the second parameter is the size of the data
 *                         that is to be dumped.
 * v3.5     P.Antoine      Fixed the 'E' option so that it now goes to the end
 *                         of the file.
 * v3.6     P.Antoine      Added the BLOCK_MODE display. (VIM Spell checked!!)
 * v3.7     P.Antoine      Changed BLOCK_MODE to allow for the block size to
 *                         be set with the change of mode.
 * v3.8     P.Antoine      Added 'G' to allow for BLOCK_MODE goto's.
 * v4.0     P.Antoine      No new feature, just code tidy up. Major changes to
 *                         the codebase so upped the major version.
 * v4.1     P.Antoine      Fixed a couple of bugs on *nix build.
 * v4.2     P.Antoine      Got board of converting hex dumps to headers so added
 *                         the "C" function to do this.
 *-----------------------------------------------------------------------------}}}*/

#ifdef _WIN32
	#include <io.h>
	#define	STDOUT	_fileno(stdout)
#else
	#include <unistd.h>
	#define	STDOUT	fileno(stdout)
#endif

#define CURRENT_VERSION	"4.2"

#include <errno.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>

#include "ConvTools.h"
#include "look.h"

unsigned int	gDriveMode = 0;
unsigned int	gPacketMode = 0;
unsigned int	gPacketSize = 0;
unsigned int	gBigEndian = 1;
unsigned long	gFileSize;
unsigned long	gDiskSector = 0;
unsigned int	gLastLine = 0;
unsigned int	gLastDisplayPoint = 0;
unsigned int	gLastSearchPos = 0;
unsigned char	gSectorBuffer[DISK_BUFFER_SIZE];
bool			gDiskOk = false;
	
int	main(int argc,char *argv[])
{
	int	infile = -1,result,width = BYTE_WIDE;
	int	screenSize = 40;
	int	mode = TEXT_MODE;
	unsigned char	test;
	bool	fileEnd(false),failed(false);

	FeatureDisplay();
	
	printf( "\n      look file viewer %s \n"
			"Copyright (c) 2003 - 2010 Peter Antoine\n"
      		"         All rights reserved. \n\n\n",CURRENT_VERSION);

	if (argc < 2)
	{
		printf(	"Usage:  look <filename>\n"
			"        look -d <disk_name>\n");
		failed = true;
	}else{
		if (argv[1][0] == '-')
		{
			if (argv[1][1] == 'd')
			{
				if (!gDiskOk)
				{
					printf("%s: Disk Mode only works on NT4+, Sorry!\n",argv[0]);
					failed = true;
				}else{
					OpenDisk(argv[0],argv[2]);
				}
			}
			else if (argv[1][1] == 'p')
			{
				/* packet mode */
				gPacketMode = true;
				
				if (argv[1][2] != '\0')
				{
					gPacketSize = atoi(&argv[1][2]);
				}
				else if (argv[2][0] != '-')
				{
					gPacketSize = atoi(argv[2]);
				}

				if (gPacketSize <= 0)
				{
					printf("%s: Invalid packet size.",argv[0]);
					failed = 1;
				}
			}
			else{
				printf("%s: Invalid parameter (%c)\n",argv[0],argv[1][1]);
				failed = true;
			}
		}
	}

	if (!failed)
	{
		infile = open(argv[1],READ_FILE_STATUS);

		/* autodetect file mode */
		for (result=0;result<16 && mode != HEX_MODE;result++)
		{
			read(infile,&test,1);
			
			if (!(isprint(test) || isspace(test) ))
			{
				mode = HEX_MODE;
				break;
			}
		}

		/* get the file size (the hard way) */
		if (!gDriveMode)
		{
			gFileSize = lseek(infile,0,SEEK_END);
			lseek(infile,0,SEEK_SET);
		}

		if (infile == -1 && !gDriveMode)
		{
			printf("Failed to open %s \n",argv[1]);
		}else{
			while ((result = ParseCommand(infile,screenSize,mode,width)) != CLOSE_DOWN)
			{
				if (result == DISPLAY_NEXT)
				{
					if (gDriveMode)
					{
						fileEnd = DriveHexDump(screenSize,width);

						if (fileEnd)
							printf("\n(EOD)\n");
					}else{
						switch(mode)
						{
							case HEX_MODE:
								fileEnd = HexDump(infile,screenSize,width);
								break;

							case TEXT_MODE:	
								fileEnd = TextDump(infile,screenSize);
								break;

							case BINARY_MODE:
								fileEnd = BinDump(infile,screenSize);
								break;

							case BLOCK_MODE:
								fileEnd = HexBlockDump(infile,screenSize,width);
								break;
						}
						if (fileEnd)
							printf("\n(EOF)\n");
					}
				}
			}
			close(infile);
			FeatureTidy();
		}
	}

	return failed;
}

int	ParseCommand(int infile,int &screenSize,int &mode,int &width)
{
	int	count,pos,adjust = 1,nextPos,block_size;
	int	value,tokenSize(0),tempSize,filePos,result = DISPLAY_NEXT;
	bool	failed(false),fromStart(false);
	bool	c_header(false);
	bool	badCommand(false);
	unsigned char	current_command;
	unsigned char	inbuffer[81],token[81],mask[81],tail;
	
	printf("Command: ");
	if (fgets((char*)inbuffer,80,stdin) == NULL)
	{
		printf("Failed to get input from stdin. Exiting...\n");
		return CLOSE_DOWN;
	}

	/* return display next block */
	if (inbuffer[0] != '\0' && inbuffer[0] != '\n')
	{
		/* skip any first line spaces */
		count = 0;

		current_command = inbuffer[count++];
		
		switch(current_command)
		{
			case 's':	/* screen size */
				tempSize = 0;
				
				if ((tokenSize = GetToken(inbuffer,count,token,false,&nextPos)) != 0)
					tempSize = atoi((const char*)token);

				if (mode == BLOCK_MODE && tempSize > 0)
					screenSize = tempSize;
				else if (tempSize > 0 && tempSize < 120)
					screenSize = tempSize;
				else
					badCommand = true;

				result = DONT_DISPLAY;				
				break;

			case 'h':	/* hex mode */
				mode = HEX_MODE;
				break;

			case 't':	/* text mode */
				mode = TEXT_MODE;
				break;

			case 'b':	/* binary mde */
				mode = BINARY_MODE;
				break;

			case 'B':	/* block mode */
				mode = BLOCK_MODE;
				value = (lseek(infile,0,SEEK_CUR)/screenSize) * screenSize;
				lseek(infile,value,SEEK_SET);

				/* optional size follows block mode -- seems more sensible after usage */
				if ((tokenSize = GetToken(inbuffer,count,token,false,&nextPos)) != 0)
				{
					tempSize = atoi((const char*)token);

					if (tempSize > 0)
						screenSize = tempSize;
				}

				break;	

			case 'e':	/* endian swap */
				gBigEndian = !gBigEndian;
				break;
			
			case 'w':	/* hex mode work width */
				if ((tokenSize = GetToken(inbuffer,count,token,false,&nextPos)) == 0)
				{
					badCommand = true;
					printf("Bad command %s \n",token);
					result = DONT_DISPLAY;				
				}else{
					switch(tolower(token[0]))
					{
						case 'b':
							width = BYTE_WIDE;
							break;
						case 'w':
							width = WORD_WIDE;
							break;
						case 'd':
							width = DWORD_WIDE;
							break;
						default:
							badCommand = true;
							printf("Invalid width, must be B(YTE), W(ORD) or D(WORD)\n");
					}
					result = DONT_DISPLAY;				
				}
				break;
		
			case 'p':	/* previous page */
				if (gDriveMode)
				{
					/* find the right sector and the line within the sector */
					gDiskSector = gDiskSector - (screenSize / 32);
					gLastLine   = 32 - (2*screenSize - ((((2*screenSize)/32)*32) + gLastLine)); 

					if (gLastLine == 32)
						gLastLine = 0;
				}else{
					if (mode == TEXT_MODE)
						FindPrevTextPage(infile,screenSize,false);
					else{
						if (mode == BLOCK_MODE)
							pos = ((lseek(infile,0,SEEK_CUR) / screenSize) -1) * screenSize;
						else
							pos = lseek(infile,0,SEEK_CUR) - 16 * (screenSize*2);

						if (pos > 0)
							lseek(infile,pos,SEEK_SET);
						else
							lseek(infile,0,SEEK_SET);
					}
				}
				break;

			case 'r':	/* round down to */
				if (gDriveMode)
					result = DONT_DISPLAY;		/* dont do anything in drive mode */
				else{
					if ((tokenSize = GetToken(inbuffer,count,token,false,&nextPos)) == 0)
					{
						result = DONT_DISPLAY;
						printf("Bad parameter needs a valid block size.\n");
					}else{
						GetValue(token,tokenSize,value);

						if (value == 0)
						{
							result = DONT_DISPLAY;
							printf("Bad parameter - must have a valid numerical block size > 0");
							
						}else{
							/* now round to the start of the <value> block */
							switch(mode)
							{
								case TEXT_MODE:
									value = (gLastDisplayPoint / value) * value;
									break;
								case HEX_MODE:
									pos = (lseek(infile,0,SEEK_CUR) - 16 * screenSize);
									value = (pos / value) * value;
									break;
								case BINARY_MODE:	
									pos = (lseek(infile,0,SEEK_CUR) - 8 * screenSize);
									value = (pos / value) * value;
									break;
								case BLOCK_MODE:
									/* round down to value, then round down to block size */
									value = (lseek(infile,0,SEEK_CUR) / value) * value;
									value = (value / screenSize) * screenSize;
									break;
							}
							
							if (lseek(infile,value,SEEK_SET) == -1)
							{
								printf("Failed: %d (%d)\n",errno,EINVAL);
								result = DONT_DISPLAY;
							}
						}
					}
				}
				break;
	
			/* goto relative position */
			case '-':	adjust = -1;	// set up at the top to 1 !!!
			case '+':
				if (gDriveMode)
				{
					printf("Command not supported for Disk display.\n");
					result = DONT_DISPLAY;
				}else{
					if ((tokenSize = GetToken(inbuffer,count,token,false,&nextPos)) == 0)
					{
						result = DONT_DISPLAY;
						printf("Bad command %s \n",token);
					}else{
						GetValue(token,tokenSize,value);
										
						switch(mode)
						{
							case TEXT_MODE:
								value = gLastDisplayPoint + (value * adjust);
								break;
							case HEX_MODE:
								value = (lseek(infile,0,SEEK_CUR) - 16 * screenSize) + (value * adjust);
								break;
							case BINARY_MODE:	
								value = (lseek(infile,0,SEEK_CUR) - 8 * screenSize) + (value * adjust);
								break;
							case BLOCK_MODE:
								value *= screenSize;
								value = lseek(infile,0,SEEK_CUR) - screenSize + (value * adjust);
								break;
						}

						if (value > gFileSize || value < 0)
						{
							printf("New postion off front or end of file.\n\n");
							result = DONT_DISPLAY;
						}
						else if (lseek(infile,value,SEEK_SET) == -1)
						{
							printf("Failed: %d (%d)\n",errno,EINVAL);
							result = DONT_DISPLAY;
						}
					}
				}
			break;	

			case 'g':	/* goto position */
			case 'G':
				if ((tokenSize = GetToken(inbuffer,count,token,false,&nextPos)) == 0)
				{
					badCommand = true;
					printf("Bad command %s \n",token);
				}else{
					if (GetValue(token,tokenSize,value))
					{
						if (current_command == 'G' && mode == BLOCK_MODE)
						{
							value = value * screenSize;
						}

						if (value > gFileSize)
						{
							printf("Bad goto position.\n\n");
							result = DONT_DISPLAY;
						}
						else if (gDriveMode)
						{
							if(value < gFileSize)
							{
								gDiskSector = value;
								gLastLine   = 0;
							}else{
								printf("Failed sector off the end of the disk\n");
								result = DONT_DISPLAY;
							}
						}
						else
						{
							/* round the number down to block size */
							if (mode == BLOCK_MODE)
							{
								value = (value/screenSize) * screenSize;
							}

							if (lseek(infile,value,SEEK_SET) == -1)
							{
								printf("Failed: %d (%d)\n",errno,EINVAL);
								result = DONT_DISPLAY;
							}
						}
					}else{
						printf("Non-Numeric goto position entered\n");
						result = DONT_DISPLAY;				
					}
				}

				break;

			case 'E':	/* goto the end of the file -- well last page */
				if (mode == TEXT_MODE)
				{
					lseek(infile,0,SEEK_END);
					FindPrevTextPage(infile,screenSize,true);
				}else{
					if (mode == BLOCK_MODE)
					{
						/* round down to the nearest block size */
						pos = ((lseek(infile,0,SEEK_END)/screenSize) - 1) * screenSize;
					}else{
						pos = lseek(infile,0,SEEK_END) - 16 * (screenSize);
					}

					if (pos > 0)
						lseek(infile,pos,SEEK_SET);
					else
						lseek(infile,0,SEEK_SET);
				}	
				break;		
			
			case 'F':	/* find a string from start */
				if (!gDriveMode)
				{
					filePos = lseek(infile,0,SEEK_CUR);
					lseek(infile,0,SEEK_SET);
				}
				fromStart = true;			
				
/* ***** No BREAK follow on ***** */

			case 'f':	/* find string */
				if ((tokenSize = GetToken(inbuffer,count,token,true,&nextPos)) != 0)
				{
					if (!FindString(infile,token,tokenSize,fromStart,false))
					{
						printf("String Not Found. (%s)\n",token);
						result = DONT_DISPLAY;
					}
				}
				break;

			case 'O':	fromStart = true;	/* block search - block size, string, mask */
			case 'o':
				
				memset(mask,0,sizeof(mask));
				badCommand = true;
				
				/* get block_size */
				if ((tokenSize = GetToken(inbuffer,count,token,false,&nextPos)) != 0)
				{
					count = nextPos;
					GetValue(token,tokenSize,block_size);

					if (!(block_size < 0 || block_size > gFileSize))
					{
						/* get the mask value */
						if ((tokenSize = GetToken(inbuffer,count,mask,true,&nextPos)) != 0)
						{
							count = nextPos;

							/* Get token to search for */
							if ((tokenSize = GetToken(inbuffer,count,token,true,&nextPos)) != 0)
							{
								badCommand = false;
								
								/* Do the search -- with the mask */
								if (!FindBlockedMaskString(infile,token,mask,tokenSize,block_size,fromStart))
								{
									printf("String Not Found.\n");
									result = DONT_DISPLAY;
								}
							}
						}
					}
				}

				if (badCommand)
				{
					printf("command format wrong: <block_size> <mask> <search_string>\n");
					result = DONT_DISPLAY;
				}
				break;


			case 'M':	/* find a masked string from start */
				if (!gDriveMode)
				{
					filePos = lseek(infile,0,SEEK_CUR);
					lseek(infile,0,SEEK_SET);
				}
				fromStart = true;			
				
/* ***** No BREAK follow on ***** */

			case 'm':	/* find masked bitmap : <mask> <token> */
				memset(mask,0,sizeof(mask));
		
				if ((tokenSize = GetToken(inbuffer,count,mask,true,&nextPos)) == 0)
					badCommand = true;
				
				count = nextPos;

				/* Get token to search for */
				if (!badCommand && (tokenSize = GetToken(inbuffer,count,token,true,&nextPos)) == 0)
						badCommand = true;
				
				if (!badCommand)
				{
					/* Do the search -- with the mask */
					if (!FindMaskedString(infile,token,mask,tokenSize,fromStart,false))
					{
						printf("String Not Found.\n");
						result = DONT_DISPLAY;
					}
				}
				break;

			case '.': /* from end of the last search */
				if ((tokenSize = GetToken(inbuffer,count,token,true,&nextPos)) != 0)
				{
					if (!FindString(infile,token,tokenSize,false,true))
					{
						printf("String Not Found. (%s)\n",token);
						result = DONT_DISPLAY;
					}
				}
				break;

			case 'C':	/* Dump <start> <end> <file>  -- C-style header file */
						c_header = true;
			case 'D':	/* Dump <start> <size> <file> */
			case 'd':	/* Dump <start> <end> <file> */
				{
					int start,end;

					if ((tokenSize = GetToken(inbuffer,count,token,true,&nextPos)) != 0)
					{
						if (token[0] == 'C' || token[0] == 'c')
						{
							switch(mode)
							{
								case TEXT_MODE:
									start = gLastDisplayPoint;
									break;
								case HEX_MODE:
									start = (lseek(infile,0,SEEK_CUR) - 16 * screenSize);
									break;
								case BINARY_MODE:	
									start = (lseek(infile,0,SEEK_CUR) - 8 * screenSize);
									break;
							}
						}else{
							badCommand = !(GetValue(token,tokenSize,start));
						}
					}

					count += tokenSize+1;

					/* where to end the dump --- E or e means End of File */
					if (!badCommand && (tokenSize = GetToken(inbuffer,count,token,true,&nextPos)) != 0)
					{
						if (token[0] == 'E' || token[0] == 'e')
							end = gFileSize;
						else{
							badCommand = !(GetValue(token,tokenSize,end));

							/* for size dumps lets make end = start + size */
							if (!badCommand && current_command == 'D')
								end += start - 1;
						}
					}
					
					count += tokenSize+1;
					
					if (!badCommand && (tokenSize = GetToken(inbuffer,count,token,false,&nextPos)) != 0)
					{
						/* validate range */
						if (end > start && start >= 0 && end <= gFileSize)
						{	
							/* valid parameters --- lets do the dump */
							if (c_header)
								DumpHexToCHeaderFile(infile,start,end,(char*)token);
							else
								DumpHexToFile(infile,start,end,width,(char*)token);
						}else{
							printf("Bad parameters: end less then start and/or not in the file \n");
						}
					}else{
						printf("Bad parameters: <start> <end> <file_name> required\n");
					}

					result = DONT_DISPLAY;
				}
				break;

		
			case 'c':	/* capacity */
				if (gDriveMode)
					printf("Capacity: %ld blocks (%lx hex blocks)\n",gFileSize,gFileSize);
				else
					printf("Capacity: %ld bytes (%lx hex bytes)\n",gFileSize,gFileSize);

				result = DONT_DISPLAY;
				break;

			case 'q':	/* quit */
				result = CLOSE_DOWN;
				break;

			case '?':	/* help screen */
				printf(" The following commands are supported:\n");
				printf("%s\n",
					" s: Size        - This will set the number of lines/bytes that are to be displayed\n"
					" h: HexMode     - The display will be in hex\n"
					" t: TextMode    - The display will be in raw text\n"
					" b: BinaryMode  - The display will be in binary\n"
					" B: BlockMode   - The display will be hex in blocks of the given size\n"
					" e: Endian swap - This function will swap the endian mode\n"
					" w: WordWidth   - The word width - B(yte) W(word) D(Dword)\n"
					" p: Previous    - Displays the previous page\n"
					" r: rollback    - Moves to the start of block size (round-down)\n"
					" +: Go Plus     - Move the file pointer forward\n"
					" -: Go Minus    - Move the file pointer backward\n"
					" g: Goto        - This will change the file position\n"
					" G: Goto        - This will change the file position, if in block mode will goto the block number\n"
					" E: End         - This will change the file position to the last page of the file\n"
					" F: Find        - This will find the first occurrence of the string\n"
					" f: Find        - This will find the next occurrence of the string\n"
					" .: Find        - This will find the next occurrence from end of the last search\n"
					" O: Offset find - This will do a masked find on the offset number specified from the start\n"
					" o: Offset find - This will do the same search as above but from the current search point\n"
					" M: Masked find - This will first find the first occurrence of the search anded against the mask\n"
					" m: masked find - The will find the next occurrence of the masked search\n"
				    " D: dump        - Dumps a region of memory to a file as hex text ('E' or 'e' is to the EOF).\n"
				    " d: dump        - Dumps a region of memory to a file as hex text ('E' or 'e' is EOF).\n"
				    " C: header dump - Dumps a region of memory to a file as C header file ('E' or 'e' is EOF).\n"
					" c: Capacity    - Shows the file/disk size\n"
					" q: Quit        - close the viewer\n"
					" ?: Help        - This display.\n\n"
					"Notes:\n\n"
					" multi-parameter commands:\n"
					"  masked finds : m <mask> <search>\n"
					"  offset finds : o <offset size> <mask> <search>\n"
					"          dump : D <start> <size> <filename>\n"
					"          dump : d <start> <end> <filename>\n"
					"          dump : C <start> <end> <filename>\n\n"
				    " Numeric commands: \n"
					"  G,g,+,- all take either decimal values or hex (0xhhhh...).\n\n"
					" String commands: \n"
					"  m,M,f,F,.      - take the following formats:\n"
					"  f peter        - this will find the string \"peter\"\n"
					"  F \"peter\"    - will find the first occurrence of the string \"peter\" in the file\n"
					"  . \"not much\" - will find the next accordance of \"not much\" starting from the last search\n"
					"  f \\x20\\x47   - will find the string \" G\" \n"
					"  m \\xff\\x0f \\x00\\x04 - will find the next word that has the format 00x4 (where x means don't care).\n"
					);
				result = DONT_DISPLAY;
				break;
				
			default:
				printf("Invalid Command - use ? to list commands\n");
				result = DONT_DISPLAY;
		}
	}

	return result;
}

/* vim: set fdm=marker : */
