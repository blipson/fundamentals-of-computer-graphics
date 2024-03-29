cmake_minimum_required (VERSION 3.0)
project (Tutorials)

find_package(OpenGL REQUIRED)


if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
    message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
    message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()

add_subdirectory (external)

cmake_policy(SET CMP0026 OLD)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/rpavlik-cmake-modules-fe2273")
include(CreateLaunchers)
include(MSVCMultipleProcessCompile)

if(INCLUDE_DISTRIB)
    add_subdirectory(distrib)
endif(INCLUDE_DISTRIB)

include_directories(
        external/AntTweakBar-1.16/include/
        external/glfw-3.1.2/include/
        external/glm-0.9.7.1/
        external/glew-1.13.0/include/
        external/assimp-3.0.1270/include/
        external/bullet-2.81-rev2613/src/
        .
)

set(ALL_LIBS
        ${OPENGL_LIBRARY}
        glfw
        GLEW_1130
)

add_definitions(
        -DTW_STATIC
        -DTW_NO_LIB_PRAGMA
        -DTW_NO_DIRECT3D
        -DGLEW_STATIC
        -D_CRT_SECURE_NO_WARNINGS
)

add_executable(opengl
        main.cpp
        common/shader.cpp
        common/shader.hpp
)
target_link_libraries(opengl
        ${ALL_LIBS}
)

set_target_properties(opengl PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/")
create_target_launcher(opengl WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/")


SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*shader$" )


if (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )
    add_custom_command(
            TARGET opengl POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/"
    )

elseif (${CMAKE_GENERATOR} MATCHES "Xcode" )

endif (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )

