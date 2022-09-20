cmake_minimum_required(VERSION 2.8.12)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(onsjh)
include("${VITASDK}/share/vita.cmake" REQUIRED)
message("in project " ${PROJECT_NAME})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3 -Wno-unused-variable -Wno-unused-but-set-variable -Wno-format-truncation -fno-lto")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -MMD -MT -MF -std=c++11 -fno-rtti -fno-exceptions -fno-optimize-sibling-calls")
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")
set(VITA_MAKE_FSELF_FLAGS "${VITA_MAKE_FSELF_FLAGS} -a 0x2808000000000001")

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/
  ${CMAKE_CURRENT_SOURCE_DIR}/../common
  $ENV{VITASDK}/include
  $ENV{VITASDK}/arm-vita-eabi/include
  $ENV{VITASDK}/arm-vita-eabi/include/luajit-2.1
  $ENV{VITASDK}/share/gcc-arm-vita-eabi/samples/common/
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  $ENV{VITASDK}/arm-vita-eabi/lib
)

add_executable(${PROJECT_NAME}
  ./../common/vita_sbrk.c # use this to solve heap alloc restriction
  AnimationInfo.cpp
  DirectReader.cpp
  DirtyRect.cpp
  FontInfo.cpp
  LUAHandler.cpp
  NsaReader.cpp
  ONScripter.cpp
  ONScripter_animation.cpp
  ONScripter_command.cpp
  ONScripter_directdraw.cpp
  ONScripter_effect.cpp
  ONScripter_effect_breakup.cpp
  ONScripter_event.cpp
  ONScripter_file.cpp
  ONScripter_file2.cpp
  ONScripter_image.cpp
  ONScripter_lut.cpp
  ONScripter_rmenu.cpp
  ONScripter_sound.cpp
  ONScripter_text.cpp
  Parallel.cpp
  SarReader.cpp
  ScriptHandler.cpp
  ScriptParser.cpp
  ScriptParser_command.cpp
  coding2utf16.cpp
  # conv_shared.cpp
  gbk2utf16.cpp
  onscripter_main.cpp
  resize_image.cpp
  sjis2utf16.cpp
  builtin_dll/layer_snow.cpp
  builtin_dll/layer_oldmovie.cpp
  builtin_dll/ONScripter_effect_trig.cpp
  builtin_dll/ONScripter_effect_cascade.cpp
)

target_link_libraries(${PROJECT_NAME}
  SDL2_mixer
  SDL2_ttf
  SDL2_image
  SDL2

  vita2d
  vita2d_ext
  ScePgf_stub
  SceHid_stub
  bz2
  m
  c
  png
  jpeg
  webp
  z
  SceNetCtl_stub 
  SceNet_stub
  ScePaf_stub 

  FLAC
  FLAC++
  mpg123
  vorbisfile
  vorbis
  ogg
  mikmod
  modplug
  SceAudioIn_stub
  SceAudio_stub
  SceAppUtil_stub
  SceAvPlayer_stub 
  SceAppMgr_stub 

  luajit-5.1
  freetype
  SceCommonDialog_stub
  SceCtrl_stub
  SceDisplay_stub
  SceGxm_stub
  ScePower_stub
  SceTouch_stub
  SceSysmodule_stub
  SceMotion_stub 
  SceShellSvc_stub 

  dl
  pthread
  SceSblSsMgr_stub
  taihen_stub
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  -DPSV # -DPSV_MODULE
  -DUSE_SDL_RENDERER -DUSE_OGG_VORBIS -DUSE_LUA
  -DUSE_SIMD -DUSE_SIMD_ARM_NEON # -DUSE_STB
  -DUSE_BUILTIN_EFFECTS -DUSE_BUILTIN_LAYER_EFFECTS
  -DUSE_PARALLEL
)

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})

# module
# vita_create_self(${PROJECT_NAME}.suprx ${PROJECT_NAME} 
#   CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/export.yml UNSAFE)
# vita_create_stubs(${PROJECT_NAME}_stub ${PROJECT_NAME} 
#   ${CMAKE_CURRENT_SOURCE_DIR}/export.yml)