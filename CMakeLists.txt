cmake_minimum_required(VERSION "3.3")
project(resampler)

add_library(resampler STATIC "src/resampler_math.cpp")
target_include_directories(resampler PUBLIC "src")

find_package(PkgConfig REQUIRED)
pkg_check_modules(SNDFILE "sndfile")
pkg_check_modules(SOXR "soxr")
pkg_check_modules(SAMPLERATE "samplerate")
pkg_check_modules(SPEEXDSP "speexdsp")

find_package(OpenMP)
if(OpenMP_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

if(NOT SNDFILE_FOUND)
  message(STATUS "sndfile not found; skipping example program")
elseif(NOT SOXR_FOUND)
  message(STATUS "soxr not found; skipping example program")
elseif(NOT SAMPLERATE_FOUND)
  message(STATUS "samplerate not found; skipping example program")
elseif(NOT SPEEXDSP_FOUND)
  message(STATUS "speexdsp not found; skipping example program")
else()
  add_executable(resample_file
    "examples/file_resamplers.cpp"
    "examples/resample_file.cpp")
  target_link_libraries(resample_file PRIVATE resampler ${SOXR_LIBRARIES} ${SAMPLERATE_LIBRARIES} ${SPEEXDSP_LIBRARIES} ${SNDFILE_LIBRARIES})
  target_include_directories(resample_file PRIVATE ${SOXR_INCLUDE_DIRS} ${SAMPLERATE_INCLUDE_DIRS} ${SPEEXDSP_INCLUDE_DIRS} ${SNDFILE_INCLUDE_DIRS})
endif()
