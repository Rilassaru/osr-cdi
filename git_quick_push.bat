@echo off
cd /d "%~dp0"

echo ===================================================
echo  Git Push Tool
echo ===================================================
echo.

git add .

echo --- Changed Files ---
git status --short
echo ---------------------
echo.

set MSG=
set /p MSG=Input commit message: 

if not defined MSG (
    echo.
    echo Commit message is empty. Aborted.
    pause
    exit /b
)

echo.
git commit -m "%MSG%"
git push origin main

echo.
echo ===================================================
echo  Push completed!
echo ===================================================
pause