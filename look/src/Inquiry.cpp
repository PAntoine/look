/*********************************************************************************
 * Name: Inquiry
 *
 * Description:
 *  This function reads the device details.
 *
 * Author: P.Antoine
 * Date:   25th September 2001
 *
 *********************************************************************************/

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "scsidata.h"

int	Inquiry(HANDLE fileHandle,char *buffer,char pageCode,int bufferLength)
{
	int					status,result;
	char					LUN_EVPD = 0;
	DWORD					returned;
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER	commandBuffer;

	ZeroMemory(&commandBuffer,sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));

	if (pageCode != 0)
	{
		LUN_EVPD = 1;
	}
	
	/* set up the command */
	commandBuffer.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
	commandBuffer.sptd.SenseInfoLength = sizeof(SENSE_DATA);
	commandBuffer.sptd.DataIn   = SCSI_IOCTL_DATA_IN;
	commandBuffer.sptd.DataTransferLength = bufferLength;
	commandBuffer.sptd.TimeOutValue = 60;
	commandBuffer.sptd.DataBuffer = buffer;
	commandBuffer.sptd.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,senseData);

	/* now the command itself */
	commandBuffer.sptd.CdbLength = 6;
	commandBuffer.sptd.Cdb[0] = INQUIRY;
	commandBuffer.sptd.Cdb[1] = LUN_EVPD;			/* LUN = lun, (CmdDT | EVPD) = 00 = std. Inq. data */
	commandBuffer.sptd.Cdb[2] = pageCode;			/* page code / operation code */
	commandBuffer.sptd.Cdb[3] = 0;				/* reserved */
	commandBuffer.sptd.Cdb[4] = bufferLength;		/* allocation length */
	commandBuffer.sptd.Cdb[5] = 0;				/* control byte */

	/* lets send the command */
	status = DeviceIoControl(
			fileHandle,
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
		
