void	GenLowerHex(unsigned char in,unsigned char *highNibble,unsigned char *lowNibble);
void	GenUpperHex(unsigned char in,unsigned char *highNibble,unsigned char *lowNibble);
void	GenHexWord(unsigned short in, unsigned char* out);
void	GenHexDWord(unsigned int in, unsigned char* out);
bool	isHexDigit(unsigned char c);
unsigned char	toHex(unsigned char c1,unsigned char c2);
unsigned char	toOctal(unsigned char c1,unsigned char c2,unsigned char c3);
bool	ConvertStringToHex(unsigned char* s,int size,unsigned int *value);
bool	GetValue(unsigned char* token, int tokenSize,int &value);

