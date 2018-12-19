cmake_minimum_required(VERSION 3.8)
project(bittorrect-like)
set(CMAKE_CXX_STANDARD 98)
set(SOURCE_DIR Starter\ Code/starter_code)
set(SOURCE_FILES ${SOURCE_DIR}/bt_parse.c ${SOURCE_DIR}/spiffy.c ${SOURCE_DIR}/debug.c
        ${SOURCE_DIR}/input_buffer.c ${SOURCE_DIR}/chunk.c ${SOURCE_DIR}/sha.c ${SOURCE_DIR}/handler.c ${SOURCE_DIR}/rcv_send.c
        ${SOURCE_DIR}/tracker.c ${SOURCE_DIR}/packet.c ${SOURCE_DIR}/queue.c ${SOURCE_DIR}/peer.c )
add_executable(peer ${SOURCE_FILES})
