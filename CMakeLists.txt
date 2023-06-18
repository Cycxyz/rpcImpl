cmake_minimum_required(VERSION 3.6)
project(RpcImpl)

set(CMAKE_CXX_STANDARD 20)

file(GLOB CPPS "${PROJECT_SOURCE_DIR}/src/*.cpp")
file(GLOB HEADS "${PROJECT_SOURCE_DIR}/include/*.h")

include_directories(
  ${PROJECT_SOURCE_DIR}/boost_1.82
  ${PROJECT_SOURCE_DIR}/json/include
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/openssl/include
)

find_library(LIB_SSL libssl_static.lib PATHS ${PROJECT_SOURCE_DIR}/openssl REQUIRED)
find_library(LIB_CRYPTO libcrypto_static.lib PATHS ${PROJECT_SOURCE_DIR}/openssl REQUIRED)

add_library(${PROJECT_NAME} ${CPPS} ${HEADS})

add_custom_target(Target ALL WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/build/$<CONFIGURATION>
COMMAND lib.exe /OUT:libSecureRPC.lib ${LIB_SSL} ${LIB_CRYPTO} crypt32.lib ${PROJECT_SOURCE_DIR}/build/$<CONFIGURATION>/${PROJECT_NAME}.lib
DEPENDS ${PROJECT_NAME})