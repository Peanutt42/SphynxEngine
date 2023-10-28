

add_library(backward-cpp INTERFACE)
target_include_directories(backward-cpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(backward-cpp INTERFACE $<BUILD_INTERFACE:bfd>)
    target_compile_definitions(backward-cpp INTERFACE BACKWARD_HAS_BFD=1)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(backward-cpp INTERFACE -Wno-unqualified-std-cast-call)
endif ()