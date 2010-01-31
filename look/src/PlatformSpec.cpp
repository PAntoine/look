/*--------------------------------------------------------------------------------*
 * Name:	PlatformSpecfic
 * Description:
 * 	These functions are part of the look program. All these functions are 
 * 	platform specific. This is to control the amount of condition compilations
 * 	to as few files as possible.
 *
 * Author:	Peter Antoine.
 * Date:	30 Aug 2004
 *--------------------------------------------------------------------------------*/

#if defined(_MSC_VER)
#include <windows.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#endif


extern bool				gDiskOk;
extern unsigned int		gDriveMode;
extern unsigned long	gFileSize;
extern unsigned long	gDiskSector;


#if defined(_MSC_VER)
	#include "ScsiData.h"
	SENSE_DATA	lastSenseCode;
	HANDLE		gDeviceHandle;
#endif

/*----------------------------------------*
 * Feature Display.
 * This will report the build version.
 * There are some functions that are not
 * supported on all platforms.
 *
 *----------------------------------------*/
void	FeatureDisplay(void)
{	
#if defined(_MSC_VER)
	if (GetVersion() >= 0x80000000)
	{
		gDiskOk = false;		// disk stuff only works on NT4
		printf("Win 3.x/95/98\n");
	}

	printf("Running microsoft compiled version\n");
#endif
}

/*----------------------------------------*
 * FeatureTidy
 * This will tidy up any platform specific
 * functions.
 *----------------------------------------*/
void	FeatureTidy(void)
{
#if defined(_MSC_VER)
	CloseHandle(gDeviceHandle);
#endif
}

	
/*----------------------------------------*
 * OpenDisk
 * This will report the build version.
 * There are some functions that are not
 * supported on all platforms.
 *
 *----------------------------------------*/
void	OpenDisk(char* progname,char* diskname)
{
#if defined(_MSC_VER)
	bool	failed(false);
	char	string[256],buffer[1024],*temp;
	
	if (GetVersion() >= 0x80000000)
	{
		sprintf(string,"\\\\.\\PHYSICALDRIVE%d",atoi(diskname));
		
		/* we want to open a drive */
		gDeviceHandle = CreateFile(
				string,
				GENERIC_WRITE | GENERIC_READ ,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);

		if (gDeviceHandle == INVALID_HANDLE_VALUE)
		{
			printf("%s: Failed to open file %s\n",progname,diskname);
			failed = true;
		}else{
			Inquiry(gDeviceHandle,buffer,0,96);
			printf("Device is name is : %s\n",&((PINQUIRY_DATA)buffer)->ProductID[0]); 
			
			ReadCapacity(gDeviceHandle,0,buffer);

			temp = (char*) &(((PREAD_CAPACITY_DATA)buffer)->LogicalBlockAddr[0]);
			gFileSize = MakeInt4(temp[0],temp[1],temp[2],temp[3]);
			
			gDriveMode = 1;
			gDiskSector = 0;
		}
	}
#endif	
}

/*********************************************************************************
 * Name: Read
 *
 * Description:
 *  This function reads a block from the tape.
 *
 * Author: P.Antoine
 * Date:   25th Nov 2000
 *
 *********************************************************************************/

int	Read(unsigned long blockNumber,char *buffer,int size)
{
	int		status,result,blocks;
#ifdef _WIN32
	DWORD	returned;
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER	commandBuffer;
	
	if (GetVersion() >= 0x80000000)
	{
		ZeroMemory(&commandBuffer,sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));
		
		/* set up the command */
		commandBuffer.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
		commandBuffer.sptd.SenseInfoLength = sizeof(SENSE_DATA);
		commandBuffer.sptd.DataIn   = SCSI_IOCTL_DATA_IN;
		commandBuffer.sptd.DataTransferLength = size*512;
		commandBuffer.sptd.TimeOutValue = 60;
		commandBuffer.sptd.DataBuffer = buffer;
		commandBuffer.sptd.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,senseData);

		/* now the command itself */
		commandBuffer.sptd.CdbLength = 10;
		commandBuffer.sptd.Cdb[0] = READ_10;
		commandBuffer.sptd.Cdb[1] = (0 | DPO_LOAD_IN_CACHE | FUA_USE_CACHE | RA_ACTUAL_ADDRESS);
		commandBuffer.sptd.Cdb[2] = GetByte4(blockNumber);
		commandBuffer.sptd.Cdb[3] = GetByte3(blockNumber);
		commandBuffer.sptd.Cdb[4] = GetByte2(blockNumber);
		commandBuffer.sptd.Cdb[5] = GetByte1(blockNumber);
		commandBuffer.sptd.Cdb[6] = 0;
		commandBuffer.sptd.Cdb[7] = GetByte2(size);
		commandBuffer.sptd.Cdb[8] = GetByte1(size);
		commandBuffer.sptd.Cdb[9] = 0;			/* control field (allways zero) */

		/* lets send the command */
		status = DeviceIoControl(gDeviceHandle,
					IOCTL_SCSI_PASS_THROUGH_DIRECT,
				&commandBuffer,
				sizeof(SCSI_PASS_THROUGH_DIRECT),
				&commandBuffer,
				sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER),
				&returned,
				NULL);

		/* status of the command */
		if ((status && commandBuffer.sptd.ScsiStatus != 0) || !status)
		{
			printf("read failed.\n");
			
			printf("Sense: %2x ASC: %2x ASCQ: %2x status: %d\n",
				commandBuffer.senseData.sense_key,
				commandBuffer.senseData.ASC,
				commandBuffer.senseData.ASCQ,
				GetLastError());
			
				result = 0;	

			/* save the result code */
			memcpy(&lastSenseCode,&commandBuffer.senseData,sizeof(SENSE_DATA));

			return result;
		}else{
			return 1;
		}
	}
#endif	

	return 0;
}
		

