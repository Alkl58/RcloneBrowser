@echo off

call :setESC

setlocal enabledelayedexpansion

if "%1" == "" (
  echo Please specify x64 ^(64-bit^) or arm64 architecture in cmdline
  goto :eof
)

set BOTH=0
if not "%1" == "x64" if not "%1" == "arm64" set BOTH=1

if %BOTH% == 1  (
  echo Only x64 ^(64-bit^) or arm64 architectures are supported!
  echo Qt 6 no longer ships 32-bit Windows desktop kits.
  goto :eof
)

set ARCH=%1
rem Cross-compile ARM64 from an x64 host; build x64 natively.
if "%ARCH%" == "arm64" (
set VCVARS_ARCH=amd64_arm64
) else (
set VCVARS_ARCH=x64
)
call "c:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" %VCVARS_ARCH% || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)



if "%ARCH%" == "arm64" (
set QT=C:\Qt\6.11.1\msvc2022_arm64\
set CMAKE_ARCH=ARM64
) else (
set QT=C:\Qt\6.11.1\msvc2022_64\
set CMAKE_ARCH=x64
)
set PATH=%QT%\bin;%PATH%

set ROOT="%~dp0.."
set BUILD="%~dp0..\build\build\release"
set CMAKEGEN="Visual Studio 17 2022"

set /p VERSION=<"%ROOT%\VERSION"

where /q git.exe
if "%ERRORLEVEL%" equ "0" (
  for /f "tokens=*" %%t in ('git.exe rev-parse --short HEAD') do (
    set COMMIT=%%t
  )
  set VERSION_COMMIT=%VERSION%-!COMMIT!
)

if "%ARCH%" == "arm64" (
  set TARGET="%~dp0\..\release\rclone-browser-%VERSION_COMMIT%-windows-arm64"
  set TARGET_EXE="%~dp0\..\release\rclone-browser-%VERSION_COMMIT%-windows-arm64"
) else (
  set TARGET="%~dp0\..\release\rclone-browser-%VERSION_COMMIT%-windows-64-bit"
  set TARGET_EXE="%~dp0\..\release\rclone-browser-%VERSION_COMMIT%-windows-64-bit"
)

pushd "%ROOT%"

if not exist release mkdir release || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)

if exist "%TARGET%" rd /s /q "%TARGET%" || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)
if exist "%TARGET%.zip" del "%TARGET%.zip" || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)
if exist "%TARGET_EXE%.exe" del "%TARGET_EXE%.exe" || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)

if exist build rd /s /q build || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)
mkdir build || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)
cd build || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)

cmake -G %CMAKEGEN% -A %CMAKE_ARCH% -DCMAKE_CONFIGURATION_TYPES="Release" -DCMAKE_PREFIX_PATH=%QT% .. || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)

cmake --build . --config Release || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)
popd

mkdir "%TARGET%" 2>nul || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)

copy "%ROOT%\README.md" "%TARGET%\Readme.md" || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)
copy "%ROOT%\CHANGELOG.md" "%TARGET%\Changelog.md" || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)
copy "%ROOT%\LICENSE" "%TARGET%\License.txt" || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)
copy "%BUILD%\RcloneBrowser.exe" "%TARGET%" || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)

windeployqt.exe --no-translations --no-compiler-runtime "%TARGET%\RcloneBrowser.exe" || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)
rd /s /q "%TARGET%\imageformats" || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)

rem include all MSVCruntime dlls (VS 2022 uses the VC143 runtime)
copy "%VCToolsRedistDir%\%ARCH%\Microsoft.VC143.CRT\msvcp140.dll" "%TARGET%\" || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)
copy "%VCToolsRedistDir%\%ARCH%\Microsoft.VC143.CRT\vcruntime140*.dll" "%TARGET%\" || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)

rem include relevant OpenSSL 3.x libraries (Qt 6 links against OpenSSL 3)
rem Adjust these paths to your installed OpenSSL 3.x location.
if "%ARCH%" == "arm64" (
copy "c:\Program Files\openssl-3-arm64\libssl-3-arm64.dll" "%TARGET%\" || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)
copy "c:\Program Files\openssl-3-arm64\libcrypto-3-arm64.dll" "%TARGET%\" || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)
) else (
copy "c:\Program Files\openssl-3-win64\libssl-3-x64.dll" "%TARGET%\" || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)
copy "c:\Program Files\openssl-3-win64\libcrypto-3-x64.dll" "%TARGET%\" || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)
)

(
echo [Paths]
echo Prefix = .
echo LibraryExecutables = .
echo Plugins = .
)>"%TARGET%\qt.conf" || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)

rem https://www.7-zip.org/
rem create zip archive of all files
"c:\Program Files\7-Zip\7z.exe" a -mx=9 -r -tzip "%TARGET%.zip" "%TARGET%" || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)

rem create proper installer
rem Inno Setup installer by https://github.com/jrsoftware/issrc
rem in case user wants to install both 32bits and 64bits versions we need two AppId
rem 64bits ;AppId={{0AF9BF43-8D44-4AFF-AE60-6CECF1BF0D31}
rem 32bits ;AppId={{5644ED3A-6028-47C0-9796-29548EF7CEA3}
if "%ARCH%" == "arm64" (
"c:\Program Files (x86)\Inno Setup 6"\iscc "/dMyAppVersion=%VERSION%" "/dMyAppId={{5644ED3A-6028-47C0-9796-29548EF7CEA3}" "/dMyAppDir=rclone-browser-%VERSION_COMMIT%-windows-arm64" "/dMyAppArch=arm64" /O"../release" /F"rclone-browser-%VERSION_COMMIT%-windows-arm64" rclone-browser-win-installer.iss || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)
) else (
"c:\Program Files (x86)\Inno Setup 6"\iscc "/dMyAppVersion=%VERSION%" "/dMyAppId={{0AF9BF43-8D44-4AFF-AE60-6CECF1BF0D31}" "/dMyAppDir=rclone-browser-%VERSION_COMMIT%-windows-64-bit" "/dMyAppArch=x64" /O"../release" /F"rclone-browser-%VERSION_COMMIT%-windows-64-bit" rclone-browser-win-installer.iss || ( call :setESC & echo. & echo. & echo %ESC%[91mBuild FAILED.%ESC%[0m  & EXIT /B 1)
)

rem Build OK

if "%ARCH%" == "arm64" (
call :setESC & echo. & echo. & echo %ESC%[92mWindows ARM64 build OK.%ESC%[0m & exit /B 0
)

if "%ARCH%" == "x64" (
call :setESC & echo. & echo. & echo %ESC%[92mWindows 64-bit build OK.%ESC%[0m & exit /B 0
)

:setESC
for /F "tokens=1,2 delims=#" %%a in ('"prompt #$H#$E# & echo on & for %%b in (1) do rem"') do (
  set ESC=%%b
  exit /B 0
)
