/******************************************************************************
 *
 *  Copyright (C) 2006-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

/******************************************************************************
 *
 *  This is the implementation of the JAVA API for Bluetooth Wireless
 *  Technology (JABWT) as specified by the JSR82 specificiation
 *
 ******************************************************************************/
#include "bta_api.h"
#include "bta_sys.h"
#include "bta_jv_api.h"
#include "bta_jv_int.h"
#include "gki.h"
#include <string.h>
#include "port_api.h"
#include "sdp_api.h"
#include "utl.h"

/*****************************************************************************
**  Constants
*****************************************************************************/

static const tBTA_SYS_REG bta_jv_reg =
{
    bta_jv_sm_execute,
    NULL
};

/*******************************************************************************
**
** Function         BTA_JvEnable
**
** Description      Enable the Java I/F service. When the enable
**                  operation is complete the callback function will be
**                  called with a BTA_JV_ENABLE_EVT. This function must
**                  be called before other function in the JV API are
**                  called.
**
** Returns          BTA_JV_SUCCESS if successful.
**                  BTA_JV_FAIL if internal failure.
**
*******************************************************************************/
tBTA_JV_STATUS BTA_JvEnable(tBTA_JV_DM_CBACK *p_cback)
{
    tBTA_JV_STATUS status = BTA_JV_FAILURE;
    tBTA_JV_API_ENABLE  *p_buf;
    int i;

    APPL_TRACE_API( "BTA_JvEnable");
    if(p_cback && FALSE == bta_sys_is_register(BTA_ID_JV))
    {
        memset(&bta_jv_cb, 0, sizeof(tBTA_JV_CB));
        /* set handle to invalid value by default */
        for (i=0; i<BTA_JV_PM_MAX_NUM; i++)
        {
            bta_jv_cb.pm_cb[i].handle = BTA_JV_PM_HANDLE_CLEAR;
        }

        /* register with BTA system manager */
        bta_sys_register(BTA_ID_JV, &bta_jv_reg);

        if (p_cback && (p_buf = (tBTA_JV_API_ENABLE *) GKI_getbuf(sizeof(tBTA_JV_API_ENABLE))) != NULL)
        {
            p_buf->hdr.event = BTA_JV_API_ENABLE_EVT;
            p_buf->p_cback = p_cback;
            bta_sys_sendmsg(p_buf);
            status = BTA_JV_SUCCESS;
        }
    }
    else
      {
         APPL_TRACE_ERROR("JVenable fails");
      }
    return(status);
}

/*******************************************************************************
**
** Function         BTA_JvDisable
**
** Description      Disable the Java I/F
**
** Returns          void
**
*******************************************************************************/
void BTA_JvDisable(void)
{
    BT_HDR  *p_buf;

    APPL_TRACE_API( "BTA_JvDisable");
    bta_sys_deregister(BTA_ID_JV);
    if ((p_buf = (BT_HDR *) GKI_getbuf(sizeof(BT_HDR))) != NULL)
    {
        p_buf->event = BTA_JV_API_DISABLE_EVT;
        bta_sys_sendmsg(p_buf);
    }
}

/*******************************************************************************
**
** Function         BTA_JvIsEnable
**
** Description      Get the JV registration status.
**
** Returns          TRUE, if registered
**
*******************************************************************************/
BOOLEAN BTA_JvIsEnable(void)
{
    return bta_sys_is_register(BTA_ID_JV);
}

/*******************************************************************************
**
** Function         BTA_JvIsEncrypted
**
** Description      This function checks if the link to peer device is encrypted
**
** Returns          TRUE if encrypted.
**                  FALSE if not.
**
*******************************************************************************/
BOOLEAN BTA_JvIsEncrypted(BD_ADDR bd_addr)
{
    BOOLEAN is_encrypted = FALSE;
    UINT8 sec_flags, le_flags;

    if (BTM_GetSecurityFlags(bd_addr, &sec_flags) &&
        BTM_GetSecurityFlagsByTransport(bd_addr, &le_flags, BT_TRANSPORT_LE))
    {
        if(sec_flags & BTM_SEC_FLAG_ENCRYPTED ||
           le_flags & BTM_SEC_FLAG_ENCRYPTED)
            is_encrypted = TRUE;
    }
    return is_encrypted;
}

