@echo off

if ["%~1"]==[""] (
  @echo invalid arguments, please select configuration
  goto end
)

setlocal
call :setESC

set "CONFIGURATION=%1"
set "SOLUTION_DIR=%~dp0..\..\solutions\dazzle_depends_msvc16_%CONFIGURATION%"

@echo Starting build dependencies msvc %CONFIGURATION% configuration...

@mkdir %SOLUTION_DIR%

@pushd %SOLUTION_DIR%
@call CMake -G "Visual Studio 16 2019" -A Win32 "%CD%\..\..\cmake\depends_win32" -DCMAKE_BUILD_TYPE:STRING=%CONFIGURATION% -DCMAKE_CONFIGURATION_TYPES:STRING=%CONFIGURATION%
@popd

if errorlevel 1 (
    @echo %ESC%[91m*****************************************%ESC%[0m
    @echo %ESC%[91m***************  Failure  ***************%ESC%[0m
    @echo %ESC%[91m*****************************************%ESC%[0m
    
    goto end
) else (
    @echo %ESC%[92m=========================================%ESC%[0m
    @echo %ESC%[92m=============  Successful  ==============%ESC%[0m
    @echo %ESC%[92m=========================================%ESC%[0m
)

@pushd %SOLUTION_DIR%
@call CMake.exe --build . -j 4 -- /verbosity:minimal
@popd

if errorlevel 1 (
    @echo %ESC%[91m*****************************************%ESC%[0m
    @echo %ESC%[91m***************  Failure  ***************%ESC%[0m
    @echo %ESC%[91m*****************************************%ESC%[0m
    
    goto end
) else (
    @echo %ESC%[92m=========================================%ESC%[0m
    @echo %ESC%[92m=============  Successful  ==============%ESC%[0m
    @echo %ESC%[92m=========================================%ESC%[0m
)

:end

@exit /b %errorlevel%

:setESC
for /F "tokens=1,2 delims=#" %%a in ('"prompt #$H#$E# & echo on & for %%b in (1) do rem"') do (
  set ESC=%%b
  exit /B 0
)