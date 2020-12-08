@echo off
rem
rem Root of Visual Developer Studio Common files.
set VSCommonDir=H:\PROGRA~1\MICROS~1\Common

rem
rem Root of Visual Developer Studio installed files.
rem
set MSDevDir=H:\PROGRA~1\MICROS~1\Common\msdev98

rem
rem Root of Visual C++ installed files.
rem
set MSVCDir=H:\PROGRA~1\MICROS~1\VC98

rem
rem VcOsDir is used to help create either a Windows 95 or Windows NT specific path.
rem
set VcOsDir=WIN95
if "%OS%" == "Windows_NT" set VcOsDir=WINNT

rem
echo Setting environment for using Microsoft Visual C++ tools.
rem

if "%OS%" == "Windows_NT" set PATH=%MSDevDir%\BIN;%MSVCDir%\BIN;%VSCommonDir%\TOOLS\%VcOsDir%;%VSCommonDir%\TOOLS;%PATH%
if "%OS%" == "" set PATH="%MSDevDir%\BIN";"%MSVCDir%\BIN";"%VSCommonDir%\TOOLS\%VcOsDir%";"%VSCommonDir%\TOOLS";"%windir%\SYSTEM";"%PATH%"
set INCLUDE=h:\mssdk\include;G:\vc\Mylib;%MSVCDir%\ATL\INCLUDE;%MSVCDir%\INCLUDE;%MSVCDir%\MFC\INCLUDE;%INCLUDE%
set LIB=h:\mssdk\lib;G:\vc\Mylib;%MSVCDir%\LIB;%MSVCDir%\MFC\LIB;%LIB%

set VcOsDir=
set VSCommonDir=


nasm -f win32 -D__WIN32__ -o 6502.obj 6502.asm > out.txt
Pause