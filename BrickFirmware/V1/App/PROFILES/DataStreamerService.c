/**********************************************************************************************
 * Filename:       DataStreamerService.c
 *
 * Description:    This file contains the implementation of the service.
 *
 * Copyright (c) 2015-2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************************************/


/*********************************************************************
 * INCLUDES
 */
#include <string.h>

#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"

#include "DataStreamerService.h"

#include <ti/display/Display.h>

#include "GLOBAL_DEFINES.h"

#include <simple_peripheral.h>
#include "util.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
* GLOBAL VARIABLES
*/

// DataStreamerService Service UUID
CONST uint8_t DataStreamerServiceUUID[ATT_UUID_SIZE] =
{
  TI_BASE_UUID_128(DATASTREAMERSERVICE_SERV_UUID)
};

// DataStream UUID
CONST uint8_t DataStreamerService_DataStreamUUID[ATT_UUID_SIZE] =
{
  TI_BASE_UUID_128(DATASTREAMERSERVICE_DATASTREAM_UUID)
};

/*********************************************************************
 * LOCAL VARIABLES
 */
uint8_t dataStreamOutputBuffer[DATASTREAM_MAX_LENGTH] = {0};
bool dataStreamReady=false;
int dataStreamCurrentLength=0;

/*********************************************************************
* Profile Attributes - variables
*/

// Service declaration
static CONST gattAttrType_t DataStreamerServiceDecl = { ATT_UUID_SIZE, DataStreamerServiceUUID };
static uint8_t DataStreamerService_DataStreamProps = GATT_PROP_READ;




/*********************************************************************
* Profile Attributes - Table
*/

static gattAttribute_t DataStreamerServiceAttrTbl[] =
{
  // DataStreamerService Service Declaration
  {
    { ATT_BT_UUID_SIZE, primaryServiceUUID },
    GATT_PERMIT_READ,
    0,
    (uint8_t *)&DataStreamerServiceDecl
  },
    // DataStream Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &DataStreamerService_DataStreamProps
    },
      // DataStream Characteristic Value
      {
        { ATT_UUID_SIZE, DataStreamerService_DataStreamUUID },
        GATT_PERMIT_READ,
        0,
        dataStreamOutputBuffer
      },
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t DataStreamerService_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                           uint8 *pValue, uint16 *pLen, uint16 offset,
                                           uint16 maxLen, uint8 method );

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Simple Profile Service Callbacks
CONST gattServiceCBs_t DataStreamerServiceCBs =
{
  DataStreamerService_ReadAttrCB,  // Read callback function pointer
  NULL, // Write callback function pointer
  NULL  // Authorization callback function pointer
};

/*********************************************************************
* PUBLIC FUNCTIONS
*/

/*
 * DataStreamerService_AddService- Initializes the DataStreamerService service by registering
 *          GATT attributes with the GATT server.
 *
 */
bStatus_t DataStreamerService_AddService( void )
{
  uint8_t status;

  // Register GATT attribute list and CBs with GATT Server App
  status = GATTServApp_RegisterService( DataStreamerServiceAttrTbl,
                                        GATT_NUM_ATTRS( DataStreamerServiceAttrTbl ),
                                        GATT_MAX_ENCRYPT_KEY_SIZE,
                                        &DataStreamerServiceCBs );

  return ( status );
}



/*********************************************************************
 * @fn          DataStreamerService_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 * @param       method - type of read message
 *
 * @return      SUCCESS, blePending or Failure
 */
static bStatus_t DataStreamerService_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                       uint8 *pValue, uint16 *pLen, uint16 offset,
                                       uint16 maxLen, uint8 method )
{
  bStatus_t status = SUCCESS;


  // See if request is regarding the DataStream Characteristic Value
  if (!memcmp(pAttr->type.uuid, DataStreamerService_DataStreamUUID, pAttr->type.len))
  {
      if(dataStreamReady){
          //The data is ready to be streamed, so just stream as usual
        if ( offset > dataStreamCurrentLength )  // Prevent malicious ATT ReadBlob offsets.
        {
            status = ATT_ERR_INVALID_OFFSET;
        }
        else
        {
            *pLen = MIN(maxLen, dataStreamCurrentLength - offset);  // Transmit as much as possible
            memcpy(pValue, pAttr->pValue + offset, *pLen);
        }

        if(*pLen<maxLen){ //Request finished
            Display_printf(dispHandle,0,0,"DataStream request finished.",*pLen);
            dataStreamReady=false; //Data stream already read, set the stream as not ready
            GAP_TerminateLinkReq(0x0000, HCI_DISCONNECT_REMOTE_USER_TERM);
        }else{
            Display_printf(dispHandle,0,0,"Partial stream read request in progress done at offset %d",offset);
        }
      }else{
          //The data is not ready to be streamed (Maybe being modified) Send 0 bytes
          Display_printf(dispHandle,0,0,"DataStream request received but it is not ready!",*pLen);
          *pLen=0;
      }

  }
  else
  {
    // If we get here, that means you've forgotten to add an if clause for a
    // characteristic value attribute in the attribute table that has READ permissions.
    *pLen = 0;
    status = ATT_ERR_ATTR_NOT_FOUND;
  }

  return status;
}


void SetNewDataStreamBegin(){
    dataStreamReady=false;
    SimplePeripheral_enqueueMsg(EVENT_DISABLE_ADVERTISEMENT, NULL);
    //DisableAdvertisement(); We don't want to disable all advertisements as other services might be using them
}

bStatus_t SetNewDataStreamEnd(){
    dataStreamReady=true;
    SimplePeripheral_enqueueMsg(EVENT_ENABLE_ADVERTISEMENT, NULL);
  //  EnableAdvertisement(); //Make sure advertisements are enabled since a new data stream is ready
}
