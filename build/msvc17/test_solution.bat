@echo off

if ["%~1"]==[""] (
  @echo invalid arguments, please select configuration
  goto end
)

set "CONFIGURATION=%1"
set "SOLUTION_DIR=..\solutions\dazzle_msvc17_test_%CONFIGURATION%"

@pushd ..
@mkdir %SOLUTION_DIR%
@pushd %SOLUTION_DIR%

CMake -G "Visual Studio 17 2022" -A Win32 "%CD%\..\.." -DCMAKE_BUILD_TYPE:STRING=%CONFIGURATION% -DCMAKE_CONFIGURATION_TYPES:STRING=%CONFIGURATION% -DDAZZLE_EDITOR_BUILD:BOOL=OFF -DDAZZLE_EXAMPLE_BUILD:BOOL=OFF -DDAZZLE_TESTS:BOOL=ON
CMake --build . --config %CONFIGURATION%
CTest -C %CONFIGURATION%

@popd
@popd

:end
@echo Done

@pause