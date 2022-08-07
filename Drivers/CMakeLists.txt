cmake_minimum_required(VERSION 3.1)         # 要求する最低限の CMake バージョン
project(udev_app C)                             # プロジェクト名とプログラミング言語の指定

# 必要なインクルードファイルを追加
set(INCLUDE_DIR
    ${PROJECT_SOURCE_DIR}/common
    ${PROJECT_SOURCE_DIR}/Uart)

include_directories("${INCLUDE_DIR}")

add_executable(udev_app                     # 実行ファイル名とソースファイルの指定
    DriverManager.c
    Uart/Uart.c
    common/queue.c
)

# pthreadを使いたいので、その設定
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(udev_app PUBLIC "-pthread")

# math.hを使いたいので、その設定
target_link_libraries(udev_app PRIVATE m)

# Python.hを使うための設定
#find_package(PythonLibs REQUIRED)
#include_directories(${PYTHON_INCLUDE_DIRS})
#target_link_libraries(test_app PRIVATE ${PYTHON_LIBRARIES})