# Microsoft Developer Studio Project File - Name="nnnesterJ" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=nnnesterJ - Win32 Release
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "nnnesterJ.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "nnnesterJ.mak" CFG="nnnesterJ - Win32 Release"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "nnnesterJ - Win32 Release" ("Win32 (x86) Application" 用)
!MESSAGE "nnnesterJ - Win32 Debug" ("Win32 (x86) Application" 用)
!MESSAGE "nnnesterJ - Win32 Release Kaillera" ("Win32 (x86) Application" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nnnesterJ - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /O2 /I "src/Mikami" /I "src/nnn" /I "src" /I "src/debug" /I "src/nes" /I "src/nes/cpu" /I "src/nes/ppu" /I "src/nes/apu" /I "src/nes/libsnss" /I "src/win32" /I "src/win32/resource" /I "RESOURCE" /I "src/GB" /I "src/GB/core" /I "src/PCE" /I "src/PCE/core" /D "NDEBUG" /D "_NNNESTERJ" /D DIRECTINPUT_VERSION=0x0700 /D DIRECTDRAW_VERSION=0x0700 /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_NES_ONLY" /FR /YX /FD /Gs /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG" /d "_JAPANESE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 zlib.lib libpng.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shlwapi.lib shell32.lib winmm.lib comctl32.lib comdlg32.lib wsock32.lib ddraw.lib dsound.lib dinput.lib dxguid.lib vfw32.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "src/mikami" /I "src/nnn" /I "src" /I "src/debug" /I "src/nes" /I "src/nes/cpu" /I "src/nes/ppu" /I "src/nes/apu" /I "src/nes/libsnss" /I "src/win32" /I "src/win32/resource" /I "RESOURCE" /I "src/GB" /I "src/GB/core" /I "src/PCE" /I "src/PCE/core" /D "_DEBUG" /D "_nnnesterJ" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D DIRECTINPUT_VERSION=0x0700 /D DIRECTDRAW_VERSION=0x0700 /FR /YX /FD /I /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG" /d "_JAPANESE"
# SUBTRACT RSC /x
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libpng.lib vfw32.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shlwapi.lib shell32.lib winmm.lib comctl32.lib comdlg32.lib wsock32.lib ddraw.lib dsound.lib dinput.lib dxguid.lib /nologo /subsystem:windows /profile /debug /machine:I386
# SUBTRACT LINK32 /map /nodefaultlib

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Release Kaillera"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nnnesterJ___Win32_Release_Kaillera"
# PROP BASE Intermediate_Dir "nnnesterJ___Win32_Release_Kaillera"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "nnnesterJ___Win32_Release_Kaillera"
# PROP Intermediate_Dir "nnnesterJ___Win32_Release_Kaillera"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /O2 /I "src/Mikami" /I "src/nnn" /I "src" /I "src/debug" /I "src/nes" /I "src/nes/cpu" /I "src/nes/ppu" /I "src/nes/apu" /I "src/nes/libsnss" /I "src/win32" /I "src/win32/resource" /I "RESOURCE" /I "src/GB" /I "src/GB/core" /I "src/PCE" /I "src/PCE/core" /D "NDEBUG" /D "_NNNESTERJ" /D DIRECTINPUT_VERSION=0x0700 /D DIRECTDRAW_VERSION=0x0700 /D "WIN32" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /Gs /c
# ADD CPP /nologo /G5 /W3 /GX /O2 /I "src/Mikami" /I "src/nnn" /I "src" /I "src/debug" /I "src/nes" /I "src/nes/cpu" /I "src/nes/ppu" /I "src/nes/apu" /I "src/nes/libsnss" /I "src/win32" /I "src/win32/resource" /I "RESOURCE" /I "src/GB" /I "src/GB/core" /I "src/PCE" /I "src/PCE/core" /D "NDEBUG" /D "_NNNESTERJ" /D DIRECTINPUT_VERSION=0x0700 /D DIRECTDRAW_VERSION=0x0700 /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "NNN_KAILLERA" /FR /YX /FD /Gs /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG" /d "_JAPANESE"
# ADD RSC /l 0x411 /d "NDEBUG" /d "_JAPANESE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 libpng.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shlwapi.lib shell32.lib winmm.lib comctl32.lib comdlg32.lib wsock32.lib ddraw.lib dsound.lib dinput.lib dxguid.lib vfw32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 libpng.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shlwapi.lib shell32.lib winmm.lib comctl32.lib comdlg32.lib wsock32.lib ddraw.lib dsound.lib dinput.lib dxguid.lib vfw32.lib kailleraclient.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "nnnesterJ - Win32 Release"
# Name "nnnesterJ - Win32 Debug"
# Name "nnnesterJ - Win32 Release Kaillera"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "NES"

# PROP Default_Filter ""
# Begin Group "APU"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SRC\NES\APU\emu2413.c
# End Source File
# Begin Source File

SOURCE=.\SRC\NES\APU\fdssnd.c
# End Source File
# Begin Source File

SOURCE=.\SRC\NES\APU\nes_apu.c
# End Source File
# Begin Source File

SOURCE=.\SRC\NES\APU\nes_apu_wrapper.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\NES\APU\nes_exsound.c
# End Source File
# Begin Source File

SOURCE=.\SRC\NES\APU\nes_fds.c
# End Source File
# Begin Source File

SOURCE=.\SRC\NES\APU\nes_fme7.c
# End Source File
# Begin Source File

SOURCE=.\SRC\NES\APU\nes_mmc5.c
# End Source File
# Begin Source File

SOURCE=.\SRC\NES\APU\nes_n106.c
# End Source File
# Begin Source File

SOURCE=.\SRC\NES\APU\nes_vrc6.c
# End Source File
# Begin Source File

SOURCE=.\SRC\NES\APU\nes_vrc7.c
# End Source File
# Begin Source File

SOURCE=.\SRC\NES\APU\vrc7.c
# End Source File
# Begin Source File

SOURCE=.\SRC\NES\APU\vrcvisnd.c
# End Source File
# End Group
# Begin Group "CPU"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SRC\NES\CPU\asmcoredebug.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Nes\cpu\nes6502.c
# End Source File
# Begin Source File

SOURCE=.\src\NES\cpu\NES_6502.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\NES\CPU\Nes_6502asm.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\NES\CPU\6502.obj
# End Source File
# End Group
# Begin Group "PPU"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\Nes\ppu\NES_PPU.cpp
# End Source File
# End Group
# Begin Group "libsnss"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Src\Nes\libsnss\libsnss.c

!IF  "$(CFG)" == "nnnesterJ - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Release Kaillera"

# ADD BASE CPP /O1
# ADD CPP /O1

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\src\Nes\NES.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\NES\NES_external_device.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Nes\NES_mapper.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Nes\NES_ROM.cpp
# End Source File
# Begin Source File

SOURCE=.\src\NES\SNSS.cpp

!IF  "$(CFG)" == "nnnesterJ - Win32 Release"

# ADD CPP /O1
# SUBTRACT CPP /Z<none>

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Release Kaillera"

# ADD BASE CPP /O1
# SUBTRACT BASE CPP /Z<none>
# ADD CPP /O1
# SUBTRACT CPP /Z<none>

!ENDIF 

# End Source File
# End Group
# Begin Group "win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SRC\nnn\cfgsaveload.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\iDirectX.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\netplay.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_datach_barcode_dialog.cpp

!IF  "$(CFG)" == "nnnesterJ - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Release Kaillera"

# ADD BASE CPP /O1
# ADD CPP /O1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_default_input.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_dialogs.cpp

!IF  "$(CFG)" == "nnnesterJ - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Release Kaillera"

# ADD BASE CPP /O1
# ADD CPP /O1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_directinput_input_mgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_directsound_sound_mgr.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_directsound_sound_mgr_mono.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\GB\win32_directsound_sound_mgr_stereo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_emu.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_GUID.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_NES_pad.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_NES_screen_mgr.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_screen_mgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_shellext.cpp

!IF  "$(CFG)" == "nnnesterJ - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Release Kaillera"

# ADD BASE CPP /O1
# ADD CPP /O1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_timing.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_windowed_NES_screen_mgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\winmain.cpp
# End Source File
# End Group
# Begin Group "debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\debug\debug.cpp

!IF  "$(CFG)" == "nnnesterJ - Win32 Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Release Kaillera"

# ADD BASE CPP /O2
# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\debug\HEX.cpp
# End Source File
# Begin Source File

SOURCE=.\src\debug\mono.cpp
# End Source File
# End Group
# Begin Group "Mikami"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Src\Mikami\arc.cpp

!IF  "$(CFG)" == "nnnesterJ - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Release Kaillera"

# ADD BASE CPP /O1
# ADD CPP /O1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Src\Mikami\CSndRec.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\Mikami\mkutils.cpp

!IF  "$(CFG)" == "nnnesterJ - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Release Kaillera"

# ADD BASE CPP /O1
# ADD CPP /O1

!ENDIF 

# End Source File
# End Group
# Begin Group "nnn"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SRC\nnn\2xsai.cpp

!IF  "$(CFG)" == "nnnesterJ - Win32 Release"

# ADD CPP /FAs

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Release Kaillera"

# ADD BASE CPP /FAs
# ADD CPP /FAs

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\Cheat.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\CheckFile.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\crc32.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\ddblt.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\dikeystr.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\EditView.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\extra_window.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\launcher.cpp

!IF  "$(CFG)" == "nnnesterJ - Win32 Release"

# ADD CPP /FA

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Release Kaillera"

# ADD BASE CPP /FA
# ADD CPP /FA

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\myconfig.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\nnndialog.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\nnnfullscreenmode.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\nnnkaillera.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\nnnKailleraProc.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\nnnwindowmode.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\preview.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\recavi.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\screenshot.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\ulunzip.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\unrardll.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\unrarlib.c
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\unrarp.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\unzip.c
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\webcheck.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\win32_EXT_key.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\zip.c
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\2xsai_mmx.obj
# End Source File
# End Group
# Begin Group "GB"

# PROP Default_Filter ""
# Begin Group "GB core"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SRC\GB\core\GB_cpu.cpp

!IF  "$(CFG)" == "nnnesterJ - Win32 Release"

# ADD CPP /FAs

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Release Kaillera"

# ADD BASE CPP /FAs
# ADD CPP /FAs

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SRC\GB\core\GB_debug.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\GB\core\GB_refresh.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\GB\core\gnuboy.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\SRC\GB\win32_GB_emu.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\GB\win32_GB_pad.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\GB\win32_GB_screen_mgr.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\GB\windowmode_GB.cpp
# End Source File
# End Group
# Begin Group "PCE"

# PROP Default_Filter ""
# Begin Group "PCE core"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SRC\PCE\core\h6280.c

!IF  "$(CFG)" == "nnnesterJ - Win32 Release"

# ADD CPP /FA

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Release Kaillera"

# ADD BASE CPP /FA
# ADD CPP /FA

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SRC\PCE\core\pce.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\SRC\PCE\nnnaspi.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\PCE\win32_PCE_emu.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\PCE\win32_PCE_pad.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\PCE\win32_PCE_screen_mgr.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\PCE\windowmode_PCE.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\SRC\nnn\getclocktick.cpp
# End Source File
# Begin Source File

SOURCE=.\src\recent.cpp

!IF  "$(CFG)" == "nnnesterJ - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Debug"

# ADD CPP /I "GB/core"

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Release Kaillera"

# ADD BASE CPP /O1
# ADD CPP /O1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\settings.cpp

!IF  "$(CFG)" == "nnnesterJ - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Debug"

# ADD CPP /I "GB/core"

!ELSEIF  "$(CFG)" == "nnnesterJ - Win32 Release Kaillera"

# ADD BASE CPP /O1
# ADD CPP /O1

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "NES headers"

# PROP Default_Filter ""
# Begin Group "APU headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\NES\apu\NES_APU.h
# End Source File
# Begin Source File

SOURCE=.\Src\Nes\Apu\nes_apu_wrapper.h
# End Source File
# Begin Source File

SOURCE=.\SRC\NES\APU\nes_extsound.h
# End Source File
# End Group
# Begin Group "CPU headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\Nes\cpu\nes6502.h
# End Source File
# Begin Source File

SOURCE=.\src\Nes\cpu\NES_6502.h
# End Source File
# Begin Source File

SOURCE=.\SRC\NES\CPU\Nes_6502asm.h
# End Source File
# Begin Source File

SOURCE=.\SRC\NES\CPU\NES_CPU.h
# End Source File
# End Group
# Begin Group "PPU headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\Nes\ppu\NES_PPU.h
# End Source File
# End Group
# Begin Group "libsnss headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Src\Nes\libsnss\libsnss.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\Nes\NES.h
# End Source File
# Begin Source File

SOURCE=.\SRC\NES\NES_external_device.h
# End Source File
# Begin Source File

SOURCE=.\src\Nes\NES_mapper.h
# End Source File
# Begin Source File

SOURCE=.\src\Nes\NES_pad.h
# End Source File
# Begin Source File

SOURCE=.\src\Nes\NES_pal.h
# End Source File
# Begin Source File

SOURCE=.\src\Nes\NES_ROM.h
# End Source File
# Begin Source File

SOURCE=.\src\Nes\NES_screen_mgr.h
# End Source File
# Begin Source File

SOURCE=.\src\Nes\NES_settings.h
# End Source File
# Begin Source File

SOURCE=.\src\NES\SNSS.h
# End Source File
# End Group
# Begin Group "win32 headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SRC\WIN32\iDirectX.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\netplay.h
# End Source File
# Begin Source File

SOURCE=.\Src\Win32\OSD_ButtonSettings.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\OSD_NES_graphics_settings.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_datach_barcode_dialog.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_dialogs.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_directinput_input_mgr.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_directsound_sound_mgr.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_directsound_sound_mgr_mono.h
# End Source File
# Begin Source File

SOURCE=.\SRC\GB\win32_directsound_sound_mgr_stereo.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_emu.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_globals.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_GUID.h
# End Source File
# Begin Source File

SOURCE=.\Src\Win32\win32_NES_pad.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_NES_screen_mgr.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_screen_mgr.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_shellext.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_timing.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_windowed_NES_screen_mgr.h
# End Source File
# End Group
# Begin Group "debug headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\debug\debug.h
# End Source File
# Begin Source File

SOURCE=.\src\debug\debuglog.h
# End Source File
# Begin Source File

SOURCE=.\src\debug\HEX.h
# End Source File
# Begin Source File

SOURCE=.\src\debug\mono.h
# End Source File
# End Group
# Begin Group "Mikami headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Src\Mikami\arc.h
# End Source File
# Begin Source File

SOURCE=.\Src\Mikami\CSndRec.h
# End Source File
# Begin Source File

SOURCE=.\Src\Mikami\mkutils.h
# End Source File
# End Group
# Begin Group "nnn headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SRC\nnn\cheat.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\CheckFile.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\crc32.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\ddblt.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\EditView.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\extra_window.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\launcher.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\mmemfile.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\mreadstream.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\myconfig.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\nnndialog.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\nnnextkeycfg.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\nnnfullscreenmode.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\nnnkaillera.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\nnnKailleraProc.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\nnnwindowmode.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\preview.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\recavi.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\savecfg.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\scale2x.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\screenshot.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\ulunzip.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\unrarp.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\webcheck.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\win32_EXT_key.h
# End Source File
# Begin Source File

SOURCE=.\SRC\nnn\win32mic.h
# End Source File
# End Group
# Begin Group "GB headers"

# PROP Default_Filter ""
# Begin Group "GB core Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SRC\GB\core\GB_cpu.h
# End Source File
# Begin Source File

SOURCE=.\SRC\GB\core\GB_cpucore.h
# End Source File
# Begin Source File

SOURCE=.\SRC\GB\core\GB_cpuregs.h
# End Source File
# Begin Source File

SOURCE=.\SRC\GB\core\gnuboy.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\SRC\GB\fullscreenmode_GB.h
# End Source File
# Begin Source File

SOURCE=.\SRC\GB\GB_screen_mgr.h
# End Source File
# Begin Source File

SOURCE=.\SRC\GB\win32_GB_emu.h
# End Source File
# Begin Source File

SOURCE=.\SRC\GB\win32_GB_pad.h
# End Source File
# Begin Source File

SOURCE=.\SRC\GB\win32_GB_screen_mgr.h
# End Source File
# Begin Source File

SOURCE=.\SRC\GB\windowmode_GB.h
# End Source File
# End Group
# Begin Group "PCE headers"

# PROP Default_Filter ""
# Begin Group "PCE core headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SRC\PCE\core\h6280.h
# End Source File
# Begin Source File

SOURCE=.\SRC\PCE\core\h6280ops.h
# End Source File
# Begin Source File

SOURCE=.\SRC\PCE\core\pce.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\SRC\PCE\nnnaspi.h
# End Source File
# Begin Source File

SOURCE=.\SRC\PCE\PCE_pad.h
# End Source File
# Begin Source File

SOURCE=.\SRC\PCE\PCE_screen_mgr.h
# End Source File
# Begin Source File

SOURCE=.\SRC\PCE\win32_PCE_emu.h
# End Source File
# Begin Source File

SOURCE=.\SRC\PCE\win32_PCE_pad.h
# End Source File
# Begin Source File

SOURCE=.\SRC\PCE\win32_PCE_screen_mgr.h
# End Source File
# Begin Source File

SOURCE=.\SRC\PCE\windowmode_PCE.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\controller.h
# End Source File
# Begin Source File

SOURCE=.\SRC\emudef.h
# End Source File
# Begin Source File

SOURCE=.\src\emulator.h
# End Source File
# Begin Source File

SOURCE=.\src\null_sound_mgr.h
# End Source File
# Begin Source File

SOURCE=.\src\pixmap.h
# End Source File
# Begin Source File

SOURCE=.\src\recent.h
# End Source File
# Begin Source File

SOURCE=.\src\screen_mgr.h
# End Source File
# Begin Source File

SOURCE=.\src\settings.h
# End Source File
# Begin Source File

SOURCE=.\src\sound_mgr.h
# End Source File
# Begin Source File

SOURCE=.\src\types.h
# End Source File
# Begin Source File

SOURCE=.\SRC\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\SRC\WIN32\RESOURCE\cart.ico
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\RESOURCE\Disk.ico
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\RESOURCE\nnn.ico
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\RESOURCE\nnnapp.ico
# End Source File
# Begin Source File

SOURCE=.\RESOURCE\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\RESOURCE\Script.rc
# End Source File
# End Group
# Begin Group "Documentation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\COPYING.txt
# End Source File
# Begin Source File

SOURCE=.\docs\issues.txt
# End Source File
# Begin Source File

SOURCE=".\docs\loopy-2005.txt"
# End Source File
# Begin Source File

SOURCE=.\docs\mappers.txt
# End Source File
# Begin Source File

SOURCE=.\docs\NESSOUND.txt
# End Source File
# Begin Source File

SOURCE=.\docs\nestech.txt
# End Source File
# Begin Source File

SOURCE=.\docs\project.txt
# End Source File
# Begin Source File

SOURCE=.\docs\readme.txt
# End Source File
# Begin Source File

SOURCE=".\docs\snss-tff.txt"
# End Source File
# Begin Source File

SOURCE=.\docs\todo.txt
# End Source File
# End Group
# End Target
# End Project
