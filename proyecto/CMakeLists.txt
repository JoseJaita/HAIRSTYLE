cmake_minimum_required(VERSION 2.8)

project(HelloOpenGL)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package( OpenCV REQUIRED )

set (CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)

# Add OpenGL and GLUT header files include path (or other include paths you may want to add)
# to those the compiler uses to search for include files.
# OPENGL_INCLUDE_DIR, GLUT_INCLUDE_DIR are built-in variables defined when the packages are found.
include_directories(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS})

# Add an executable named Example using the specified source file(s).
add_executable(main main.cpp InitShader.cpp sketch2D.cc)

# Link the executable to given libraries.
target_link_libraries(main ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${GLEW_LIBRARIES} ${OpenCV_LIBS})
