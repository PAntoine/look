/*********************************************************************************
 * Name: SCSI DATA HEADER
 *
 * Description:
 * 	This file holds the SCSI data structures that are returned from the SCSI
 * 	devices. Where this structure differs in SCSI-2 amd SCSI-3 there will be
 * 	notes, and if the structures do not actually clash, both structures will
 * 	be supported.
 *
 * Author: Peter Antoine.
 * Date:   30th Nov 2000
 *
 *********************************************************************************/

#ifndef	__SCSI_DATA_STRUC__
#define __SCSI_DATA_STRUC__

/* system ones needed by all the scsi functions */
#include <windows.h>
#include "ntddscsi.h"
#include "devioctl.h"
#include <sys/stat.h>

//#include <ntdddisk.h>

/* Magic Numbers */
#define	MAX_DRIVES_PER_LIBRARY	10

/* scsi defined numbers */
#define	SCSI_CHANGER	0x08
#define SCSI_TAPE_DRIVE	0x01

/* Usefull macros */

#define	MakeInt2(byte1,byte2)			(((byte1 << 8) & 0xff00) | (byte2 & 0x00ff))
#define MakeInt3(byte1,byte2,byte3)		(((byte1 << 16) & 0x00ff0000) | ((byte2 << 8) & 0xff00) | (byte3 & 0xff)) 
#define MakeInt4(byte1,byte2,byte3,byte4)	((((byte1 << 24) & 0x0ff000000) | ((byte2 << 16) & 0xff0000) | ((byte3 << 8) & 0xff00)) | (byte4 & 0x0ff))

#define GetByte1(x) (x & 0xff)
#define GetByte2(x) ((x >> 8) & 0xff)
#define GetByte3(x) ((x >> 16) & 0xff)
#define GetByte4(x) ((x >> 24) & 0xff)

/* SCSI_ADDR
 * this structure just holds the address for the devices that where found.
 */

typedef	struct{
	unsigned char	port;
	unsigned char	bus;
	unsigned char	scsiID;
	unsigned char	lun;
	unsigned short	element;
} SCSI_ADDR;

typedef struct{
	SCSI_ADDR	address;
	unsigned short	numTransport;
	unsigned short	transportElement;
	unsigned short	numStorage;
	unsigned short	firstStorage;
	unsigned short	numDrives;
	SCSI_ADDR	drive[MAX_DRIVES_PER_LIBRARY];
} LIBRARY, *PLIBRARY;

/* SCSI COMMANDS */
#define READ			0x08
#define READ_10			0x28
#define SPACE			0x11
#define WRITE			0x0A
#define WRITE_10		0x2A
#define	REWIND			0x01
#define LOAD_UNLOAD		0x1B
#define MOVE_MEDIUM		0xA5
#define	READ_ELEMENT_STATUS	0xB8
#define MODE_SENSE_6		0x1A
#define MODE_SENSE_10		0x5A
#define MODE_SELECT_6		0x15
#define MODE_SELECT_10		0x55
#define TEST_UNIT_READY		0x00
#define INQUIRY			0x12
#define READ_CAPACITY		0x25
#define WRITE_VERIFY		0x2E

/*--------------------------------------------------------------------------------
 * Sense code defintions.
 *
 * The sense codes break down into three parts. The first part the sense key tells
 * what class of error has occoured. This has not real bearing on the meaning of
 * the error. But... That is not totally true, with some devices the ASC, and ASCQ
 * can have a sutle differnce in meaning for the different sense keys. The ASC and
 * ASCQ are defined in the SCSI standards. These are the same and the range is 
 * defined across the different classes of devices. The same ASC and ASCQ should
 * mean the same thing on different device and for different device classes. But 
 * they dont. There are cases (from the same manufacturer) that the ASC and ASCQ
 * have different meanings.
 *
 * But, here are the genrerics.
 *
 * PS: these and some of the structures below are defined in a header file somewhere
 * inside the DDK but they are either not quite right, or you have to include to
 * many include files to use them.
 */

