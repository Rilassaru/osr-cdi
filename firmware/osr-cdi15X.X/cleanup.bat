@echo off
rem FXÁ‚·
title ProjectShaper
rem ðŒ•ªŠò
set /p pushKey="Shape up OSR-CDI projectH(y/n)"
if "%pushkey%" == "y" goto KILL
if "%pushkey%" == "Y" goto KILL

rem NO -> EXT

goto EXT

rem Delete for Folder and object
:KILL

copy .\dist\default\production\*.hex .
rmdir /q /s .\dist\default
rmdir /q /s .\build\default
rmdir /q /s .\debug\default

goto EXT


:EXT