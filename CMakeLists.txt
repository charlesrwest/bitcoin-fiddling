cmake_minimum_required (VERSION 2.8.3)
PROJECT(BitcoinProject)

#Add git submodule
#git submodule add https://github.com/philsquared/Catch.git

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules/")

find_package(Boost REQUIRED COMPONENTS system)

#Set the binaries to be placed in the ./bin/ directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "lib/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "lib/")

#Get c++11
set(CMAKE_CXX_STANDARD 11)

#Tell compiler where to find required libraries
link_directories(/usr/local/lib/ /usr/lib/x86_64-linux-gnu/)

include_directories(./src/library/ ./src/executables/testProgram /usr/local/include ${Boost_INCLUDE_DIRS})

find_package(Threads)

#Get lists of the source files
FILE(GLOB SOURCEFILES ./src/library/*.cpp ./src/library/*.c)

FILE(GLOB TEST_PROGRAM_SOURCE ./src/executables/testProgram/*.cpp ./src/executables/testProgram/*.c)

#Create a libraries
add_library(commonLibrary SHARED  ${SOURCEFILES})


#Tell CMake what executables to make
ADD_EXECUTABLE(testProgram ${TEST_PROGRAM_SOURCE}) 

target_link_libraries(commonLibrary ${CMAKE_THREAD_LIBS_INIT} bitcoin bitcoin-network bitcoin-client bitcoin-protocol bitcoin-explorer ${Boost_LIBRARIES} ${Boost_SYSTEM_LIBRARY} ) 

#link libraries to executable
target_link_libraries(testProgram commonLibrary)
