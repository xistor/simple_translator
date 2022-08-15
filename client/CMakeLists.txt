

set(EXE "simple_translator")


# Dear ImGui
set(IMGUI_DIR ../3rdparty/imgui)
set(3RDPARTY_DIR ../3rdparty)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends ..)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${3RDPARTY_DIR}/ImGuiFileDialog)

file(GLOB sources *.cpp)

add_executable(${EXE} ${sources} ${IMGUI_DIR}/backends/imgui_impl_sdl.cpp
                                 ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp
                                 ${IMGUI_DIR}/imgui_demo.cpp ${IMGUI_DIR}/imgui_tables.cpp
                                 ${IMGUI_DIR}/imgui_widgets.cpp
                                 ${IMGUI_DIR}/backends/imgui_impl_sdl.cpp
                                 ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
                                 ${3RDPARTY_DIR}/ImGuiFileDialog/ImGuiFileDialog.cpp)

target_include_directories(${EXE} PUBLIC ${OPENGL_INCLUDE_DIR})

# Libraries
target_link_libraries(${EXE} ${OPENGL_LIBRARIES})
target_link_libraries(${EXE} ${SDL2_LIBRARIES})
target_link_libraries(${EXE} nlohmann_json::nlohmann_json)
target_link_libraries(${EXE} pthread)
target_link_libraries(${EXE} ${CMAKE_DL_LIBS})

#install

install(TARGETS ${EXE} DESTINATION translator)
