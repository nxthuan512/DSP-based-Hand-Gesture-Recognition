@echo off
echo Running %0
echo.
echo Please wait . . .
set BIOS_INSTALL_DIR=C:\CCStudio_v3.3\bios_5_31_02\
cd /d "C:\CCStudio_v3.1\boards\evmdm642\examples\video\driver"
"C:\CCStudio_v3.3\bios_5_31_02\\xdctools\cdb2tcf.exe" "C:\CCStudio_v3.3\C6000\bios\include" "C:\CCStudio_v3.1\boards\evmdm642\examples\video\driver\video.cdb"> "C:\CCStudio_v3.1\boards\evmdm642\examples\video\driver\cdb2tcf.log"
if not %errorlevel%==0 goto fail
echo cdb2tcf command SUCCESSFULL.
echo.
goto end
:fail
echo Conversion  FAILED. This is the command execution log:

type "C:\CCStudio_v3.1\boards\evmdm642\examples\video\driver\cdb2tcf.log"
echo.
echo cdb2tcf command FAILED.
:end
pause