/*******************************************************************************
**
** Function         BTA_JvStartDiscovery
**
** Description      This function performs service discovery for the services
**                  provided by the given peer device. When the operation is
**                  complete the tBTA_JV_DM_CBACK callback function will be
**                  called with a BTA_JV_DISCOVERY_COMP_EVT.
**
** Returns          BTA_JV_SUCCESS, if the request is being processed.
**                  BTA_JV_FAILURE, otherwise.
**
*******************************************************************************/
tBTA_JV_STATUS BTA_JvStartDiscovery(BD_ADDR bd_addr, UINT16 num_uuid,
            tSDP_UUID *p_uuid_list, void * user_data)
{
    tBTA_JV_STATUS status = BTA_JV_FAILURE;
    tBTA_JV_API_START_DISCOVERY *p_msg;

    APPL_TRACE_API( "BTA_JvStartDiscovery");
    if ((p_msg = (tBTA_JV_API_START_DISCOVERY *)GKI_getbuf(sizeof(tBTA_JV_API_START_DISCOVERY))) != NULL)
    {
        p_msg->hdr.event = BTA_JV_API_START_DISCOVERY_EVT;
        bdcpy(p_msg->bd_addr, bd_addr);
        p_msg->num_uuid = num_uuid;
        memcpy(p_msg->uuid_list, p_uuid_list, num_uuid * sizeof(tSDP_UUID));
        p_msg->num_attr = 0;
        p_msg->user_data = user_data;
        bta_sys_sendmsg(p_msg);
        status = BTA_JV_SUCCESS;
    }

    return(status);
}

/*******************************************************************************
**
** Function         BTA_JvCreateRecord
**
** Description      Create a service record in the local SDP database.
**                  When the operation is complete the tBTA_JV_DM_CBACK callback
**                  function will be called with a BTA_JV_CREATE_RECORD_EVT.
**
** Returns          BTA_JV_SUCCESS, if the request is being processed.
**                  BTA_JV_FAILURE, otherwise.
**
*******************************************************************************/
tBTA_JV_STATUS BTA_JvCreateRecordByUser(void *user_data)
{
    tBTA_JV_STATUS status = BTA_JV_FAILURE;
    tBTA_JV_API_CREATE_RECORD *p_msg;

    APPL_TRACE_API( "BTA_JvCreateRecordByUser");
    if ((p_msg = (tBTA_JV_API_CREATE_RECORD *)GKI_getbuf(sizeof(tBTA_JV_API_CREATE_RECORD))) != NULL)
    {
        p_msg->hdr.event = BTA_JV_API_CREATE_RECORD_EVT;
        p_msg->user_data = user_data;
        bta_sys_sendmsg(p_msg);
        status = BTA_JV_SUCCESS;
    }

    return(status);
}

/*******************************************************************************
**
** Function         BTA_JvDeleteRecord
**
** Description      Delete a service record in the local SDP database.
**
** Returns          BTA_JV_SUCCESS, if the request is being processed.
**                  BTA_JV_FAILURE, otherwise.
**
*******************************************************************************/
tBTA_JV_STATUS BTA_JvDeleteRecord(UINT32 handle)
{
    tBTA_JV_STATUS status = BTA_JV_FAILURE;
    tBTA_JV_API_ADD_ATTRIBUTE *p_msg;

    APPL_TRACE_API( "BTA_JvDeleteRecord");
    if ((p_msg = (tBTA_JV_API_ADD_ATTRIBUTE *)GKI_getbuf(sizeof(tBTA_JV_API_ADD_ATTRIBUTE))) != NULL)
    {
        p_msg->hdr.event = BTA_JV_API_DELETE_RECORD_EVT;
        p_msg->handle = handle;
        bta_sys_sendmsg(p_msg);
        status = BTA_JV_SUCCESS;
    }
    return(status);
}