/* sense keys */
#define SCSI_SENSE_NO_SENSE         0x00
#define SCSI_SENSE_RECOVERED_ERROR  0x01
#define SCSI_SENSE_NOT_READY        0x02
#define SCSI_SENSE_MEDIUM_ERROR     0x03
#define SCSI_SENSE_HARDWARE_ERROR   0x04
#define SCSI_SENSE_ILLEGAL_REQUEST  0x05
#define SCSI_SENSE_UNIT_ATTENTION   0x06
#define SCSI_SENSE_DATA_PROTECT     0x07
#define SCSI_SENSE_BLANK_CHECK      0x08
#define SCSI_SENSE_UNIQUE           0x09
#define SCSI_SENSE_COPY_ABORTED     0x0A
#define SCSI_SENSE_ABORTED_COMMAND  0x0B
#define SCSI_SENSE_EQUAL            0x0C
#define SCSI_SENSE_VOL_OVERFLOW     0x0D
#define SCSI_SENSE_MISCOMPARE       0x0E
#define SCSI_SENSE_RESERVED         0x0F

/* additional sense codes */
#define	ADSC_INVALID_FIELD_IN_CDB	0x2400
#define ADSC_UNRECOVERED_READ_ERROR	0x1100
#define ADSC_RECORDED_ENTITY_NOT_FOUND	0x1400

/*--------------------------------------------------------------------------------
 * DISK_COMMAND_FLAGS
 * The flags that are supported by the disk drives are more complicated than
 * those for tape drives. Thus I creating defines here to make them more
 * readable.
 *
 * I have defined 0x00 field, this is so the code is more informative, you
 * are telling the reader what the unset field does.
 */

/* REL_ADDR (bit 1)
 * This tells the command what the logical block address actual means. It
 * can be either a actual address that is it is the address of the block 
 * that you want to read. Or a signed next block addres that is relative
 * to the last block that was read.
 */
#define	RA_ACTUAL_ADDRESS	0x00
#define	RA_RELATIVE_ADDRESS	0x01

/* BYTE CHECK (bit 2)
 * This bit tells the verify commands wether to check the data itself or
 * just to check that the medium thinks that the data has been written.
 */
#define	BC_NO_DATA_COMPARISON	0x00
#define BC_DATA_COMPARISON	0x02

/* FORCE_UNIT_ACCESS (bit 3)
 * This tells the device to read the block off the disk. It also causes the
 * device to read/write the block to the disk. When this is set the command
 * will/may take longer but you are garenteed that the data is on the device.
 */
#define FUA_FORCE_ACCESS	0x08
#define FUA_USE_CACHE		0x00

/* DISABLE_PAGE_OUT (bit 4)
 * This tells the device to not load the current page into the cache. This
 * stops uncommonly used pages from wasting the cache space where blocks
 * that are used more often can stay.
 */
#define DPO_DONT_LOAD_IN_CACHE	0x10
#define DPO_LOAD_IN_CACHE	0x00



/*--------------------------------------------------------------------------------
 *  INQUIRY DATA
 *  the following is SCSI-3 and SCSI-2 no fields clash in the standard, except that
 * some of the fields do overlap, but only reseverd->used, used->reseved
 */
