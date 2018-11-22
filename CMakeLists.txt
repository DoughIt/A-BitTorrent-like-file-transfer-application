cmake_minimum_required(VERSION 3.8)
project(A-BitTorrent-like-file-transfer-application)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES Starter\ Code/starter_code/peer.c)
add_executable(peer ${SOURCE_FILES})
