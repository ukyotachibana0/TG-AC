cmake_minimum_required(VERSION 3.18)
project(FastSGG_CPP VERSION 0.1 LANGUAGES CXX)

# C++ Standard 11
# CMake 3.x version
set(CMAKE_CXX_STANDARD 11)
# CMake lower version
# add_compile_options(-std=c++11)


find_package(OpenMP REQUIRED)
if (OpenMP_FOUND)
    message("OpenMP Version: " OpenMP_VERSION)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
else()
    message("Missed OpenMP! x_x")
endif()


include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/temp_attr/include
)

add_executable(${PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/main.cpp
    ${CMAKE_SOURCE_DIR}/temp_attr/src/gen_attribute.cpp
    ${CMAKE_SOURCE_DIR}/src/Attribute.cpp
    ${CMAKE_SOURCE_DIR}/src/BufferWriter.cpp
    ${CMAKE_SOURCE_DIR}/src/DeltaOutDistribution.cpp
    ${CMAKE_SOURCE_DIR}/src/Distribution.cpp
    ${CMAKE_SOURCE_DIR}/src/Generation.cpp
    ${CMAKE_SOURCE_DIR}/src/OutDegreeDistribution.cpp
    ${CMAKE_SOURCE_DIR}/src/Utility.cpp
)

# link libraries
# target_link_libraries(${PROJECT_NAME} ...)
