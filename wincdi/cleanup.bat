@echo off
title ProjectShaper
set /p pushKey="Do you want clean up project files? (y/n)"
if "%pushkey%" == "y" goto KILL
if "%pushkey%" == "Y" goto KILL

rem NO -> EXT

goto EXT

rem Delete for Folder and object
:KILL
mkdir exe
copy release\*.exe exe\
copy lib\hidapi.lib exe\
mkdir setup
copy InstallerProject\Release\*.* setup\
rmdir /s /q wincdi/release
rmdir /s /q wincdi/debug
rmdir /s /q InstallerProject\Release
rmdir /s /q InstallerProject\Debug
rmdir /s /q .vs
rmdir /s /q x64


rmdir /s /q debug
for /r %%C in ( ipch ) do ( if exist "%%C" ( rmdir /s /q "%%C"))
for /r %%D in ( obj ) do ( if exist "%%D" ( rmdir /s /q "%%D"))
for /r %%E in ( *.suo ) do ( if exist "%%E" ( del /s /q "%%E"))
for /r %%F in ( *.user ) do ( if exist "%%F" ( del /s /q "%%F"))
for /r %%G in ( *.sdf ) do ( if exist "%%G" ( del /s /q "%%G"))
for /r %%H in ( *.bak ) do ( if exist "%%H" ( del /s /q "%%H"))
for /r %%I in ( *.vcproj ) do ( if exist "%%I" ( del /s /q "%%I"))
for /r %%J in ( _UpgradeReport_File ) do ( if exist "%%J" ( rmdir /s /q "%%J"))
for /r %%L in ( *.ncb ) do ( if exist "%%L" ( del /s /q "%%L"))
for /r %%M in ( *.opensdf ) do ( if exist "%%M" ( del /s /q "%%M"))
for /r %%N in ( Thumbs.db ) do ( if exist "%%N" ( del /s /q "%%N"))
rem for /r %%O in ( *~ ) do ( if exist "%%O" ( del /s /q "%%O"))

goto EXT


:EXT