/*******************************************************************************
**
** Function         BTA_JvRfcommConnect
**
** Description      This function makes an RFCOMM conection to a remote BD
**                  Address.
**                  When the connection is initiated or failed to initiate,
**                  tBTA_JV_RFCOMM_CBACK is called with BTA_JV_RFCOMM_CL_INIT_EVT
**                  When the connection is established or failed,
**                  tBTA_JV_RFCOMM_CBACK is called with BTA_JV_RFCOMM_OPEN_EVT
**
** Returns          BTA_JV_SUCCESS, if the request is being processed.
**                  BTA_JV_FAILURE, otherwise.
**
*******************************************************************************/
tBTA_JV_STATUS BTA_JvRfcommConnect(tBTA_SEC sec_mask,
                           tBTA_JV_ROLE role, UINT8 remote_scn, BD_ADDR peer_bd_addr,
                           tBTA_JV_RFCOMM_CBACK *p_cback, void* user_data)
{
    tBTA_JV_STATUS status = BTA_JV_FAILURE;
    tBTA_JV_API_RFCOMM_CONNECT *p_msg;

    APPL_TRACE_API( "BTA_JvRfcommConnect");
    if (p_cback &&
        (p_msg = (tBTA_JV_API_RFCOMM_CONNECT *)GKI_getbuf(sizeof(tBTA_JV_API_RFCOMM_CONNECT))) != NULL)
    {
        p_msg->hdr.event    = BTA_JV_API_RFCOMM_CONNECT_EVT;
        p_msg->sec_mask     = sec_mask;
        p_msg->role         = role;
        p_msg->remote_scn   = remote_scn;
        memcpy(p_msg->peer_bd_addr, peer_bd_addr, sizeof(BD_ADDR));
        p_msg->p_cback      = p_cback;
        p_msg->user_data    = user_data;
        bta_sys_sendmsg(p_msg);
        status = BTA_JV_SUCCESS;
    }

    return(status);
}

/*******************************************************************************
**
** Function         BTA_JvRfcommClose
**
** Description      This function closes an RFCOMM connection
**
** Returns          BTA_JV_SUCCESS, if the request is being processed.
**                  BTA_JV_FAILURE, otherwise.
**
*******************************************************************************/
tBTA_JV_STATUS BTA_JvRfcommClose(UINT32 handle, void *user_data)
{
    tBTA_JV_STATUS status = BTA_JV_FAILURE;
    tBTA_JV_API_RFCOMM_CLOSE *p_msg;
    UINT32  hi = ((handle & BTA_JV_RFC_HDL_MASK)&~BTA_JV_RFCOMM_MASK) - 1;
    UINT32  si = BTA_JV_RFC_HDL_TO_SIDX(handle);

    APPL_TRACE_API( "BTA_JvRfcommClose");
    if (hi < BTA_JV_MAX_RFC_CONN && bta_jv_cb.rfc_cb[hi].p_cback &&
        si < BTA_JV_MAX_RFC_SR_SESSION && bta_jv_cb.rfc_cb[hi].rfc_hdl[si] &&
        (p_msg = (tBTA_JV_API_RFCOMM_CLOSE *)GKI_getbuf(sizeof(tBTA_JV_API_RFCOMM_CLOSE))) != NULL)
    {
        p_msg->hdr.event = BTA_JV_API_RFCOMM_CLOSE_EVT;
        p_msg->handle = handle;
        p_msg->p_cb = &bta_jv_cb.rfc_cb[hi];
        p_msg->p_pcb = &bta_jv_cb.port_cb[p_msg->p_cb->rfc_hdl[si] - 1];
        p_msg->user_data = user_data;
        bta_sys_sendmsg(p_msg);
        status = BTA_JV_SUCCESS;
    }

    return(status);
}

