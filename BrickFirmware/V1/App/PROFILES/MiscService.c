/**********************************************************************************************
 * Filename:       MiscService.c
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

#include "MiscService.h"

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

// MiscService Service UUID
CONST uint8_t MiscServiceUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(MISCSERVICE_SERV_UUID), HI_UINT16(MISCSERVICE_SERV_UUID)
};

// SetupID UUID
CONST uint8_t MiscService_SetupIDUUID[ATT_UUID_SIZE] =
{
  TI_BASE_UUID_128(MISCSERVICE_SETUPID_UUID)
};
// FinishConnection UUID
CONST uint8_t MiscService_FinishConnectionUUID[ATT_UUID_SIZE] =
{
  TI_BASE_UUID_128(MISCSERVICE_FINISHCONNECTION_UUID)
};

/*********************************************************************
 * LOCAL VARIABLES
 */

static MiscServiceCBs_t *pAppCBs = NULL;

/*********************************************************************
* Profile Attributes - variables
*/

// Service declaration
static CONST gattAttrType_t MiscServiceDecl = { ATT_BT_UUID_SIZE, MiscServiceUUID };

// Characteristic "SetupID" Properties (for declaration)
static uint8_t MiscService_SetupIDProps = GATT_PROP_WRITE;

// Characteristic "SetupID" Value variable
static uint8_t MiscService_SetupIDVal[MISCSERVICE_SETUPID_LEN] = {0};
// Characteristic "FinishConnection" Properties (for declaration)
static uint8_t MiscService_FinishConnectionProps = GATT_PROP_WRITE;

// Characteristic "FinishConnection" Value variable
static uint8_t MiscService_FinishConnectionVal[MISCSERVICE_FINISHCONNECTION_LEN] = {0};

/*********************************************************************
* Profile Attributes - Table
*/

static gattAttribute_t MiscServiceAttrTbl[] =
{
  // MiscService Service Declaration
  {
    { ATT_BT_UUID_SIZE, primaryServiceUUID },
    GATT_PERMIT_READ,
    0,
    (uint8_t *)&MiscServiceDecl
  },
    // SetupID Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &MiscService_SetupIDProps
    },
      // SetupID Characteristic Value
      {
        { ATT_UUID_SIZE, MiscService_SetupIDUUID },
        GATT_PERMIT_WRITE,
        0,
        MiscService_SetupIDVal
      },
    // FinishConnection Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &MiscService_FinishConnectionProps
    },
      // FinishConnection Characteristic Value
      {
        { ATT_UUID_SIZE, MiscService_FinishConnectionUUID },
        GATT_PERMIT_WRITE,
        0,
        MiscService_FinishConnectionVal
      },
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t MiscService_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                           uint8 *pValue, uint16 *pLen, uint16 offset,
                                           uint16 maxLen, uint8 method );
static bStatus_t MiscService_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                            uint8 *pValue, uint16 len, uint16 offset,
                                            uint8 method );

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Simple Profile Service Callbacks
CONST gattServiceCBs_t MiscServiceCBs =
{
  MiscService_ReadAttrCB,  // Read callback function pointer
  MiscService_WriteAttrCB, // Write callback function pointer
  NULL                       // Authorization callback function pointer
};

/*********************************************************************
* PUBLIC FUNCTIONS
*/

/*
 * MiscService_AddService- Initializes the MiscService service by registering
 *          GATT attributes with the GATT server.
 *
 */
bStatus_t MiscService_AddService( void )
{
  uint8_t status;

  // Register GATT attribute list and CBs with GATT Server App
  status = GATTServApp_RegisterService( MiscServiceAttrTbl,
                                        GATT_NUM_ATTRS( MiscServiceAttrTbl ),
                                        GATT_MAX_ENCRYPT_KEY_SIZE,
                                        &MiscServiceCBs );

  return ( status );
}

/*
 * MiscService_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
bStatus_t MiscService_RegisterAppCBs( MiscServiceCBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    pAppCBs = appCallbacks;

    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
}

/*
 * MiscService_SetParameter - Set a MiscService parameter.
 *
 *    param - Profile parameter ID
 *    len - length of data to right
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
bStatus_t MiscService_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    default:
      ret = INVALIDPARAMETER;
      break;
  }
  return ret;
}


/*
 * MiscService_GetParameter - Get a MiscService parameter.
 *
 *    param - Profile parameter ID
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
bStatus_t MiscService_GetParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case MISCSERVICE_SETUPID:
      memcpy(value, MiscService_SetupIDVal, MISCSERVICE_SETUPID_LEN);
      break;

    case MISCSERVICE_FINISHCONNECTION:
      memcpy(value, MiscService_FinishConnectionVal, MISCSERVICE_FINISHCONNECTION_LEN);
      break;

    default:
      ret = INVALIDPARAMETER;
      break;
  }
  return ret;
}


/*********************************************************************
 * @fn          MiscService_ReadAttrCB
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
static bStatus_t MiscService_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                       uint8 *pValue, uint16 *pLen, uint16 offset,
                                       uint16 maxLen, uint8 method )
{
  bStatus_t status = SUCCESS;



  return status;
}


/*********************************************************************
 * @fn      MiscService_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 * @param   method - type of write message
 *
 * @return  SUCCESS, blePending or Failure
 */
static bStatus_t MiscService_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                        uint8 *pValue, uint16 len, uint16 offset,
                                        uint8 method )
{
  bStatus_t status  = SUCCESS;

  if ( ! memcmp(pAttr->type.uuid, MiscService_SetupIDUUID, pAttr->type.len) )
  {
    if ( offset + len > MISCSERVICE_SETUPID_LEN )
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
        memcpy(pAttr->pValue + offset, pValue, len);
        SimplePeripheral_enqueueMsg(EVENT_UPDATE_BRICKID, NULL);
    }
  }
  // See if request is regarding the FinishConnection Characteristic Value
  else if ( ! memcmp(pAttr->type.uuid, MiscService_FinishConnectionUUID, pAttr->type.len) )
  {
    if ( offset + len > MISCSERVICE_FINISHCONNECTION_LEN )
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
        Display_printf(dispHandle,0,0,"Disconnection requested by writing to attribute.");
        GAP_TerminateLinkReq(0x0000, HCI_DISCONNECT_REMOTE_USER_TERM);
    }
  }
  else
  {
    // If we get here, that means you've forgotten to add an if clause for a
    // characteristic value attribute in the attribute table that has WRITE permissions.
    status = ATT_ERR_ATTR_NOT_FOUND;
  }


  return status;
}
