cmake_minimum_required(VERSION 2.8)

FIND_PACKAGE(OpenCV REQUIRED)

project (cv_study)
add_executable(cv_study main.cpp)


TARGET_LINK_LIBRARIES(
        cv_study
        ${OpenCV_LIBS}
        )