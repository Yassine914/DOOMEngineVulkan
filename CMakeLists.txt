cmake_minimum_required(VERSION "3.20.x")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER clang++)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")
set(PROJ_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(VULKAN_SDK "C:/VulkanSDK/1.3.290.0/")

# ------------------- DOOM ENGINE ----------------------
project(DOOMEngine)

# ----- SOURCE -----
file(GLOB_RECURSE SRC CMAKE_CONFIGURE_DEPENDS "${SRC_DIR}/*.cpp")

add_executable(DOOMEngine "${SRC}" "src/main.cpp")

target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC DDEBUG=true)

# ----- IN-CODE ASSET DIRS -----
# for debug builds
target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RES_PATH="${PROJ_DIR}/res/")

# for release builds (make filepaths relative to the exe)
# target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RES_PATH="./res/")

# ----- LIBRARIES -----
include_directories("${SRC_DIR}/")


# GLFW
add_subdirectory("${LIB_DIR}/GLFW")
include_directories("${LIB_DIR}/GLFW/include/")

# VK
find_package(Vulkan REQUIRED)

# glm
include_directories("${LIB_DIR}/glm")

# file(GLOB IMGUI_SRC CMAKE_CONFIGURE_DEPENDS "${LIB_DIR}/imgui/src/*.cpp")
# add_library(imgui STATIC "${IMGUI_SRC}" "src/main.cpp")
# include_directories("${LIB_DIR}/imgui/include/")

# add_library(stb_image STATIC "${LIB_DIR}/stb_image/src/stb_image.cpp")
# include_directories("${LIB_DIR}/stb_image/include/")

target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE glfw Vulkan::Vulkan)

# ----- DEFINES -----

if(WIN32)
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC _WIN32)
endif()
