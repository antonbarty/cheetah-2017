//---------------------------------------------------------------------------------
// Title         : Kernel Module For PGP To PCI Bridge Card
// Project       : PGP To PCI-E Bridge Card
//---------------------------------------------------------------------------------
// File          : PgpCardMode.h
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
#include <linux/types.h>

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

   // PGP 0 Status
   __u32 Pgp0LoopBack;
   __u32 Pgp0RxReset;
   __u32 Pgp0TxReset;
   __u32 Pgp0LocLinkReady;
   __u32 Pgp0RemLinkReady;
   __u32 Pgp0RxReady;
   __u32 Pgp0TxReady;
   __u32 Pgp0RxCount;
   __u32 Pgp0CellErrCnt;
   __u32 Pgp0LinkDownCnt;
   __u32 Pgp0LinkErrCnt;
   __u32 Pgp0FifoErr;

   // PGP 1 Status
   __u32 Pgp1LoopBack;
   __u32 Pgp1RxReset;
   __u32 Pgp1TxReset;
   __u32 Pgp1LocLinkReady;
   __u32 Pgp1RemLinkReady;
   __u32 Pgp1RxReady;
   __u32 Pgp1TxReady;
   __u32 Pgp1RxCount;
   __u32 Pgp1CellErrCnt;
   __u32 Pgp1LinkDownCnt;
   __u32 Pgp1LinkErrCnt;
   __u32 Pgp1FifoErr;

   // PGP 2 Status
   __u32 Pgp2LoopBack;
   __u32 Pgp2RxReset;
   __u32 Pgp2TxReset;
   __u32 Pgp2LocLinkReady;
   __u32 Pgp2RemLinkReady;
   __u32 Pgp2RxReady;
   __u32 Pgp2TxReady;
   __u32 Pgp2RxCount;
   __u32 Pgp2CellErrCnt;
   __u32 Pgp2LinkDownCnt;
   __u32 Pgp2LinkErrCnt;
   __u32 Pgp2FifoErr;

   // PGP 3 Status
   __u32 Pgp3LoopBack;
   __u32 Pgp3RxReset;
   __u32 Pgp3TxReset;
   __u32 Pgp3LocLinkReady;
   __u32 Pgp3RemLinkReady;
   __u32 Pgp3RxReady;
   __u32 Pgp3TxReady;
   __u32 Pgp3RxCount;
   __u32 Pgp3CellErrCnt;
   __u32 Pgp3LinkDownCnt;
   __u32 Pgp3LinkErrCnt;
   __u32 Pgp3FifoErr;

   // TX Descriptor Status
   __u32 TxDma3AFull;
   __u32 TxDma2AFull;
   __u32 TxDma1AFull;
   __u32 TxDma0AFull;
   __u32 TxReadReady;
   __u32 TxRetFifoCount;
   __u32 TxCount;
   __u32 TxWrite;
   __u32 TxRead;

   // RX Descriptor Status
   __u32 RxFreeEmpty;
   __u32 RxFreeFull;
   __u32 RxFreeValid;
   __u32 RxFreeFifoCount;
   __u32 RxReadReady;
   __u32 RxRetFifoCount;
   __u32 RxCount;
   __u32 RxWrite;
   __u32 RxRead;

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
