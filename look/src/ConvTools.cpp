#include <ctype.h>	
#include <stdlib.h>


static unsigned char UpperHexBits[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
static unsigned char LowerHexBits[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

void	GenUpperHex(unsigned char in,unsigned char *highNibble,unsigned char *lowNibble)
{
	*highNibble = UpperHexBits[((in & 0xf0)>>4)];
	*lowNibble  = UpperHexBits[in & 0x0f];
}

void	GenLowerHex(unsigned char in,unsigned char *highNibble,unsigned char *lowNibble)
{
	*highNibble = LowerHexBits[((in & 0xf0)>>4)];
	*lowNibble  = LowerHexBits[in & 0x0f];
}

void	GenHexWord(unsigned short in, unsigned char* out)
{
	out[0] = UpperHexBits[(in & 0xf000)>>12];
	out[1] = UpperHexBits[(in & 0x0f00)>>8];
	out[2] = UpperHexBits[(in & 0x00f0)>>4];
	out[3] = UpperHexBits[(in & 0x000f)];
}

void	GenHexDWord(unsigned int in, unsigned char* out)
{
	out[0] = UpperHexBits[(in & 0xf0000000)>>28];
	out[1] = UpperHexBits[(in & 0x0f000000)>>24];
	out[2] = UpperHexBits[(in & 0x00f00000)>>20];
	out[3] = UpperHexBits[(in & 0x000f0000)>>16];
	out[4] = UpperHexBits[(in & 0x0000f000)>>12];
	out[5] = UpperHexBits[(in & 0x00000f00)>>8];
	out[6] = UpperHexBits[(in & 0x000000f0)>>4];
	out[7] = UpperHexBits[(in & 0x0000000f)];
}

bool	isHexDigit(unsigned char c)
{
	if ((c > '/' && c < ':')||(c > '@' && c < 'G')||(c > 96 && c < 'g'))
    		return true;
	else
		return false;
}

unsigned char	toHex(unsigned char c1,unsigned char c2)
{
	unsigned char	temp,result;
	
	if (c1 > '9')
	{
		temp = tolower(c1);

		if (c1 < 'a' || c1 > 'f')
			result = 0;
		else
			result = 10 + c1 - 'a';
	}else{
		result = c1 - '0';
	}

	result <<= 4;

	if (c2 > '9')
	{
		temp = tolower(c2);

		if (c2 < 'a' || c2 > 'f')
			result = 0;
		else
			result |= 10 + c2 - 'a';
	}else{
		result |= c2 - '0';
	}

	return result;
}
	
unsigned char	toOctal(unsigned char c1,unsigned char c2,unsigned char c3)
{
	unsigned char	result;

	if ((c1 < '0' || c1 > '7') || (c2 < '0' || c2 > '7') || (c3 < '0' || c3 > '7'))
		result = 0;
	else{
		result = (c1 - '0') * 64 + (c2 - '0') * 8 + c3 - '0';
	}
	return result;
}


bool	ConvertStringToHex(unsigned char* s,int size,unsigned int *value)
{
	unsigned int	working = 0,count = 0;
	bool	worked(true);
	
	if (((size/2)*2) != size)
	{
		/* its odd - first byte is a nibble */
		if (!isHexDigit(s[0]))
			worked = false;
		else{
			working = (unsigned char) toHex('0',s[0]);
			count = 1;
		}
	}

	while (count < size-1 && worked)
	{
		if (isHexDigit(s[count]) && isHexDigit(s[count+1]))
			working = (working << 8) + (unsigned char) toHex(s[count],s[count+1]);
		else
			worked = false;

		count += 2;
	}
	
	*value = working;

	return worked;
}

bool	GetValue( unsigned char* token, int tokenSize,int &value)
{
	bool	result = true;
	unsigned char	tail;
	
	tail = tolower(token[tokenSize-1]);
	value = atoi((const char*)token);

	if (token[0] == '0' && tolower(token[1]) == 'x')
	{
		/* its a hex address */
		if (!ConvertStringToHex(&token[2],tokenSize-2,(unsigned int*)&value))
		{
			result = false;
		}
	}else{
		tail = token[tokenSize-1];

		if (tail == 'm')
		{
			/* the number is in Meg */
			token[tokenSize-1] = '\0';
			value *= (1024 * 1024);
		}
		else if (tail == 'k')
		{
			/* the number is in K */
			token[tokenSize-1] = '\0';
			value *= 1024;
		}
	}

	return result;
}

