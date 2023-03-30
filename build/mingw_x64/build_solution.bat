@echo off

if ["%~1"]==[""] (
  @echo invalid arguments, please select configuration
  goto end
)

set "CONFIGURATION=%1"
set "SOLUTION_DIR=%~dp0..\solutions\dazzle_mingw_x64_%CONFIGURATION%"

@mkdir %SOLUTION_DIR%

@pushd %SOLUTION_DIR%
CMake -G "Ninja" "%CD%\..\.." -DCMAKE_BUILD_TYPE:STRING=%CONFIGURATION% -DCMAKE_CONFIGURATION_TYPES:STRING=%CONFIGURATION% -DDAZZLE_EDITOR_BUILD:BOOL=ON -DDAZZLE_EXAMPLE_BUILD:BOOL=ON -DDAZZLE_TESTS:BOOL=OFF -DCMAKE_TOOLCHAIN_FILE="%CD%\..\..\cmake\mingw_x64_clang_toolchains.cmake"
@popd

@pushd %SOLUTION_DIR%
CMake --build .\ -j 8 --config %CONFIGURATION%
@popd

:end