typedef	struct{
	struct{

		unsigned char periphType:5;
		unsigned char periphQual:3;
	};
	struct{
		unsigned char deviceType:7;		// Reseved in SCSI-3
		unsigned char removable:1;
	};
	struct{
		unsigned char ANSIVersion:3;
		unsigned char ECMAVersion:3;
		unsigned char ISOVersion:2;
	};
	struct{
		unsigned char RespDataFmt:4;
		unsigned char rsvd1:1;
		unsigned char NormACA:1;
		unsigned char TrimIOP:1;
		unsigned char AENC:1;
	};
	unsigned char	AddLength;
	unsigned char	rsvd2;
	struct{
		unsigned char Addr16:1;			// Following byte reseved in SCSI-2
		unsigned char Addr32:1;
		unsigned char ACKQREQQ:1;
		unsigned char MediaChanger:1;
		unsigned char MultiPort:1;
		unsigned char VendSpec1:1;
		unsigned char EndServ:1;
		unsigned char rsvd3:1;
	};
	struct{
		unsigned char VendSpec2:1;
		unsigned char CmdQueue:1;
		unsigned char TransDis:1;
		unsigned char Linked:1;
		unsigned char Sync:1;
		unsigned char WBus16:1;
		unsigned char WBus32:1;
		unsigned char RelAddr:1;
	};
	unsigned char	VendorID[8];
	unsigned char	ProductID[16];
	unsigned char	RevisionID[4];
	unsigned char	VendorSpec[1];		// vendor specific code to follow
} INQUIRY_DATA, *PINQUIRY_DATA;

/*------------------------------------------------------------------------------------*
 *  MODE_SELECT & MODE_SENSE (6 & 10)
 * The following structures are the mode select structures for the command and the
 * the configuration pages. The Mode sense command also uses the same structures to
 * return the data in.
 *
 * Format of the returned data:
 *
 * two formats: paged and non-paged:
 *
 * Non-Paged:
 * 	PARAMTER_LIST_HEADER
 * 	BLOCK_DECRIPTOR (OPTIONAL)
 * 	VENDOR_UNIQUE DATA (1 - 5 bytes)
 *
 * Paged:
 * 	PARAMETER_LIST_HEADER
 * 	BLOCK_DESCRIPTOR (OPTIONAL)
 * 	* PAGE DATA	
 */

#define	MS_NON_PAGE_FORMAT	0x00
#define MS_READ_WRITE_ER_PAGE	0x01
#define MS_DISS_RECON_PAGE	0x02
#define MS_CONTROL_MODE_PAGE	0x0a
#define MS_DATA_COMPRESS_PAGE	0x0f
#define MS_DEVICE_CONFIG_PAGE	0x10
#define MS_MEDIUM_PARTION_PAGE	0x11
#define MS_VENDOR_UNIQUE_1_PAGE	0x20
#define MS_VENDOR_UNIQUE_2_PAGE	0x20
#define MS_ALL_AVAILABLE_PAGES	0x3f

typedef struct{
	unsigned char	revd1[2];
	struct{
		unsigned char speed:4;
		unsigned char buffered_mode:3;
		unsigned char rsvd2:1;
	};
	unsigned char	blockDescLen;
} MODE_SELECT_HEADER_6,*PMODE_SELECT_HEADER_6;

typedef struct{
	unsigned char	revd1[3];
	struct{
		unsigned char speed:4;
		unsigned char buffered_mode:3;
		unsigned char rsvd2:1;
	};
	unsigned char	rsvd3[2];
	unsigned char	blockDescLenMSB;
	unsigned char	blockDescLenLSB;
} MODE_SELECT_HEADER_10,*PMODE_SELECT_HEADER_10;

typedef struct{
	unsigned char	modeDataLen;
	unsigned char	mediumType;
	struct{
		unsigned char speed:4;
		unsigned char buffered_mode:3;
		unsigned char wp:1;
	};
	unsigned char	blockDescLen;
} MODE_SENSE_HEADER_6,*PMODE_SENSE_HEADER_6;

typedef struct{
	unsigned char	modeDataLenMSB;
	unsigned char	modeDataLenLSB;
	unsigned char	mediumType;
	struct{
		unsigned char speed:4;
		unsigned char buffered_mode:3;
		unsigned char rsvd2:1;
	};
	unsigned char	rsvd3[2];
	unsigned char	blockDescLenMSB;
	unsigned char	blockDescLenLSB;
} MODE_SENSE_HEADER_10,*PMODE_SENSE_HEADER_10;

typedef struct{
	unsigned char	densityCode;
	unsigned char	numBlocks[3];
	unsigned char	rsvd;
	unsigned char	blockLength[3];
} MS_BLOCK_DESCRIPTOR,*PMS_BLOCK_DESCRIPTOR;

