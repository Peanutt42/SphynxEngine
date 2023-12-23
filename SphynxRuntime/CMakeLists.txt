file(GLOB_RECURSE SPHYNX_RUNTIME_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_executable(SphynxRuntime ${SPHYNX_RUNTIME_SOURCES})

target_include_directories(SphynxRuntime PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(SphynxRuntime PRIVATE cxx_std_20)

add_dependencies(SphynxRuntime SphynxEngine)

target_link_libraries(SphynxRuntime PRIVATE SphynxEngine ${BULLET3_LIBS})

target_precompile_headers(SphynxRuntime PUBLIC pch.hpp)

copy_content_folder(SphynxRuntime ${CMAKE_CURRENT_BINARY_DIR})

copy_file(SphynxRuntime $<TARGET_FILE:TracyClient> $<TARGET_FILE_DIR:SphynxRuntime>)