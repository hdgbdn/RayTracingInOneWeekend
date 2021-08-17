# CMakeList.txt : CMake project for RayTracingInOneWeekend, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("RayTracingInOneWeekend")

# Add 3rd include glad glfw glm
find_package(OpenGL REQUIRED)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/3rd/glad)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/3rd/glfw)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/3rd/glm)
# Add source to this project's executable.
add_executable (RayTracingInOneWeekend "RayTracingInOneWeekend.cpp"  "include/hittable.h" "include/camera.h" "include/rtweekend.h")
target_link_libraries(RayTracingInOneWeekend glad glfw OpenGL::GL glm::glm)
target_include_directories(RayTracingInOneWeekend PUBLIC "include")
set_target_properties(RayTracingInOneWeekend PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
            )
