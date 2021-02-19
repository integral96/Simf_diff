cmake_minimum_required(VERSION 3.5)

project(temp_var ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth=1800")

find_package (Boost ${BOOST_MIN_VERSION} COMPONENTS context coroutine filesystem date_time timer thread system program_options regex)

file(GLOB SOURCES
    *.hpp
    *.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES} )
target_link_libraries (${PROJECT_NAME} ${Boost_LIBRARIES})