typedef struct{
	struct{
		unsigned char	pageCode:6;
		unsigned char	rsvd1:2;
	};
	unsigned char	pageLength;
	struct{
		unsigned char	activeFormat:5;
		unsigned char	CAF:1;
		unsigned char	CAP:1;
		unsigned char	rsvd2:1;
	};
	unsigned char	activePartition;
	unsigned char	writebufferRatio;
	unsigned char	readbufferemptyratio;
	unsigned char	writeDelay[3];
	struct{
		unsigned char	REW:1;
		unsigned char	RBO:1;
		unsigned char	SOCF:2;
		unsigned char	AVC:1;
		unsigned char	RSmk:1;
		unsigned char	BIS:1;
		unsigned char	DBR:1;
	};
	unsigned char	gapSize;
	unsigned char	buffSizeAtUWarn[3];
	unsigned char	selectCompAlgo;
	unsigned char	rsvd3;
} DEVICE_CONFIGURATION_PAGE,*PDEVICE_CONFIGURATION_PAGE;

/*------------------------------------------------------------------------------------*
 *  READ_ELEMENT_STATUS
 * These structures are needed to be able to read the element status that is returned
 * from a library.
 *
 * The format of the data returned from the drive is:
 *	ELEMENT_STATUS_DATA
 *	ELEMENT_STATUS_PAGE	one of these for each element page type requested
 *	(element page data) 	as many of these as was requested/the library has
 */

#define RES_ALL_ELEMENTS		0x00
#define RES_MEDIUM_TRANSPORT_ELEMENT	0x01
#define RES_STORAGE_ELEMENT		0x02
#define RES_IMPORT_EXPORT_ELEMENT	0x03
#define	RES_DATA_TRANSFER_ELEMENT	0x04

typedef	struct{
	unsigned char	FirstElemAddrMSB;
	unsigned char	FirstElemAddrLSB;
	unsigned char	NumElemReptMSB;
	unsigned char	NumElemReptLSB;
	unsigned char	rsvd;
	unsigned char	ByteCountAvailByte1;
	unsigned char	ByteCountAvailByte2;
	unsigned char	ByteCountAvailByte3;
	} ELEMENT_STATUS_DATA,*PELEMENT_STATUS_DATA;

typedef struct{
	unsigned char	ElementTypeCode;
	struct{
		unsigned char rsvd1:6;
		unsigned char AVolTag:1;
		unsigned char PVolTag:1;
	};
	unsigned char	ElementDescLenMSB;
	unsigned char	ElementDescLenLSB;
	unsigned char	rsvd2;
	unsigned char	ByteCountAvailByte1;
	unsigned char	ByteCountAvailByte2;
	unsigned char	ByteCountAvailByte3;
	} ELEMENT_STATUS_PAGE,*PELEMENT_STATUS_PAGE;

typedef struct{
	unsigned char	ElementAddrMSB;
	unsigned char	ElementAddrLSB;
	struct{
		unsigned char full:1;
		unsigned char rsvd1:1;
	   	unsigned char exceptf:1;
		unsigned char access:1;	
		unsigned char rsvd2:4;
	};
	unsigned char	rsvd3;
	unsigned char	AddSenseCode;
	unsigned char	AddSenseQual;
	unsigned char	rsvd4[3];
	struct{
		unsigned char rsvd5:6;
		unsigned char invert:1;
		unsigned char svaild:1;
	};
	unsigned char	sourceElemAddrMSB;
	unsigned char	sourceElemAddrLSB;
//	unsigned char	primVolTag[36];
	unsigned char	resevd6[4];
	} STORAGE_ELEMENT_DESCRIPTOR,*PSTORAGE_ELEMENT_DESCRIPTOR;

