@echo off
setlocal enabledelayedexpansion

echo ===================================================
echo  3. Git Commit and Push
echo ===================================================

git status --short
echo.
set /p COMMIT_MSG="Input commit message: "
if "%COMMIT_MSG%"=="" (
    echo [CANCEL] Commit message was empty.
    pause
    exit /b 0
)

git add .
git commit -m "%COMMIT_MSG%"
git push origin HEAD

echo.
echo ===================================================
echo  All operations completed successfully!
echo ===================================================
pause