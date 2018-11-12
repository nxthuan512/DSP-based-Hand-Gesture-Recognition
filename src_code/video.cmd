/*
 *  Copyright 2003 by Texas Instruments Incorporated.
 *  All rights reserved. Property of Texas Instruments Incorporated.
 *  Restricted rights to use, duplicate or disclose this code are
 *  granted through contract.
 *  
 */
/* "@(#) DDK 1.10.00.21 06-26-03 (ddk-b10)" */
/*
 *  Linker-command file for the application
 *
 *  This file governs the linking process. The minimum of information
 *  it needs to contain is:
 *
 *  1) the list of libraries used by the application; they will be
 *     searched for in directories specified in the "build options->
 *     linker->library search path" field of the project
 *  2) inclusion of the configuration file automatically generated
 *     by the configuration database.
 *  
 */
 
/* include config-generated link command file */
-l videocfg.cmd

/* include library for the IOM video driver */
-l vport.l64

/* include library for the EVMDM642 board support library (BSL) */
-l evmdm642bsl.lib

SECTIONS
{    
    .INTPROCBUFF        >       ISRAM
    .EXTPROCBUFF        >       SDRAM
}