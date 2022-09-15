cmake_minimum_required(VERSION 2.8.12)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

add_subdirectory(src/onsjh)
add_subdirectory(src/onsjh_vitagui)

project(VitaOns)
include("${VITASDK}/share/vita.cmake" REQUIRED)

## Configuration options for this app
set(VITA_APP_NAME "Vita Ons")
set(VITA_TITLEID  "VITAONSJH")
set(VITA_VERSION  "00.31")

# use this for custom target denpends
set(VITA_PACK_ARGS 
  -s ${CMAKE_CURRENT_BINARY_DIR}/param.sfo
  -a ${CMAKE_CURRENT_SOURCE_DIR}/asset/default.ttf=default.ttf
  -a ${CMAKE_CURRENT_SOURCE_DIR}/asset/icon0.png=sce_sys/icon0.png
  -a ${CMAKE_CURRENT_SOURCE_DIR}/asset/icon1.png=sce_sys/icon1.png
  -a ${CMAKE_CURRENT_SOURCE_DIR}/asset/bg.png=sce_sys/livearea/contents/bg.png
  -a ${CMAKE_CURRENT_SOURCE_DIR}/asset/logo.png=sce_sys/livearea/contents/logo.png
  -a ${CMAKE_CURRENT_SOURCE_DIR}/asset/startup.png=sce_sys/livearea/contents/startup.png
  -a ${CMAKE_CURRENT_SOURCE_DIR}/asset/template.xml=sce_sys/livearea/contents/template.xml 
)
set(VITA_PACK_ARGS ${VITA_PACK_ARGS} 
  -a ${CMAKE_CURRENT_BINARY_DIR}/src/onsjh/onsjh.self=onsjh.bin)
# set(VITA_PACK_ARGS ${VITA_PACK_ARGS} 
#   -a ${CMAKE_CURRENT_BINARY_DIR}/src/onsjh/onsjh.suprx=module/onsjh.suprx)
# set(VITA_PACK_ARGS ${VITA_PACK_ARGS} 
#   -b ${CMAKE_CURRENT_BINARY_DIR}/src/onsjh/onsjh.self)
set(VITA_PACK_ARGS ${VITA_PACK_ARGS} 
  -b ${CMAKE_CURRENT_BINARY_DIR}/src/onsjh_vitagui/onsjh_vitagui.self)

if(${CMAKE_VERSION} VERSION_LESS "3.20.0")
  set(VITAONS_DEPENDS onsjh.self onsjh_vitagui.self)
else()
  set(VITAONS_DEPENDS onsjh.self-self onsjh_vitagui.self-self)
endif()

add_custom_target(vpk
  echo "prepare for making ${PROJECT_NAME}.vpk ..."
  COMMAND vita-mksfoex -s APP_VER=${VITA_VERSION} -s TITLE_ID=${VITA_TITLEID} ${VITA_APP_NAME} ${CMAKE_CURRENT_BINARY_DIR}/param.sfo
  COMMAND vita-pack-vpk ${VITA_PACK_ARGS} ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.vpk
  DEPENDS ${VITAONS_DEPENDS}
)