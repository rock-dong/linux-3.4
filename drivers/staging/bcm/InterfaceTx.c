/*
 * drivers/staging/bcm/InterfaceTx.c
 *
 * Copyright (c) 2016 Allwinnertech Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */
#include "headers.h"

/*this is transmit call-back(BULK OUT)*/
static void write_bulk_callback(struct urb *urb/*, struct pt_regs *regs*/)
{
	PUSB_TCB pTcb= (PUSB_TCB)urb->context;
	PS_INTERFACE_ADAPTER psIntfAdapter = pTcb->psIntfAdapter;
	CONTROL_MESSAGE *pControlMsg = (CONTROL_MESSAGE *)urb->transfer_buffer;
	PMINI_ADAPTER psAdapter = psIntfAdapter->psAdapter ;
	BOOLEAN bpowerDownMsg = FALSE ;
    PMINI_ADAPTER Adapter = GET_BCM_ADAPTER(gblpnetdev);

    if (unlikely(netif_msg_tx_done(Adapter)))
	    pr_info(PFX "%s: transmit status %d\n", Adapter->dev->name, urb->status);

	if(urb->status != STATUS_SUCCESS)
	{
		if(urb->status == -EPIPE)
		{
			psIntfAdapter->psAdapter->bEndPointHalted = TRUE ;
			wake_up(&psIntfAdapter->psAdapter->tx_packet_wait_queue);
		}
		else
		{
			BCM_DEBUG_PRINT(Adapter,DBG_TYPE_TX, NEXT_SEND, DBG_LVL_ALL,"Tx URB has got cancelled. status :%d", urb->status);
		}
	}

	pTcb->bUsed = FALSE;
	atomic_dec(&psIntfAdapter->uNumTcbUsed);



	if(TRUE == psAdapter->bPreparingForLowPowerMode)
	{

		if(((pControlMsg->szData[0] == GO_TO_IDLE_MODE_PAYLOAD) &&
			(pControlMsg->szData[1] == TARGET_CAN_GO_TO_IDLE_MODE)))

		{
			bpowerDownMsg = TRUE ;
			//This covers the bus err while Idle Request msg sent down.
			if(urb->status != STATUS_SUCCESS)
			{
				psAdapter->bPreparingForLowPowerMode = FALSE ;
				BCM_DEBUG_PRINT(Adapter,DBG_TYPE_TX, NEXT_SEND, DBG_LVL_ALL,"Idle Mode Request msg failed to reach to Modem");
				//Signalling the cntrl pkt path in Ioctl
				wake_up(&psAdapter->lowpower_mode_wait_queue);
				StartInterruptUrb(psIntfAdapter);
				goto err_exit;
			}

			if(psAdapter->bDoSuspend == FALSE)
			{
				psAdapter->IdleMode = TRUE;
				//since going in Idle mode completed hence making this var false;
				psAdapter->bPreparingForLowPowerMode = FALSE ;

				BCM_DEBUG_PRINT(Adapter,DBG_TYPE_TX, NEXT_SEND, DBG_LVL_ALL, "Host Entered in Idle Mode State...");
				//Signalling the cntrl pkt path in Ioctl
				wake_up(&psAdapter->lowpower_mode_wait_queue);
			}

		}
		else if((pControlMsg->Leader.Status == LINK_UP_CONTROL_REQ) &&
			(pControlMsg->szData[0] == LINK_UP_ACK) &&
			(pControlMsg->szData[1] == LINK_SHUTDOWN_REQ_FROM_FIRMWARE)  &&
			(pControlMsg->szData[2] == SHUTDOWN_ACK_FROM_DRIVER))
		{
			//This covers the bus err while shutdown Request msg sent down.
			if(urb->status != STATUS_SUCCESS)
			{
				psAdapter->bPreparingForLowPowerMode = FALSE ;
				BCM_DEBUG_PRINT(Adapter,DBG_TYPE_TX, NEXT_SEND, DBG_LVL_ALL,"Shutdown Request Msg failed to reach to Modem");
				//Signalling the cntrl pkt path in Ioctl
				wake_up(&psAdapter->lowpower_mode_wait_queue);
				StartInterruptUrb(psIntfAdapter);
				goto err_exit;
			}

			bpowerDownMsg = TRUE ;
			if(psAdapter->bDoSuspend == FALSE)
			{
				psAdapter->bShutStatus = TRUE;
				//since going in shutdown mode completed hence making this var false;
				psAdapter->bPreparingForLowPowerMode = FALSE ;
				BCM_DEBUG_PRINT(Adapter,DBG_TYPE_TX, NEXT_SEND, DBG_LVL_ALL,"Host Entered in shutdown Mode State...");
				//Signalling the cntrl pkt path in Ioctl
				wake_up(&psAdapter->lowpower_mode_wait_queue);
			}
		}

		if(psAdapter->bDoSuspend && bpowerDownMsg)
		{
			//issuing bus suspend request
			BCM_DEBUG_PRINT(Adapter,DBG_TYPE_TX, NEXT_SEND, DBG_LVL_ALL,"Issuing the Bus suspend request to USB stack");
			psIntfAdapter->bPreparingForBusSuspend = TRUE;
			schedule_work(&psIntfAdapter->usbSuspendWork);

		}

	}

err_exit :
	usb_free_coherent(urb->dev, urb->transfer_buffer_length,
 			urb->transfer_buffer, urb->transfer_dma);
}


