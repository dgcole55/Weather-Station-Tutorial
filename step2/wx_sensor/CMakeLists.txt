# Copyright (c) 2022, Kry10 Limited. All rights reserved.
#
# SPDX-License-Identifier: LicenseRef-Kry10

cmake_minimum_required(VERSION 3.24)

# Set the KOS environment sources
set(CMAKE_C_COMPILER $ENV{KOS_CC})
set(CMAKE_TOOLCHAIN_FILE $ENV{KOS_RUNTIME_PATH}/share/toolchain.cmake)
list(APPEND CMAKE_MODULE_PATH $ENV{KOS_BUILTINS_PATH}/share)

project(wx_sensor C ASM)
add_compile_options(-fPIC)

# Include libraries as needed. One per include statement:
include(kos_i2c_client)
# include(kosup)

set(I2C "i2c")
set(I2C_CLIENT "-L$ENV{KOS_BUILTINS_PATH}/lib -li2c_client")

file(GLOB deps src/*.c)

add_executable(wx_sensor ${deps} ${bme280_files})

target_include_directories(
    wx_sensor 
    PRIVATE
    wx_sensor/src
    $ENV{KOS_BUILTINS_PATH}/include
)
target_link_options(wx_sensor BEFORE PRIVATE -pie)
target_link_libraries(wx_sensor ${I2C_CLIENT})