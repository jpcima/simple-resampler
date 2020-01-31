cmake_minimum_required(VERSION "3.3")
project(resampler)

add_library(resampler INTERFACE)
target_include_directories(resampler INTERFACE "src")

add_executable(resample_file "examples/resample_file.cpp")
target_link_libraries(resample_file PRIVATE resampler)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SNDFILE "sndfile" REQUIRED)
target_link_libraries(resample_file PRIVATE "${SNDFILE_LIBRARIES}")
target_include_directories(resample_file PRIVATE "${SNDFILE_INCLUDE_DIRS}")
