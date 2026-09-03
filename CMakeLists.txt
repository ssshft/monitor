cmake_minimum_required(VERSION 3.1)
project(TB)

# 设置编译器 c++17 required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -fPIC -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -fPIC -DABI -DNDEBUG -DOPENBLAS  -O3 -Wall -flto -fuse-linker-plugin -minline-all-stringops -malign-double -ffast-math -march=native -ffunction-sections -fdata-sections -fmessage-length=0 -msse -msse2 -msse3 -funroll-loops")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)


add_definitions("-D_TURN_OFF_PLATFORM_STRING")
# add_definitions("-DUSE_BINANCE_UNIFIED")

add_definitions(-DUSE_INFO_SHM)



#include
set(INC_DIR                  ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(API_INC_DIR              ${CMAKE_CURRENT_SOURCE_DIR}/api)
set(SYSTEM_INC_DIR           ../include)
set(SHMQUEUE_INCLUDE_DIR      ${SYSTEM_INC_DIR}/shmqueue)



aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src       DIR_MAIN_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/api/binance   DIR_BINANCE_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/api/gateio    DIR_GATEIO_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/api/okx     DIR_OKX_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/api/bybit    DIR_BYBIT_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/api/coinbase   DIR_COINBASE_SRCS)



include_directories(
        ${INC_DIR}
        ${API_INC_DIR}
        ${SYSTEM_INC_DIR}
        ${SHMQUEUE_INCLUDE_DIR}
)


link_libraries(
        pthread
        boost_system 
        crypto 
        ssl 
        cpprest 
        z 
        rt  
        dl 
        cpp_redis 
        tacopie 
        tbb 
        stdc++fs 
        fmt
        mysqlclient
)

add_executable(Monitor
        ${DIR_MAIN_SRCS}
        ${DIR_BASE_SRCS}
        ${DIR_OMS_SRCS}
        ${DIR_OPERATION_SRCS}
        ${DIR_BINANCE_SRCS}
        ${DIR_GATEIO_SRCS}
        ${DIR_OKX_SRCS}
        ${DIR_BYBIT_SRCS}
        ${DIR_COINBASE_SRCS}
        # ${HUOBI_SRCS}
        # ${XT_SRCS}
        # ${KUCOIN_SRCS}
)
