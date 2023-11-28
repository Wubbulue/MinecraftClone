cmake_minimum_required(VERSION 3.10)
set (CMAKE_CXX_STANDARD 17)

#find_package(OpenGL REQUIRED)

 

set (ThirdParty ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty)
set (ImGuiSrc ${ThirdParty}/lib/imgui)
set (ImPlotSrc ${ThirdParty}/lib/implot)

include_directories(${ThirdParty}/include ${CMAKE_CURRENT_SOURCE_DIR}/glm)

add_library(glad ${ThirdParty}/lib/glad/glad.c)

add_library(imgui ${ImGuiSrc}/imgui.cpp ${ImGuiSrc}/imgui_impl_glfw.cpp ${ImGuiSrc}/imgui_impl_opengl3.cpp ${ImGuiSrc}/imgui_demo.cpp ${ImGuiSrc}/imgui_draw.cpp ${ImGuiSrc}/imgui_tables.cpp ${ImGuiSrc}/imgui_widgets.cpp)

add_library(implot ${ImPlotSrc}/implot.cpp ${ImPlotSrc}/implot_items.cpp)

TARGET_LINK_LIBRARIES (implot PUBLIC imgui)         

message(${ThirdParty})

PROJECT (GL_Practice)


add_executable( GL_Practice main.cpp shader.h stb_image.h camera.h camera.cpp 
minecraft.h minecraft.cpp PerlinNoise.h EBO.cpp EBO.h VBO.cpp VBO.h VAO.cpp VAO.h
line.cpp line.h geometery.h geometery.cpp Logging.h Logging.cpp Player.h Player.cpp 
font.h font.cpp Colors.h save.h save.cpp Timer.h Timer.cpp ThreadPool.h ThreadPool.cpp 
Block.cpp Block.h)



TARGET_LINK_LIBRARIES (GL_Practice PUBLIC ${ThirdParty}/lib/glfw/glfw3.lib ${ThirdParty}/lib/freetype/freetype.lib glad imgui implot)         


