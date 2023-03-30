@echo off

if ["%~1"]==[""] (
  @echo invalid arguments, please select configuration
  goto end
)

set "CONFIGURATION=%1"
set "SOLUTION_DIR=%~dp0..\..\solutions\dazzle_msvc16_%CONFIGURATION%"

@mkdir %SOLUTION_DIR%

@pushd %SOLUTION_DIR%
CMake -G "Visual Studio 16 2019" -A Win32 "%CD%\..\..\cmake\solution_win32" -DCMAKE_BUILD_TYPE:STRING=%CONFIGURATION% -DCMAKE_CONFIGURATION_TYPES:STRING=%CONFIGURATION% -DDAZZLE_EDITOR_BUILD:BOOL=ON -DDAZZLE_EXAMPLE_BUILD:BOOL=ON -DDAZZLE_TESTS:BOOL=OFF
@popd

:end

@exit /b %errorlevel%