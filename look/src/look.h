/*--------------------------------------------------------------------------------*
 * Name:	Look
 * Description:
 * 	This program is a simple file viewer for large files. It will fill a 
 * 	buffer and display the contents. It will accept a small sub command
 * 	set the describes the way that the data is displayed on the screen.
 *
 * Author:	Peter Antoine.
 * Date:	16th July 2001
 *
 *--------------------------------------------------------------------------------*/


#define	HEX_MODE		1
#define	TEXT_MODE		2
#define BINARY_MODE		3
#define BLOCK_MODE		4

#define	K_64		64*1024
#define K_64_IN_SECTORS	((K_64)/512)

#define DISK_BUFFER_SIZE ((512 * 3))

#define	DISPLAY_NEXT	0
#define CLOSE_DOWN	1
#define	DONT_DISPLAY	2

#define BYTE_WIDE	0
#define WORD_WIDE	1
#define	DWORD_WIDE	2

#ifndef _WIN32
	#define HANDLE void*
	#define	FIND_LAST(str,chr) rindex(str,chr)
	#define WRITE_FILE_STATUS	(O_TRUNC|O_CREAT|O_WRONLY)
	#define WRITE_FILE_PERM		(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
	#define READ_FILE_STATUS	(O_RDONLY)
#else
	#define FIND_LAST(str,chr) strrchr(str,chr)
	#define WRITE_FILE_STATUS	(O_BINARY|O_TRUNC|O_CREAT|O_WRONLY)
	#define WRITE_FILE_PERM		(I_SWRITE|I_SREAD)
	#define READ_FILE_STATUS	(O_BINARY|O_RDONLY)
#endif


int	GetToken(unsigned char* buffer,int start,unsigned char* outBuffer,bool escapeChars,int *nextPos);
int	ParseCommand(int infile,int &screenSize,int &mode,int &width);
bool	TextDump(int infile,int numLines);
bool	HexDump(int infile,int numLines,int width);
bool	BinDump(int infile,int numLines);
bool	DriveHexDump(int numlines,int width);
bool	FindString(int infile,unsigned char* token,int tokenSize,bool fromStart,bool fromLastSearch);
bool	FindMaskedString(int infile,unsigned char* token,unsigned char* mask,int tokenSize,bool fromStart,bool fromLastSearch);
bool	FindBlockedMaskString(int infile,unsigned char* token,unsigned char* mask, int tokenSize,int block_size,bool fromStart);
void	FindPrevTextPage(int infile,int numLines,bool atEnd);
bool	DumpHexToFile(int infile,int start,int end,int width,char* filename);
bool	HexBlockDump(int infile,int numBytes,int width);
bool	DumpHexToCHeaderFile(int infile,int start,int end,char* filename);

/* platform specifc functions --- these functions may well compile to nothing */
void	FeatureDisplay(void);
void	OpenDisk(char* progname,char* diskname);
void	FeatureTidy(void);
int		Read(unsigned long blockNumber,char *buffer,int size);

