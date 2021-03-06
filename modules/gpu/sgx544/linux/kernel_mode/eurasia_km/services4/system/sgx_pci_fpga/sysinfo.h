/*
 * modules/gpu/sgx544/linux/kernel_mode/eurasia_km/services4/system/sgx_pci_fpga/sysinfo.h
 *
 * Copyright (c) 2016 Allwinnertech Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */
/*************************************************************************/ /*!
@Title          System Description Header
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    This header provides system-specific declarations and macros
@License        Dual MIT/GPLv2

The contents of this file are subject to the MIT license as set out below.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

Alternatively, the contents of this file may be used under the terms of
the GNU General Public License Version 2 ("GPL") in which case the provisions
of GPL are applicable instead of those above.

If you wish to allow use of your version of this file only under the terms of
GPL, and not to allow others to use your version of this file under the terms
of the MIT license, indicate your decision by deleting the provisions above
and replace them with the notice and other provisions required by GPL as set
out in the file called "GPL-COPYING" included in this distribution. If you do
not delete the provisions above, a recipient may use your version of this file
under the terms of either the MIT license or GPL.

This License is also included in this distribution in the file called
"MIT-COPYING".

EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/ /**************************************************************************/

#if !defined(__SYSINFO_H__)
#define __SYSINFO_H__

#define SYS_SGX_CLOCK_SPEED					(16000000)	/* 16 MHz */
/*!< System specific poll/timeout details */
#define MAX_HW_TIME_US				(5000000)
#define WAIT_TRY_COUNT				(10000)

#define SYS_DEVICE_COUNT 3 /* SGX, SGXDISPLAY (external) */

#define PVRPDP_STR1SURF			0x0000
#define PVRPDP_STR1ADDRCTRL		0x0004
#define PVRPDP_STR1POSN			0x0008
#define PVRPDP_MEMCTRL			0x000C
#define PVRPDP_STRCTRL			0x0010
#define PVRPDP_SYNCTRL			0x0014
#define PVRPDP_BORDCOL			0x0018
#define PVRPDP_UPDCTRL			0x001C
#define PVRPDP_HSYNC1			0x0020
#define PVRPDP_HSYNC2			0x0024
#define PVRPDP_HSYNC3			0x0028
#define PVRPDP_VSYNC1			0x002C
#define PVRPDP_VSYNC2			0x0030
#define PVRPDP_VSYNC3			0x0034
#define PVRPDP_HDECTRL			0x0038
#define PVRPDP_VDECTRL			0x003C
#define PVRPDP_VEVENT			0x0040

#define PVRPDP_STR1SURF_STRFORMAT_SHIFT			24
#define PVRPDP_STR1SURF_FORMAT_ARGB8888			0xE

#define PVRPDP_STR1SURF_STRWIDTH_SHIFT			11
#define PVRPDP_STR1SURF_STRHEIGHT_SHIFT			0

#define PVRPDP_STR1ADDRCTRL_ADDR_ALIGNSHIFT		4
#define PVRPDP_STR1ADDRCTRL_STREAMENABLE		0x80000000

#define PVRPDP_STR1POSN_STRIDE_ALIGNSHIFT		4

#define PVRPDP_WIDTH		640
#define PVRPDP_HEIGHT		480
#define PVRPDP_STRIDE		(640 * 4)
#define PVRPDP_PIXELFORMAT	PVRPDP_STR1SURF_FORMAT_ARGB8888


#if !defined(USE_CODE)
PVRSRV_ERROR SysInitRegisters(IMG_VOID);
IMG_VOID SysResetSGX(IMG_PVOID pvRegsBaseKM);

#if defined(READ_TCF_HOST_MEM_SIGNATURE)
PVRSRV_ERROR SysReadTCFHostMemSig(IMG_PVOID pvRegsBaseKM);
PVRSRV_ERROR SysTestTCFHostMemSig(IMG_CPU_PHYADDR sRegRegion,
								  IMG_CPU_PHYADDR sMemRegion);
#endif
#endif /* !defined(USE_CODE) */


#endif	/* __SYSINFO_H__ */