/*******************************************************************************
**
** Function         BTA_JvRfcommStartServer
**
** Description      This function starts listening for an RFCOMM connection
**                  request from a remote Bluetooth device.  When the server is
**                  started successfully, tBTA_JV_RFCOMM_CBACK is called
**                  with BTA_JV_RFCOMM_START_EVT.
**                  When the connection is established, tBTA_JV_RFCOMM_CBACK
**                  is called with BTA_JV_RFCOMM_OPEN_EVT.
**
** Returns          BTA_JV_SUCCESS, if the request is being processed.
**                  BTA_JV_FAILURE, otherwise.
**
*******************************************************************************/
tBTA_JV_STATUS BTA_JvRfcommStartServer(tBTA_SEC sec_mask,
                           tBTA_JV_ROLE role, UINT8 local_scn, UINT8 max_session,
                           tBTA_JV_RFCOMM_CBACK *p_cback, void* user_data)
{
    tBTA_JV_STATUS status = BTA_JV_FAILURE;
    tBTA_JV_API_RFCOMM_SERVER *p_msg;

    APPL_TRACE_API( "BTA_JvRfcommStartServer");
    if (p_cback &&
        (p_msg = (tBTA_JV_API_RFCOMM_SERVER *)GKI_getbuf(sizeof(tBTA_JV_API_RFCOMM_SERVER))) != NULL)
    {
        if (max_session == 0)
            max_session = 1;
        if (max_session > BTA_JV_MAX_RFC_SR_SESSION)
        {
            APPL_TRACE_DEBUG( "max_session is too big. use max (%d)", max_session, BTA_JV_MAX_RFC_SR_SESSION);
            max_session = BTA_JV_MAX_RFC_SR_SESSION;
        }
        p_msg->hdr.event = BTA_JV_API_RFCOMM_START_SERVER_EVT;
        p_msg->sec_mask = sec_mask;
        p_msg->role = role;
        p_msg->local_scn = local_scn;
        p_msg->max_session = max_session;
        p_msg->p_cback = p_cback;
        p_msg->user_data = user_data; //caller's private data
        bta_sys_sendmsg(p_msg);
        status = BTA_JV_SUCCESS;
    }

    return(status);
}

/*******************************************************************************
**
** Function         BTA_JvRfcommStopServer
**
** Description      This function stops the RFCOMM server. If the server has an
**                  active connection, it would be closed.
**
** Returns          BTA_JV_SUCCESS, if the request is being processed.
**                  BTA_JV_FAILURE, otherwise.
**
*******************************************************************************/
tBTA_JV_STATUS BTA_JvRfcommStopServer(UINT32 handle, void * user_data)
{
    tBTA_JV_STATUS status = BTA_JV_FAILURE;
    tBTA_JV_API_RFCOMM_SERVER *p_msg;
    APPL_TRACE_API( "BTA_JvRfcommStopServer");
    if ((p_msg = (tBTA_JV_API_RFCOMM_SERVER *)GKI_getbuf(sizeof(tBTA_JV_API_RFCOMM_SERVER))) != NULL)
    {
        p_msg->hdr.event = BTA_JV_API_RFCOMM_STOP_SERVER_EVT;
        p_msg->handle = handle;
        p_msg->user_data = user_data; //caller's private data
        bta_sys_sendmsg(p_msg);
        status = BTA_JV_SUCCESS;
    }

    return(status);
}

