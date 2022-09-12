cmake_minimum_required(VERSION 2.8.12)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(onsjh_vitagui)
include("${VITASDK}/share/vita.cmake" REQUIRED)
message("in project " ${PROJECT_NAME})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3 -Wno-unused-variable -Wno-unused-but-set-variable -Wno-format-truncation -fno-lto")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")
set(VITA_MAKE_FSELF_FLAGS "${VITA_MAKE_FSELF_FLAGS} -a 0x2808000000000000")

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/
  ${CMAKE_CURRENT_SOURCE_DIR}/../common
  $ENV{VITASDK}/include
  $ENV{VITASDK}/arm-vita-eabi/include
  $ENV{VITASDK}/arm-vita-eabi/include
  $ENV{VITASDK}/share/gcc-arm-vita-eabi/samples/common/
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(${PROJECT_NAME}
  GUI_Main.cpp
  GUI_Utils.cpp
  vitaPackage.cpp
  ./../common/filesystem.cpp
  ./../common/dictionary.c
  ./../common/iniparser.c
  ./../common/sha1.cpp
)

target_link_libraries(${PROJECT_NAME}
  # ${CMAKE_CURRENT_BINARY_DIR}/../onsjh/libonsjh_stub.a
  SceLibKernel_stub 
  SceKernelModulemgr_stub
  ScePromoterUtil_stub 
  
  vita2d 
  vita2d_ext
  SceAppMgr_stub
  SceAppUtil_stub
  SceShellSvc_stub 
  SceDisplay_stub
  SceCommonDialog_stub 
  SceTouch_stub
  SceCtrl_stub
  SceGxm_stub
  ScePgf_stub 
  ScePaf_stub_weak 
  SceHid_stub
  freetype
  png
  z
  m
  c
)

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})