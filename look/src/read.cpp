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

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "scsidata.h"

int	Read(HANDLE fileHandle,unsigned long blockNumber,char *buffer,int size)
{
	int					status,result,blocks;
	DWORD					returned;
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER	commandBuffer;

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
	status = DeviceIoControl(fileHandle,
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
		
