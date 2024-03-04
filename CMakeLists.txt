cmake_minimum_required(VERSION 3.10)
set (CMAKE_CXX_STANDARD 17)

#find_package(OpenGL REQUIRED)

PROJECT (Minecraft_Clone)
 

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (ThirdParty ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty)
set (ImGuiSrc ${ThirdParty}/lib/imgui)
set (ImPlotSrc ${ThirdParty}/lib/implot)

include_directories(${ThirdParty}/include ${CMAKE_CURRENT_SOURCE_DIR}/glm)

add_library(glad ${ThirdParty}/lib/glad/glad.c)

add_library(imgui ${ImGuiSrc}/imgui.cpp ${ImGuiSrc}/imgui_impl_glfw.cpp ${ImGuiSrc}/imgui_impl_opengl3.cpp ${ImGuiSrc}/imgui_demo.cpp ${ImGuiSrc}/imgui_draw.cpp ${ImGuiSrc}/imgui_tables.cpp ${ImGuiSrc}/imgui_widgets.cpp)

add_library(implot ${ImPlotSrc}/implot.cpp ${ImPlotSrc}/implot_items.cpp)

TARGET_LINK_LIBRARIES (implot PUBLIC imgui)         

message(${ThirdParty})




add_executable( Minecraft_Clone main.cpp shader.h stb_image.h Camera.h Camera.cpp 
minecraft.h minecraft.cpp PerlinNoise.h EBO.cpp EBO.h VBO.cpp VBO.h VAO.cpp VAO.h
line.cpp line.h geometery.h geometery.cpp Player.h Player.cpp 
font.h font.cpp Colors.h save.h save.cpp Timer.h Timer.cpp ThreadPool.h ThreadPool.cpp 
Block.cpp Block.h)



if (WIN32)
    TARGET_LINK_LIBRARIES (Minecraft_Clone PUBLIC ${ThirdParty}/lib/glfw/glfw3.lib ${ThirdParty}/lib/freetype/freetype.lib glad imgui implot)         
else()
    find_package(glfw3 3.3 REQUIRED)
    find_package(freetype REQUIRED)
    target_link_libraries (Minecraft_Clone PUBLIC glfw freetype glad imgui stdc++fs implot)         
endif (WIN32)


