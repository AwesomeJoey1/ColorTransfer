cmake_minimum_required(VERSION "3.10.2")

project("ColorTransfer")

find_package(xtl REQUIRED)
find_package(xtensor REQUIRED)
find_package(xtensor-blas REQUIRED)

include_directories(${xtensor_INCLUDE_DIRS} ${xtensor-blas_INCLUDE_DIRS})

add_definitions(-DXTENSOR_ENABLE_XSIMD)

add_executable("${PROJECT_NAME}" main.cpp Image.h)

target_link_libraries("${PROJECT_NAME}" xtensor xtensor-blas)

target_compile_options("${PROJECT_NAME}" PRIVATE -march=native)


