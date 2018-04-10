cmake_minimum_required (VERSION 2.8.3)
PROJECT(BitcoinProject)

#Add git submodule
#git submodule add https://github.com/philsquared/Catch.git

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules/")

#Set the binaries to be placed in the ./bin/ directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "lib/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "lib/")

#Get c++11
set(CMAKE_CXX_STANDARD 11)

#Tell compiler where to find required libraries
link_directories(/usr/local/lib/ /usr/lib/x86_64-linux-gnu/)

include_directories(./src/library/ ./src/executables/test /usr/local/include)

find_package(Threads)

#Get lists of the source files
FILE(GLOB SOURCEFILES ./src/library/*.cpp ./src/library/*.c)

FILE(GLOB TEST_SOURCE ./src/executables/test/*.cpp ./src/executables/test/*.c)

#Create a libraries
add_library(commonLibrary SHARED  ${SOURCEFILES})


#Tell CMake what executables to make
ADD_EXECUTABLE(test ${EXPERIMENTER_SOURCE}) 

target_link_libraries(commonLibrary ${CMAKE_THREAD_LIBS_INIT} bitcoin bitcoin-network bitcoin-client bitcoin-protocol bitcoin-explorer ) 

#link libraries to executable
target_link_libraries(test commonLibrary)
