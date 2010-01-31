#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ConvTools.h"


#include <stdio.h>

int	GetToken(unsigned char* buffer,int start,unsigned char* outBuffer,bool escapeChars,int *nextChar)
{
	int				count,outcount = 0;
	bool			finished(false),quotesOn(false);
	unsigned char	temp;
	
	for (count=0;count<strlen((const char*)buffer) && !finished;count++)
	{
		if (escapeChars)
		{
			
			if (buffer[start+count] == '\\')
			{
				count++;
				
				switch(buffer[start+count])
				{
					case 'n':
						outBuffer[outcount++] = 0x0a;
						break;
					case 't':
						outBuffer[outcount++] = 0x09;
						break;
					case 'v':
						outBuffer[outcount++] = 0x0b;
						break;
					case 'b':
						outBuffer[outcount++] = 0x08;
						break;
					case 'r':
						outBuffer[outcount++] = 0x0d;
						break;
					case 'f':
						outBuffer[outcount++] = 0x0c;
						break;
					case 'a':
						outBuffer[outcount++] = 0x07;
						break;
					case '\\':
						outBuffer[outcount++] = '\\';
						break;
					case '\"':
						outBuffer[outcount++] = '\"';
						break;

					case ' ':
						/* remove leading spaces */
						if (outcount != 0)
							outBuffer[outcount++] = ' ';
						break;
						
					case 'x':
						/* do hex coverstion */
						if (isHexDigit(buffer[start+count+1]) && isHexDigit(buffer[start+count+2]))
						{
							outBuffer[outcount++] = toHex(buffer[start+count+1],buffer[start+count+2]);
						}
						count += 2;
						break;

					case '0':
						if (!isdigit(buffer[start+count+1]))
						{
							outBuffer[outcount++] = '\0';
							break;
						}
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
						/* do Octal conersion */
						outBuffer[outcount++] = toOctal(buffer[start+count],buffer[start+count+1],buffer[start+count+2]);
						count += 2;
						break;
					
					default:
						/* bad escape code */
						count++;
						finished = true;
				}
			}
			else if (buffer[start+count] == '\"')
			{
				/* the token is in quotes - so must ignore spaces within */
				quotesOn = !quotesOn;
			}else{
				/* not a escape char - and lose leading spaces */
				if (!(outcount == 0 && buffer[start+count] == ' '))
				{
					if ((!quotesOn && (buffer[start+count] == ' ' || buffer[start+count] == '\n')) || buffer[start+count] == '\0')
					{
						finished = true;
						outBuffer[outcount] = '\0';
					}else{
						outBuffer[outcount++] = buffer[start + count];
					}
				}
			}
		}else{
			/* dont convert escape chrs */
			if (!(outcount == 0 && buffer[start+count] == ' '))
			{
				if (buffer[start+count] == ' ' || buffer[start+count] == '\n' || buffer[start+count] == '\0')
				{
					finished = true;
				}else{
					outBuffer[outcount++] = buffer[start + count];
					outBuffer[outcount] = '\0';
				}
			}
		}
	}

	*nextChar = (start+count);
	
	return outcount;
}


