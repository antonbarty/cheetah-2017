//---------------------------------------------------------------------------------
// Title         : Kernel Module For PGP To PCI Bridge Card
// Project       : PGP To PCI-E Bridge Card
//---------------------------------------------------------------------------------
// File          : PgpCardMod.h
// Author        : Ryan Herbst, rherbst@slac.stanford.edu
// Created       : 05/18/2010
//---------------------------------------------------------------------------------
//
//---------------------------------------------------------------------------------
// Copyright (c) 2010 by SLAC National Accelerator Laboratory. All rights reserved.
//---------------------------------------------------------------------------------
// Modification history:
// 05/18/2010: created.
//---------------------------------------------------------------------------------
#ifndef PGP_CARD_MOD_H
#define PGP_CARD_MOD_H

#include <linux/types.h>

#ifndef NUMBER_OF_LANES
#define NUMBER_OF_LANES (4)
#endif

// Return values
#define SUCCESS 0
#define ERROR   -1

// Scratchpad write value
#define SPAD_WRITE 0x55441122

// TX Structure
typedef struct {

   __u32  model; // large=8, small=4
   __u32  cmd; // ioctl commands
   __u32* data;
   // Lane & VC
   __u32  pgpLane;
   __u32  pgpVc;

   // Data
   __u32   size;  // dwords

} PgpCardTx;

// RX Structure
typedef struct {
    __u32   model; // large=8, small=4
    __u32   maxSize; // dwords
    __u32*  data;

   // Lane & VC
   __u32    pgpLane;
   __u32    pgpVc;

   // Data
   __u32   rxSize;  // dwords

   // Error flags
   __u32   eofe;
   __u32   fifoErr;
   __u32   lengthErr;

} PgpCardRx;

typedef struct {
    __u32 PgpLoopBack;
    __u32 PgpRxReset;
    __u32 PgpTxReset;
    __u32 PgpLocLinkReady;
    __u32 PgpRemLinkReady;
    __u32 PgpRxReady;
    __u32 PgpTxReady;
    __u32 PgpRxCount;
    __u32 PgpCellErrCnt;
    __u32 PgpLinkDownCnt;
    __u32 PgpLinkErrCnt;
    __u32 PgpFifoErr;
} PgpCardLinkStatus;

// Status Structure
typedef struct {

   // General Status
   __u32 Version;

   // Scratchpad
   __u32 ScratchPad;

   // PCI Status & Control Registers
   __u32 PciCommand;
   __u32 PciStatus;
   __u32 PciDCommand;
   __u32 PciDStatus;
   __u32 PciLCommand;
   __u32 PciLStatus;
   __u32 PciLinkState;
   __u32 PciFunction;
   __u32 PciDevice;
   __u32 PciBus;

   PgpCardLinkStatus PgpLink[4];

   // TX Descriptor Status
   __u32 TxDma3AFull;
   __u32 TxDma2AFull;
   __u32 TxDma1AFull;
   __u32 TxDma0AFull;
   __u32 TxReadReady;
   __u32 TxRetFifoCount;
   __u32 TxCount;
   __u32 TxBufferCount;
   __u32 TxRead;

   // RX Descriptor Status
   __u32 RxFreeEmpty;
   __u32 RxFreeFull;
   __u32 RxFreeValid;
   __u32 RxFreeFifoCount;
   __u32 RxReadReady;
   __u32 RxRetFifoCount;
   __u32 RxCount;
   __u32 RxBufferCount;
   __u32 RxWrite[4];
   __u32 RxRead[4];

} PgpCardStatus;

// IO Control Commands

// Normal Write commmand
#define IOCTL_Normal_Write 0

// Read Status, Pass PgpCardStatus as arg
#define IOCTL_Read_Status 0x01

// Set Debug, Pass Debug Value As Arg
#define IOCTL_Set_Debug 0x02

// Set RX Reset, Pass PGP Channel As Arg
#define IOCTL_Set_Rx_Reset 0x03
#define IOCTL_Clr_Rx_Reset 0x04

// Set TX Reset, Pass PGP Channel As Arg
#define IOCTL_Set_Tx_Reset 0x05
#define IOCTL_Clr_Tx_Reset 0x06

// Set Loopback, Pass PGP Channel As Arg
#define IOCTL_Set_Loop 0x07
#define IOCTL_Clr_Loop 0x08

// Reset counters
#define IOCTL_Count_Reset 0x09

// Dump debug
#define IOCTL_Dump_Debug 0x0A

#define IOCTL_Clear_Open_Clients 0xB

#define IOCTL_Clear_Polling      0xC

#endif
