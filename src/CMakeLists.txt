
add_library(maxplus)

add_subdirectory(algebra)
add_subdirectory(base)
add_subdirectory(game)
add_subdirectory(graph)

if (BUILD_TESTS)
    add_subdirectory(testbench)
endif (BUILD_TESTS)


set(MAXPLUSLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_include_directories(maxplus PUBLIC
    ${MAXPLUSLIB_INCLUDE_DIR}
    PRIVATE
    ${MAXPLUSLIB_INCLUDE_DIR}/maxplus
)

target_compile_features(maxplus PUBLIC cxx_std_17)
set_target_properties(maxplus PROPERTIES CXX_EXTENSIONS OFF)
