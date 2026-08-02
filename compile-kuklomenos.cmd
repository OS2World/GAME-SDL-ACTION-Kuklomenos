@echo off
set EMXOMFLD_TYPE=WLINK
set EMXOMFLD_LINKER=wl.exe
set EMXOMFLD_PRELINK=0
make -f Makefile.os2 %1 > compile-kuklomenos.log 2>&1
if errorlevel 1 (
    echo Build FAILED. See compile-kuklomenos.log for details.
) else (
    echo Build OK.
)
