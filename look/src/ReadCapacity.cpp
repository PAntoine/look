/*********************************************************************************
 * Name: ReadCapacity
 *
 * Description:
 *  This function reads the device capacity.
 *
 * Author: P.Antoine
 * Date:   25th September 2001
 *
 *********************************************************************************/

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "scsidata.h"

int	ReadCapacity(HANDLE fileHandle,int firstLBA,char *buffer)
{
	int					status=0,result;
	DWORD					returned;
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER	commandBuffer;

	ZeroMemory(&commandBuffer,sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));
	
	/* set up the command */
	commandBuffer.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
	commandBuffer.sptd.SenseInfoLength = sizeof(SENSE_DATA);
	commandBuffer.sptd.DataIn   = SCSI_IOCTL_DATA_IN;
	commandBuffer.sptd.DataTransferLength = 8;
	commandBuffer.sptd.TimeOutValue = 60;
	commandBuffer.sptd.DataBuffer = buffer;
	commandBuffer.sptd.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,senseData);

	/* now the command itself */
	commandBuffer.sptd.CdbLength = 10;
	commandBuffer.sptd.Cdb[0] = READ_CAPACITY;
	commandBuffer.sptd.Cdb[1] = ( 0 | 0);	/* LUN = lun and RelAddr = 0 */
	commandBuffer.sptd.Cdb[2] = GetByte4(firstLBA);			/* logical block address */
	commandBuffer.sptd.Cdb[3] = GetByte3(firstLBA);			/* logical block address */
	commandBuffer.sptd.Cdb[4] = GetByte2(firstLBA);
	commandBuffer.sptd.Cdb[5] = GetByte1(firstLBA);
	commandBuffer.sptd.Cdb[6] = 0;					/* reserved */
	commandBuffer.sptd.Cdb[7] = 0;					/* reserved */
	commandBuffer.sptd.Cdb[8] = 0;					/* reserved */
	commandBuffer.sptd.Cdb[9] = 0;					/* control byte */

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
		printf("Sense: %2x ASC: %2x ASCQ: %2x status: %d\n",
			commandBuffer.senseData.sense_key,
			commandBuffer.senseData.ASC,
			commandBuffer.senseData.ASCQ,
			GetLastError());
		
		/* save the result code */
		memcpy(&lastSenseCode,&commandBuffer.senseData,sizeof(SENSE_DATA));
	
		return 0;
	}else{
		return 1;
	}
}
		
