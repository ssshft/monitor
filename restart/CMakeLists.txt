# CMakeList.txt: CMake 项目，在此处包括源代码并定义项目特定的逻辑。

aux_source_directory(. RESTART_SRCS)

add_executable(restart_risk_monitor ${RESTART_SRCS})