/*******************************************************************************
**
** Function         BTA_JvRfcommRead
**
** Description      This function reads data from an RFCOMM connection
**                  The actual size of data read is returned in p_len.
**
** Returns          BTA_JV_SUCCESS, if the request is being processed.
**                  BTA_JV_FAILURE, otherwise.
**
*******************************************************************************/
tBTA_JV_STATUS BTA_JvRfcommRead(UINT32 handle, UINT32 req_id, UINT8 *p_data, UINT16 len)
{
    tBTA_JV_STATUS status = BTA_JV_FAILURE;
    tBTA_JV_API_RFCOMM_READ *p_msg;
    UINT32  hi = ((handle & BTA_JV_RFC_HDL_MASK)&~BTA_JV_RFCOMM_MASK) - 1;
    UINT32  si = BTA_JV_RFC_HDL_TO_SIDX(handle);

    APPL_TRACE_API( "BTA_JvRfcommRead");
    if (hi < BTA_JV_MAX_RFC_CONN && bta_jv_cb.rfc_cb[hi].p_cback &&
        si < BTA_JV_MAX_RFC_SR_SESSION && bta_jv_cb.rfc_cb[hi].rfc_hdl[si] &&
        (p_msg = (tBTA_JV_API_RFCOMM_READ *)GKI_getbuf(sizeof(tBTA_JV_API_RFCOMM_READ))) != NULL)
    {
        p_msg->hdr.event = BTA_JV_API_RFCOMM_READ_EVT;
        p_msg->handle = handle;
        p_msg->req_id = req_id;
        p_msg->p_data = p_data;
        p_msg->len = len;
        p_msg->p_cb = &bta_jv_cb.rfc_cb[hi];
        p_msg->p_pcb = &bta_jv_cb.port_cb[p_msg->p_cb->rfc_hdl[si] - 1];
        bta_sys_sendmsg(p_msg);
        status = BTA_JV_SUCCESS;
    }

    return(status);
}

/*******************************************************************************
**
** Function         BTA_JvRfcommGetPortHdl
**
** Description    This function fetches the rfcomm port handle
**
** Returns          BTA_JV_SUCCESS, if the request is being processed.
**                  BTA_JV_FAILURE, otherwise.
**
*******************************************************************************/
UINT16 BTA_JvRfcommGetPortHdl(UINT32 handle)
{
    UINT32  hi = ((handle & BTA_JV_RFC_HDL_MASK) & ~BTA_JV_RFCOMM_MASK) - 1;
    UINT32  si = BTA_JV_RFC_HDL_TO_SIDX(handle);

    if (hi < BTA_JV_MAX_RFC_CONN &&
        si < BTA_JV_MAX_RFC_SR_SESSION && bta_jv_cb.rfc_cb[hi].rfc_hdl[si])
        return bta_jv_cb.port_cb[bta_jv_cb.rfc_cb[hi].rfc_hdl[si] - 1].port_handle;
    else
        return 0xffff;
}


/*******************************************************************************
**
** Function         BTA_JvRfcommReady
**
** Description      This function determined if there is data to read from
**                  an RFCOMM connection
**
** Returns          BTA_JV_SUCCESS, if data queue size is in *p_data_size.
**                  BTA_JV_FAILURE, if error.
**
*******************************************************************************/
tBTA_JV_STATUS BTA_JvRfcommReady(UINT32 handle, UINT32 *p_data_size)
{
    tBTA_JV_STATUS status = BTA_JV_FAILURE;
    UINT16          size = 0;
    UINT32  hi = ((handle & BTA_JV_RFC_HDL_MASK)&~BTA_JV_RFCOMM_MASK) - 1;
    UINT32  si = BTA_JV_RFC_HDL_TO_SIDX(handle);

    APPL_TRACE_API( "BTA_JvRfcommReady");
    if (hi < BTA_JV_MAX_RFC_CONN && bta_jv_cb.rfc_cb[hi].p_cback &&
        si < BTA_JV_MAX_RFC_SR_SESSION && bta_jv_cb.rfc_cb[hi].rfc_hdl[si])
    {
        if(PORT_GetRxQueueCnt(bta_jv_cb.rfc_cb[hi].rfc_hdl[si], &size) == PORT_SUCCESS)
        {
            status = BTA_JV_SUCCESS;
        }
    }
    *p_data_size = size;
    return(status);
}

