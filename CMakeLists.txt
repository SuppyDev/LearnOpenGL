cmake_minimum_required(VERSION 3.30)
project(LearnOpenGL_SuppyDev)

set(CMAKE_CXX_STANDARD 23)

# Set resources output directories
set(RESOURCE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/resources)
set(RESOURCE_DEST_DIR ${CMAKE_BINARY_DIR}/resources)
file(MAKE_DIRECTORY ${RESOURCE_DEST_DIR})

find_package(OpenGL REQUIRED)

include(FetchContent)

# Fetch glfw
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
)

# Configure glfw build
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# Fetch sdl
FetchContent_Declare(
        SDL
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.2.0
)

FetchContent_MakeAvailable(SDL glfw)

add_library(glad STATIC extern/glad/src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/extern/glad/include)

# Common source files that many executables might share
set(COMMON_SOURCES
        extern/stb_image/src/stb_image.cpp
        src/shader.cpp
        src/camera.cpp
)

# Main executable
add_executable(MainApplication
        main.cpp
        ${COMMON_SOURCES}
)

add_executable(DiffuseMaps
        apps/lighting_maps/diffuse_maps.cpp
        ${COMMON_SOURCES}
)

add_executable(SpecularMaps
        apps/lighting_maps/specular_maps.cpp
        ${COMMON_SOURCES}
)
add_executable(LightingMaps_Ex4
        apps/lighting_maps/exercise_4.cpp
        ${COMMON_SOURCES}
)

set(ALL_EXECUTABLES
        MainApplication

        # Lighting Maps
        DiffuseMaps
        SpecularMaps
        LightingMaps_Ex4
)

# Function to recursively copy ALL resource files including subdirectories
function(copy_resource_directory)
    # Ensure destination directory exists
    file(MAKE_DIRECTORY ${RESOURCE_DEST_DIR})

    # Use file(GLOB_RECURSE) to find specific file formats in ALL subdirectories
    file(GLOB_RECURSE RESOURCE_FILES
            RELATIVE ${RESOURCE_SOURCE_DIR}
            "${RESOURCE_SOURCE_DIR}/*.jpg"
            "${RESOURCE_SOURCE_DIR}/*.png"
            "${RESOURCE_SOURCE_DIR}/*.vert"
            "${RESOURCE_SOURCE_DIR}/*.frag"
    )

    # Create a list of full output paths
    set(RESOURCE_OUTPUTS)

    # Copy each file, maintaining directory structure
    foreach(RESOURCE_FILE IN LISTS RESOURCE_FILES)
        set(SOURCE_PATH "${RESOURCE_SOURCE_DIR}/${RESOURCE_FILE}")
        set(DEST_PATH "${RESOURCE_DEST_DIR}/${RESOURCE_FILE}")

        # Ensure destination directory exists
        get_filename_component(DEST_DIR ${DEST_PATH} DIRECTORY)
        file(MAKE_DIRECTORY ${DEST_DIR})

        # Add custom command to copy each file
        add_custom_command(
                OUTPUT ${DEST_PATH}
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SOURCE_PATH} ${DEST_PATH}
                DEPENDS ${SOURCE_PATH}
                COMMENT "Copying ${RESOURCE_FILE}"
        )

        list(APPEND RESOURCE_OUTPUTS ${DEST_PATH})
    endforeach()

    # Create a target to ensure all files are copied
    add_custom_target(copy_resources ALL
            DEPENDS ${RESOURCE_OUTPUTS}
            COMMENT "Copying resource files"
    )

    # Add as a dependency to ALL targets
    foreach(EXEC IN LISTS ALL_EXECUTABLES)
        add_dependencies(${EXEC} copy_resources)
    endforeach()

    # Debug output with each resource on a new line
    message(STATUS "Resource files found:")
    foreach(RESOURCE_FILE IN LISTS RESOURCE_FILES)
        message(STATUS "  - ${RESOURCE_FILE}")
    endforeach()
endfunction()

copy_resource_directory()

# Common include directories
set(COMMON_INCLUDES
        ${CMAKE_SOURCE_DIR}/inc
        ${OPENGL_INCLUDE_DIR}
        ${glfw_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/extern
        ${CMAKE_SOURCE_DIR}/extern/glad/include
        ${CMAKE_SOURCE_DIR}/extern/stb_image/inc
)

# Set include directories for all executables
foreach(EXEC IN LISTS ALL_EXECUTABLES)
    target_include_directories(${EXEC} PRIVATE ${COMMON_INCLUDES})

    # Link libraries
    target_link_libraries(${EXEC}
            PUBLIC
            glfw
            glad
    )
endforeach()