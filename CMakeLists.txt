cmake_minimum_required(VERSION 3.10)
project (client)

set(client_VERSION_MAJOR 0)
set(client_VERSION_MINOR 1)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -Wno-unused-variable -Wno-conversion")
set(CMAKE_C_FLAGS "-Wno-unused-variable -Wno-int-conversion")

#find_library(LIBUTILS utils)
#find_library(LIBSMARTSCAN smartscan)  #smartscan utils library

# configuration variables
set (EMU_LOCAL "1")
set (DUMP_CONFIG "1")

find_package (Threads REQUIRED)
find_package (mongocxx REQUIRED)

#file(
 #   GLOB SOURCES
  #  include/CommonLibraries/libsmartscan/*.h
   # include/CommonLibraries/libutils/*.h
    #include/SmartScanBlue/*.hpp
     #src/CommonLibraries/libsmartscan/*.c
      #src/CommonLibraries/libutils/*.c
       #src/SmartScanBlue/*.cpp

#)
add_library(SmartScan  
	    src/CommonLibraries/libsmartscan/data_queue.c  
	    src/CommonLibraries/libsmartscan/msg_queue.c
	    src/CommonLibraries/libsmartscan/smartscan_utils.c
 	    src/CommonLibraries/libsmartscan/socket.c

 )
target_include_directories (SmartScan PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/CommonLibraries/libsmartscan)

add_library(Utils 
	    src/CommonLibraries/libutils/utils.c
 )	    

target_include_directories (Utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/CommonLibraries/libutils)

# configuration file
configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/config/config.hpp.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/SmartScanBlue/config.hpp"
  )

include_directories(
include
#include/CommonLibraries/libsmartscan
#include/CommonLibraries/libutils
#include/SmartScanBlue

)

set(
 project_sources

 src/SmartScanBlue/client.cpp 
 src/SmartScanBlue/listener.cpp
 src/SmartScanBlue/parser.cpp 
src/SmartScanBlue/mongoDAO.cpp


 #src/CommonLibraries/libsmartscan/data_queue.c
 #src/CommonLibraries/libsmartscan/msg_queue.c
 #src/CommonLibraries/libsmartscan/smartscan_utils.c
 #src/CommonLibraries/libsmartscan/socket.c
 #src/CommonLibraries/libutils/utils.c
 )
#target_include_directories (project_sources PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/SmartScanBlue)

add_executable(client ${project_sources})
target_link_libraries(client ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(client mongo::mongocxx_shared)
target_link_libraries(client SmartScan)
target_link_libraries(client Utils)

