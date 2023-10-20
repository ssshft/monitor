# CMakeList.txt: Thresh 的 CMake 项目，在此处包括源代码并定义项目特定的逻辑。
cmake_minimum_required(VERSION 3.8)
project(monitor VERSION 0.1.0)


# 设置编译器 c++11 required 
set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_BUILD_TYPE "Release")
#set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -fPIC -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -fPIC -Wall")

add_definitions("-D_TURN_OFF_PLATFORM_STRING")   # U macro conflicts between cpprestsdk with concurrentqueue
add_definitions("-DLINUX")

# 设置build输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
# 设置库文件的输出目录
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(3RD_PATH /opt/version/3rd)
file(READ /inc/version.inc VERSION LIMIT 8 OFFSET 8)
string(STRIP ${VERSION} VERSION)
set(DB_VERSION_DIR /opt/version/${VERSION})
message(${DB_VERSION_DIR})

# 添加包含目录
set(BOOST_INCLUDE_DIR ${3RD_PATH}/boost/include)
set(CPPRESTSDK_INCLUDE_DIR ${3RD_PATH}/cpprest/include)
set(CPP_REDIS_INCLUDE_DIR ${3RD_PATH}/cpp_redis/includes)
set(CPP_CRYPTLITE_INCLUDE_DIR ${3RD_PATH}/cpp_cryptlite/includes)
set(TACOPIE_INCLUDE_DIR ${3RD_PATH}/cpp_redis/tacopie/includes)
set(CPP_PYTHON_INCLUDE_DIR /opt/anaconda3/pkgs/python-3.8.8-hdb3f193_5/include/python3.8)
set(ZMQ_INCLUDE_DIR ${3RD_PATH}/zeromq/include)
set(NANOLOGLIT_INCLUDE_DIR ${3RD_PATH}/NanoLogLite)
set(MYSQL_INCLUDE_DIR ${3RD_PATH}/mysql/mysql5.7.37/include)
set(SQLITE_INCLUDE_DIR ${3RD_PATH}/sqlite/include)


#add link path
set(CPPRESTSDK_LIBRARY ${3RD_PATH}/cpprest/lib64)
set(BOOST_LIBRARY ${3RD_PATH}/boost/lib)
set(CPP_REDIS_LIBRARY ${3RD_PATH}/cpp_redis/lib)
set(NANOLOGLIT_LIBRARY ${3RD_PATH}/NanoLogLite/lib)
set(CPP_ZMQ_LIBRARY ${3RD_PATH}/zeromq/lib)
set(MYSQL_LIBRARY ${3RD_PATH}/mysql/mysql5.7.37/lib)
set(SQLITE_LIBRARY ${3RD_PATH}/sqlite/lib)

#set(NANOLOGLIT_LIBRARY ${3RD_PATH}/rapidjson/lib)

# 查找当前目录下的所有源文件并将名称保存到 DIR_LIB_SRCS 变量
#aux_source_directory(. DIR_LIB_SRCS)

# include path
include_directories(
        ${3RD_PATH}
        ${SYSTEM_INC_DIR}
        ${BOOST_INCLUDE_DIR}
        ${CPPRESTSDK_INCLUDE_DIR}
        #${COMPILE_VERSION}
        ${CPP_REDIS_INCLUDE_DIR}
        ${CPP_CRYPTLITE_INCLUDE_DIR}
        ${CPP_PYTHON_INCLUDE_DIR}
        ${TACOPIE_INCLUDE_DIR}
        ${ZMQ_INCLUDE_DIR}
        ${MYSQL_INCLUDE_DIR}
        ${SQLITE_INCLUDE_DIR}
        ./
)

include_directories(${PROJECT_SOURCE_DIR}/com)
include_directories(${PROJECT_SOURCE_DIR}/../include)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/concurrentqueue)
include_directories(${PROJECT_SOURCE_DIR}/api)
include_directories(${PROJECT_SOURCE_DIR}/api/binance)
include_directories(${PROJECT_SOURCE_DIR}/api/coinbase)
include_directories(${PROJECT_SOURCE_DIR}/api/gateio)
include_directories(${PROJECT_SOURCE_DIR}/api/bybit)
include_directories(${DB_VERSION_DIR}/include)
include_directories(${DB_VERSION_DIR})

# link path
link_directories(
        ${CPPRESTSDK_LIBRARY}
        ${BOOST_LIBRARY}
        ${CPP_REDIS_LIBRARY}
        ${CPP_ZMQ_LIBRARY}
        ${NANOLOGLIT_LIBRARY}
        ${MYSQL_LIBRARY}
        ${SQLITE_LIBRARY}
        ./lib
)

link_libraries(
        pthread
        ssl
        boost_system
        boost_iostreams
        crypto
        cpprest
        z
        rt
        nanologlite
        dl
        cpp_redis
        zmq
        tacopie
        mysqlclient
        sqlite3
)

aux_source_directory(${PROJECT_SOURCE_DIR}/src MAIN_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/api/binance BINANCE_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/api/coinbase COINBASE_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/api/gateio GATEIO_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/api/bybit BYBIT_SRCS)


#add_subdirectory(restart)
add_subdirectory(test)

add_executable(risk_monitor
        ${MAIN_SRCS}
        ${BINANCE_SRCS}
        ${COINBASE_SRCS}
        ${GATEIO_SRCS}
        ${BYBIT_SRCS}
        )