typedef struct{
	unsigned char	ElementAddrMSB;
	unsigned char	ElementAddrLSB;
	struct{
		unsigned char full:1;
		unsigned char rsvd1:1;
	   	unsigned char exceptf:1;
		unsigned char access:1;	
		unsigned char rsvd2:4; 
	};
	unsigned char	rsvd3;
	unsigned char	AddSenseCode;
	unsigned char	AddSenseQual;
	struct{
		unsigned char lun:3;
		unsigned char rsvd4:1;
		unsigned char LUValid:1;
		unsigned char IDVaild:1;
		unsigned char rsvd7:1;
		unsigned char notBus:1;
	};
	unsigned char	scsiBusAddr;
	unsigned char	rsvd9;
	struct{
		unsigned char rsvd6:6;
		unsigned char invert:1;
		unsigned char svaild:1;
	};
	unsigned char	sourceElemAddrMSB;
	unsigned char	sourceElemAddrLSB;
//	unsigned char	primVolTag[36];
	unsigned char	rsvd8[4];
//	unsigned char	VendorSpec[10];
	} DATA_TRANSFER_ELEMENT_DESCRIPTOR,*PDATA_TRANSFER_ELEMENT_DESCRIPTOR;

/*----------------------------------------------------------------------------*
 *  SENSE_DATA
 * this structure is SCSI-2, SCSI-3 used the the first two bytes that
 * are reserved here.
 */
typedef struct{
	unsigned char	rsvd1;
	unsigned char	rsvd2;
	struct{
		unsigned char	sense_key:4;
		unsigned char	rsvd:4;
	};
	unsigned char	information[4];
	unsigned char	addSenseLen;
	unsigned char	specificInfo[4];
	unsigned char	ASC;
	unsigned char	ASCQ;
	unsigned char	FRUC;
	unsigned char	SenseKeySpecific[3];
	} SENSE_DATA, *PSENSE_DATA;

/* structure that returns the capacity data */
typedef struct{
	unsigned char	LogicalBlockAddr[4];
	unsigned char	BlockLength[3];
	} READ_CAPACITY_DATA,*PREAD_CAPACITY_DATA;

/* the SPACE_CODES. This is where you can space to */
typedef enum{
	TAPE_BLOCKS	= 0x00,
	FILEMARKS	= 0x01,
	END_OF_DATA	= 0x03,
	SETMARKS	= 0x04
	} SPACE_CODE;

/* this structure is needed for the DeviceIoControl commands */
typedef struct _SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER {
	SCSI_PASS_THROUGH_DIRECT	sptd;
	ULONG				Filler;      // realign buffer to double word boundary
	SENSE_DATA			senseData;		
	} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;


/*-------------------------------------------------*
 *  application functions
 */
int	FindSCSIDevices(LIBRARY **library);

/*-------------------------------------------------* 
 * Functions for calling the scsi commands
 */

/* Generic */
int	Read(HANDLE fileHandle,unsigned long blockNumber,char *buffer,int size);
int	Write(HANDLE fileHandle,char *buffer,int size);
int	Inquiry(HANDLE fileHandle,char *buffer,char pageCode,int bufferLength);
int	ModeSense(HANDLE fileHandle,unsigned char page,char *buffer,int bufferSize);
int	ModeSelect(HANDLE fileHandle,unsigned char page,char *buffer,int bufferSize);

/* serial device */
int	Space(HANDLE fileHandle,SPACE_CODE code,int number);
int	Unload(HANDLE fileHandle);
int	Rewind(HANDLE fileHandle);

/* autochanger */
int	ReadElementStatus(HANDLE fileHandle,char pageType,unsigned char *buffer,int bufferSize);
int	MoveMedium(HANDLE fileHandle,unsigned short source,unsigned short target,unsigned short transport);

/* Block Device */
int	ReadCapacity(HANDLE fileHandle,int firstLBA,char *buffer);
int	Write_10(HANDLE fileHandle,unsigned long firstLBA,char *buffer,int size);
int	WriteAndVerify(HANDLE fileHandle,unsigned long firstLBA,char *buffer,int size);

/*---------------------------------------------------* 
 * Globals that are used by all functions. 
 */
extern	SENSE_DATA	lastSenseCode;

#endif
			   




