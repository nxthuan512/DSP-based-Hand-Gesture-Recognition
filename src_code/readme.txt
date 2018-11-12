#
#  Copyright 2003 by Texas Instruments Incorporated.
#  All rights reserved. Property of Texas Instruments Incorporated.
#  Restricted rights to use, duplicate or disclose this code are
#  granted through contract.
#  
#
# "@(#) DDK 1.10.00.21 06-26-03 (ddk-b10)"
TITLE
-----
VPORT example (video)

USAGE
-----
These examples test the video driver on the DM642 EVM board. The applications
use the vportcap and vportdis video mini drivers for video capture and display.

The examples cover all supported video formats by the DM642 EVM:

Capture:
- Standard-Definition TV: NTSC/PAL

Display:
- Standard-Definition TV: NTSC/PAL
- 16-bit RGB: VGA(640x480@60Hz), SVGA(800x600@60Hz), XGA(1024x768@60Hz).
- High-Definition TV: 480p@60fps, 720p@60fps, 1080i@30fps


DESCRIPTION
-----------
There are four CCS projects used to build the examples: ntsc.pjt, pal.pjt,
vga.pjt, and hd.pjt.

These projects cover examples in NTSC, PAL, VGA and HD video formats.  In
each project, there are three configurations:
- DISPLAY_ONLY: Displays a runing color bar. 
- LOOPBACK: capture and display loop-back.
- LOOPBACK_PIP: dual-capture and picture in picture display.

The examples use the videoport display minidriver to open the videoport 0 & 1 
in capture mode and the video port 2 in display mode.  It then programs the 
video decoders and encoder on the EVM642 board through I2C before starting 
capture or display operations.

TESTING
-------
To run the examples, the following equipment is required:

- For NTSC examples: 
  * A NTSC compatible camera with composite(RCA) output.
  * A NTSC compatible TV monitor with composite or S-Video inputs.

- For PAL examples: 
  * A PAL compatible camera with composite(RCA) output.
  * A PAL compatible TV monitor with composite or S-Video inputs.

- For RGB examples:
  * A NTSC compatible camera with composite(RCA) output.
  * A computer monitor that is capable of displaying VGA@60Hz, SVGA@60Hz and 
    XGA@60Hz. 

- For HD examples:
  * A NTSC compatible camera with composite(RCA) output.
  * A High-Definition monitor that supports the following HD formats: 
    480p@60fps, 720p@60fps and 1080i@30fps.

KNOWN PROBLEMS
--------------
The video driver document (SPRA918) omitted the irqId element from the VPORTCAP_Params and
the VPORTDIS_Params structures.  This element sets the EDMA interrupt ID.

