echo off
rem This file is called from top-level build6000.bat to build video examples
rem This script can be run stand-alone if you initialize the 'TIROOT', 'TCONF'
rem and TIMAKE variables.
echo on

set TIROOT=%TI_DIR%
set TCONF=%TIROOT%\bin\utilities\tconf\tconf
set TIMAKE=%TIROOT%\cc\bin\timake

%TIMAKE% switchCamera.pjt switchCamera_LOOPBACK        %1
%TIMAKE% switchCamera.pjt switchCamera_LOOPBACK_PIP    %1

%TIMAKE% ntsc.pjt NTSC_DISPLAY_ONLY %1
%TIMAKE% ntsc.pjt NTSC_LOOPBACK     %1
%TIMAKE% ntsc.pjt NTSC_LOOPBACK_PIP %1

%TIMAKE% pal.pjt PAL_DISPLAY_ONLY  %1
%TIMAKE% pal.pjt PAL_LOOPBACK      %1
%TIMAKE% pal.pjt PAL_LOOPBACK_PIP  %1

%TIMAKE% vga.pjt VGA_DISPLAY_ONLY  %1
%TIMAKE% vga.pjt VGA_LOOPBACK      %1
%TIMAKE% vga.pjt VGA_LOOPBACK_PIP  %1

%TIMAKE% hd.pjt HD_DISPLAY_ONLY    %1
%TIMAKE% hd.pjt HD_LOOPBACK        %1
%TIMAKE% hd.pjt HD_LOOPBACK_PIP    %1

%TIMAKE% grayscale.pjt GRAYSCALE_LOOPBACK %1
%TIMAKE% grayscale.pjt GRAYSCALE_LOOPBACK_PIP %1

