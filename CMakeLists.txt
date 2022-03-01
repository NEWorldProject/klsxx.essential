cmake_minimum_required(VERSION 3.11)

project(KLSXX)

set(_KLS_THS_PUB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Published)
set(_KLS_THS_MOD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Module)

file(GLOB_RECURSE SRC_PUB ${_KLS_THS_PUB_PATH}/*.*)
file(GLOB_RECURSE SRC_MOD ${_KLS_THS_MOD_PATH}/*.*)

add_library(kls.essential STATIC ${SRC_PUB} ${SRC_MOD})
target_include_directories(kls.essential PUBLIC ${_KLS_THS_PUB_PATH})
target_include_directories(kls.essential PRIVATE ${_KLS_THS_MOD_PATH})
target_include_directories(kls.essential INTERFACE ${_KLS_THS_PUB_PATH})

add_library(klsxx::essential ALIAS kls.essential)
