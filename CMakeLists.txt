cmake_minimum_required(VERSION 3.18)
project(BVHVisualization)
set(CMAKE_CXX_STANDARD 17)

# list(APPEND CMAKE_PREFIX_PATH "C:\\COMMON\\glfw\\build\\install\\")
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenMP REQUIRED)
# set(GLFW_BUILD_DOCS OFF CACHE BOOL "GLFW lib only")
# set(GLFW_INSTALL OFF CACHE BOOL "GLFW lib only")


# if (MSVC)
#     SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
# endif ()

set(IMGUI_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui")
set(GLAD_PREFIX  "${CMAKE_CURRENT_SOURCE_DIR}/lib/glad")

include_directories(
    "lib"
    "lib/glad/include"
    "lib/glm"
    "lib/imgui"
    "lib/imgui/backends"
    "lib/glfw/include"
)

# glm dependecy
add_subdirectory(lib/glm)

# imgui dependency
set(IMGUI_SOURCE_FILES
    ${IMGUI_PREFIX}/imgui.cpp
    # ${IMGUI_PREFIX}/imgui_demo.cpp
    ${IMGUI_PREFIX}/imgui_draw.cpp
    ${IMGUI_PREFIX}/imgui_tables.cpp
    ${IMGUI_PREFIX}/imgui_widgets.cpp

    ${IMGUI_PREFIX}/backends/imgui_impl_glfw.cpp
    ${IMGUI_PREFIX}/backends/imgui_impl_opengl3.cpp
)
add_library(imgui ${IMGUI_SOURCE_FILES})

# glad dependency
set(GLAD_HEAD "${GLAD_PREFIX}/include/glad/glad.h")
set(GLAD_SRC  "${GLAD_PREFIX}/src/glad.c")
add_library(glad ${GLAD_HEAD} ${GLAD_SRC})

# target
set(${CMAKE_PROJECT_NAME}-SRC
        main.cpp
        # lib/imgui/imconfig.h lib/imgui/imgui.cpp lib/imgui/imgui.h lib/imgui/imgui_demo.cpp lib/imgui/imgui_draw.cpp lib/imgui/imgui_internal.h lib/imgui/imgui_tables.cpp lib/imgui/imgui_widgets.cpp lib/imgui/imstb_rectpack.h lib/imgui/imstb_textedit.h lib/imgui/imstb_truetype.h lib/imgui/imgui_impl_glfw.h lib/imgui/imgui_impl_glfw.cpp lib/imgui/imgui_impl_opengl3_loader.h lib/imgui/imgui_impl_opengl3.h lib/imgui/imgui_impl_opengl3.cpp
        lib/stb/stb_image.h lib/stb/stb_image_resize2.h lib/stb/stb_image_write.h lib/OBJ_Loader.h
        renderengine/shader/AShaderProgram.cpp renderengine/shader/AShaderProgram.h renderengine/shader/ShaderProgram.cpp renderengine/shader/ShaderProgram.h renderengine/utils/IOUtils.cpp renderengine/utils/IOUtils.h renderengine/engine/RenderEngine.cpp renderengine/engine/RenderEngine.h renderengine/engine/IRenderLogic.h renderengine/engine/MouseInput.cpp renderengine/engine/MouseInput.h renderengine/camera/ACamera.cpp renderengine/camera/ACamera.h renderengine/camera/ThirdPersonCamera.cpp renderengine/camera/ThirdPersonCamera.h renderengine/engine/KeyboardInput.cpp renderengine/engine/KeyboardInput.h renderengine/utils/Transformation.cpp renderengine/utils/Transformation.h renderengine/textures/Image.cpp renderengine/textures/Image.h renderengine/textures/Texture.cpp renderengine/textures/Texture.h renderengine/shader/ComputeShaderProgram.cpp renderengine/shader/ComputeShaderProgram.h
        visualization/BVHVisualizationRenderer.h
        visualization/BVHVisualizationRenderer.cpp
        visualization/BVHVisualizationRenderLogic.h
        visualization/BVHVisualizationRenderLogic.cpp
        visualization/AABB.h
        visualization/BVH.h
        visualization/BVH.cpp
        construction/bbox.hpp construction/cluster.hpp construction/kmeans.cpp construction/primitive.h construction/vertex.h
        construction/bvh_builder.cpp
)

add_executable(${CMAKE_PROJECT_NAME} ${${CMAKE_PROJECT_NAME}-SRC})

# target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/lib/glew/include)

target_link_libraries(${CMAKE_PROJECT_NAME}
    OpenGL::GL
    glfw
    glm::glm
    imgui
    glad
    OpenMP::OpenMP_CXX
)

if (MSVC)
    if (${CMAKE_VERSION} VERSION_LESS "3.6.0")
        message("[WARNING] CMake version lower than 3.6. - Please update CMake and rerun.\n")
    else ()
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${CMAKE_PROJECT_NAME})
    endif ()
endif ()

# if (WIN32)
#     add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
#             COMMAND ${CMAKE_COMMAND} -E copy_directory
#             "${CMAKE_BINARY_DIR}/bin"
#             ${CMAKE_BINARY_DIR}) # copy glew32[d].dll to the folder with the executable
# endif ()

#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)
#target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -fsanitize=address)
#target_link_options(${CMAKE_PROJECT_NAME} PRIVATE -fsanitize=address)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)