static PUSB_TCB GetBulkOutTcb(PS_INTERFACE_ADAPTER psIntfAdapter)
{
	PUSB_TCB pTcb = NULL;
	UINT index = 0;

	if((atomic_read(&psIntfAdapter->uNumTcbUsed) < MAXIMUM_USB_TCB) &&
		(psIntfAdapter->psAdapter->StopAllXaction ==FALSE))
	{
		index = atomic_read(&psIntfAdapter->uCurrTcb);
		pTcb = &psIntfAdapter->asUsbTcb[index];
		pTcb->bUsed = TRUE;
		pTcb->psIntfAdapter= psIntfAdapter;
		BCM_DEBUG_PRINT(psIntfAdapter->psAdapter,DBG_TYPE_TX, NEXT_SEND, DBG_LVL_ALL, "Got Tx desc %d used %d",
			index, atomic_read(&psIntfAdapter->uNumTcbUsed));
		index = (index + 1) % MAXIMUM_USB_TCB;
		atomic_set(&psIntfAdapter->uCurrTcb, index);
		atomic_inc(&psIntfAdapter->uNumTcbUsed);
	}
	return pTcb;
}

static int TransmitTcb(PS_INTERFACE_ADAPTER psIntfAdapter, PUSB_TCB pTcb, PVOID data, int len)
{

	struct urb *urb = pTcb->urb;
	int retval = 0;

	urb->transfer_buffer = usb_alloc_coherent(psIntfAdapter->udev, len,
 						GFP_ATOMIC, &urb->transfer_dma);
	if (!urb->transfer_buffer)
	{
		BCM_DEBUG_PRINT(psIntfAdapter->psAdapter,DBG_TYPE_PRINTK, 0, 0, "Error allocating memory\n");
		return  -ENOMEM;
	}
	memcpy(urb->transfer_buffer, data, len);
	urb->transfer_buffer_length = len;

	BCM_DEBUG_PRINT(psIntfAdapter->psAdapter,DBG_TYPE_TX, NEXT_SEND, DBG_LVL_ALL, "Sending Bulk out packet\n");
	//For T3B,INT OUT end point will be used as bulk out end point
	if((psIntfAdapter->psAdapter->chip_id == T3B) && (psIntfAdapter->bHighSpeedDevice == TRUE))
	{
		usb_fill_int_urb(urb, psIntfAdapter->udev,
	    	psIntfAdapter->sBulkOut.bulk_out_pipe,
			urb->transfer_buffer, len, write_bulk_callback, pTcb,
			psIntfAdapter->sBulkOut.int_out_interval);
	}
	else
	{
	usb_fill_bulk_urb(urb, psIntfAdapter->udev,
		  psIntfAdapter->sBulkOut.bulk_out_pipe,
		  urb->transfer_buffer, len, write_bulk_callback, pTcb);
	}
	urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP; /* For DMA transfer */

	if(FALSE == psIntfAdapter->psAdapter->device_removed &&
	   FALSE == psIntfAdapter->psAdapter->bEndPointHalted &&
	   FALSE == psIntfAdapter->bSuspended &&
	   FALSE == psIntfAdapter->bPreparingForBusSuspend)
	{
		retval = usb_submit_urb(urb, GFP_ATOMIC);
		if (retval)
		{
			BCM_DEBUG_PRINT(psIntfAdapter->psAdapter,DBG_TYPE_TX, NEXT_SEND, DBG_LVL_ALL, "failed submitting write urb, error %d", retval);
			if(retval == -EPIPE)
			{
				psIntfAdapter->psAdapter->bEndPointHalted = TRUE ;
				wake_up(&psIntfAdapter->psAdapter->tx_packet_wait_queue);
			}
		}
	}
	return retval;
}

int InterfaceTransmitPacket(PVOID arg, PVOID data, UINT len)
{
	PUSB_TCB pTcb= NULL;

	PS_INTERFACE_ADAPTER psIntfAdapter = (PS_INTERFACE_ADAPTER)arg;
	pTcb= GetBulkOutTcb(psIntfAdapter);
	if(pTcb == NULL)
	{
		BCM_DEBUG_PRINT(psIntfAdapter->psAdapter,DBG_TYPE_PRINTK, 0, 0, "No URB to transmit packet, dropping packet");
		return -EFAULT;
	}
	return TransmitTcb(psIntfAdapter, pTcb, data, len);
}


