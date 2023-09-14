set(IMGUI_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)

add_library(imgui STATIC
		imconfig.h
		imgui.h
		imgui.cpp
		imgui_draw.cpp
		imgui_internal.h
		imgui_tables.cpp
		imgui_widgets.cpp
		imstb_rectpack.h
		imstb_textedit.h
		imstb_truetype.h
		imgui_demo.cpp
)

target_compile_features(imgui PRIVATE cxx_std_17)