/*******************************************************************************
**
** Function         BTA_JvRfcommWrite
**
** Description      This function writes data to an RFCOMM connection
**
** Returns          BTA_JV_SUCCESS, if the request is being processed.
**                  BTA_JV_FAILURE, otherwise.
**
*******************************************************************************/
tBTA_JV_STATUS BTA_JvRfcommWrite(UINT32 handle, UINT32 req_id)
{
    tBTA_JV_STATUS status = BTA_JV_FAILURE;
    tBTA_JV_API_RFCOMM_WRITE *p_msg;
    UINT32  hi = ((handle & BTA_JV_RFC_HDL_MASK)&~BTA_JV_RFCOMM_MASK) - 1;
    UINT32  si = BTA_JV_RFC_HDL_TO_SIDX(handle);

    APPL_TRACE_API( "BTA_JvRfcommWrite");
    APPL_TRACE_DEBUG( "handle:0x%x, hi:%d, si:%d", handle, hi, si);
    if (hi < BTA_JV_MAX_RFC_CONN && bta_jv_cb.rfc_cb[hi].p_cback &&
        si < BTA_JV_MAX_RFC_SR_SESSION && bta_jv_cb.rfc_cb[hi].rfc_hdl[si] &&
        (p_msg = (tBTA_JV_API_RFCOMM_WRITE *)GKI_getbuf(sizeof(tBTA_JV_API_RFCOMM_WRITE))) != NULL)
    {
        p_msg->hdr.event = BTA_JV_API_RFCOMM_WRITE_EVT;
        p_msg->handle = handle;
        p_msg->req_id = req_id;
        p_msg->p_cb = &bta_jv_cb.rfc_cb[hi];
        p_msg->p_pcb = &bta_jv_cb.port_cb[p_msg->p_cb->rfc_hdl[si] - 1];
        APPL_TRACE_API( "write ok");
        bta_sys_sendmsg(p_msg);
        status = BTA_JV_SUCCESS;
    }

    return(status);
}


/*******************************************************************************
 **
 ** Function    BTA_JVSetPmProfile
 **
 ** Description This function set or free power mode profile for different JV application
 **
 ** Parameters:  handle,  JV handle from RFCOMM or L2CAP
 **              app_id:  app specific pm ID, can be BTA_JV_PM_ALL, see bta_dm_cfg.c for details
 **              BTA_JV_PM_ID_CLEAR: removes pm management on the handle. init_st is ignored and
 **              BTA_JV_CONN_CLOSE is called implicitely
 **              init_st:  state after calling this API. typically it should be BTA_JV_CONN_OPEN
 **
 ** Returns      BTA_JV_SUCCESS, if the request is being processed.
 **              BTA_JV_FAILURE, otherwise.
 **
 ** NOTE:        BTA_JV_PM_ID_CLEAR: In general no need to be called as jv pm calls automatically
 **              BTA_JV_CONN_CLOSE to remove in case of connection close!
 **
 *******************************************************************************/
tBTA_JV_STATUS BTA_JvSetPmProfile(UINT32 handle, tBTA_JV_PM_ID app_id, tBTA_JV_CONN_STATE init_st)
{
    tBTA_JV_STATUS status = BTA_JV_FAILURE;
    tBTA_JV_API_SET_PM_PROFILE *p_msg;

    APPL_TRACE_API("BTA_JVSetPmProfile handle:0x%x, app_id:%d", handle, app_id);
    if ((p_msg = (tBTA_JV_API_SET_PM_PROFILE *)GKI_getbuf(sizeof(tBTA_JV_API_SET_PM_PROFILE)))
        != NULL)
    {
        p_msg->hdr.event = BTA_JV_API_SET_PM_PROFILE_EVT;
        p_msg->handle = handle;
        p_msg->app_id = app_id;
        p_msg->init_st = init_st;
        bta_sys_sendmsg(p_msg);
        status = BTA_JV_SUCCESS;
    }

    return (status);
}
