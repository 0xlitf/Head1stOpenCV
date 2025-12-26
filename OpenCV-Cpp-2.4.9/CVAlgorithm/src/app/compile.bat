@echo off
cd /d "%~dp0
echo pwd: %CD%

echo Generating compile.h...

del /Q compile.h 2>nul

echo #ifndef COMPILE_INFO_H > compile.h
echo #define COMPILE_INFO_H >> compile.h
@REM echo const char* BUILD_DATE = "%DATE% %TIME%"; >> compile.h

set RAW_DATE=%DATE%
set BUILD_DATE=%RAW_DATE:~0,4%-%RAW_DATE:~5,2%-%RAW_DATE:~8,2% %TIME%
echo const char* BUILD_DATE = "%BUILD_DATE%"; >> compile.h

:: 获取 Git 分支
for /f "tokens=*" %%i in ('git rev-parse --abbrev-ref HEAD 2^>nul') do set GIT_BRANCH=%%i
if "%GIT_BRANCH%"=="" set GIT_BRANCH=unknown
echo const char* GIT_BRANCH = "%GIT_BRANCH%"; >> compile.h

:: 获取 Git 提交哈希
for /f "tokens=*" %%i in ('git log -1 --pretty^=format:%%h 2^>nul') do set GIT_COMMIT_HASH=%%i
if "%GIT_COMMIT_HASH%"=="" set GIT_COMMIT_HASH=unknown
echo const char* GIT_COMMIT_HASH = "%GIT_COMMIT_HASH%"; >> compile.h

echo #endif >> compile.h

endlocal
