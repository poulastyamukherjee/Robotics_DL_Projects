cmake_minimum_required(VERSION 2.8.11)
project(myViewDemo)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-D_USE_MATH_DEFINES)
find_package(Qt5 COMPONENTS Core Gui OpenGL Widgets REQUIRED)
find_package(RL COMPONENTS SG REQUIRED)
find_package(SoQt REQUIRED)
add_executable(myViewDemo myViewDemo.cpp)
target_link_libraries(
	myViewDemo
	${Qt5Core_LIBRARIES}
	${Qt5Gui_LIBRARIES}
	${Qt5OpenGL_LIBRARIES}
	${Qt5Widgets_LIBRARIES}
	${RL_LIBRARIES}
	${SoQt_LIBRARIES}
)