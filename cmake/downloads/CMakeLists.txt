CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

OPTION(DAZZLE_DOWNLOADS_SILENT "dazzle download silent" OFF)

PROJECT(Downloads)

SET(DAZZLE_REPOSITORY ${CMAKE_CURRENT_SOURCE_DIR}/../..)
SET(DAZZLE_THIRDPARTY_DIR ${DAZZLE_REPOSITORY}/dependencies)

include(${CMAKE_CURRENT_SOURCE_DIR}/../download_template.cmake)

DOWNLOAD_FILE(stb_image https://raw.githubusercontent.com/nothings/stb/master/stb_image.h stb_image.h)

GIT_CLONE(glad https://github.com/Dav1dde/glad.git "v2.0.4")
GIT_CLONE(glfw3 https://github.com/glfw/glfw.git "3.3.8")
GIT_CLONE(imgui https://github.com/ocornut/imgui.git "v1.89.6")
GIT_CLONE(jansson https://github.com/akheron/jansson.git "v2.14")
GIT_CLONE(jpp https://github.com/irov/jpp.git)
GIT_CLONE(nativefiledialog https://github.com/mlabbe/nativefiledialog.git)
GIT_CLONE(graphics https://github.com/irov/graphics.git)
GIT_CLONE(zlib https://github.com/madler/zlib.git "v1.2.13")
