cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

file(GLOB MODULE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/*/" LIST_DIRECTORIES true)
list(APPEND CMAKE_MODULE_PATH "${MODULE_DIRS}")

include(git_version_detect)

project(sparse_matrix
    VERSION ${GITVERSIONDETECT_VERSION_MAJOR}.${GITVERSIONDETECT_VERSION_MINOR}.${GITVERSIONDETECT_VERSION_PATCH}
    DESCRIPTION "course c++ developer: hw6 - sparse matrix" 
    LANGUAGES CXX
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in 
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
    @ONLY
)

add_executable(${PROJECT_NAME} src/main.cpp)
set_target_properties(${PROJECT_NAME} 
    PROPERTIES
        CXX_STANDARD ${CMAKE_CXX_STANDARD}
        CXX_STANDARD_REQUIRED ${CMAKE_CXX_STANDARD_REQUIRED}
)
# target_compile_features(
#     ${PROJECT_NAME}
#     PRIVATE
#         cxx_std_23
# )
target_compile_options(${PROJECT_NAME} 
    PRIVATE
	    -Wall -Wextra -pedantic -fpermissive
)
target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

if(BUILD_TESTING)
    find_package(GTest 1.13 REQUIRED)
    include(CTest)
    enable_testing()
    add_executable(
        hw6_gtest
        "src/sparse/matrix_gtest.cpp"
        "src/sparse/matrix23_gtest.cpp"        
    )
    set_target_properties(
        hw6_gtest
        PROPERTIES
            CXX_STANDARD ${CMAKE_CXX_STANDARD}
            CXX_STANDARD_REQUIRED ${CMAKE_CXX_STANDARD_REQUIRED}
    )
    target_include_directories(
        hw6_gtest
        PRIVATE
            ${GTEST_INCLUDE_DIRS}
            "${CMAKE_CURRENT_SOURCE_DIR}/src"            
    )
    target_link_libraries(
        hw6_gtest
        PRIVATE
            ${GTEST_BOTH_LIBRARIES}
    )
    add_test(hw6_gtest hw6_gtest)
    if (MSVC)
        target_compile_options(
            hw6_gtest
            PRIVATE  
                /W4
        )
    else()
        target_compile_options(
            hw6_gtest
            PRIVATE
                -Wall -Wextra -pedantic
        )
    endif(MSVC)  
endif(BUILD_TESTING)

install(TARGETS ${PROJECT_NAME})

if (RELEASE_VERSION)
    set(CPACK_GENERATOR "${PACKAGE}")
    set(CPACK_PACKAGE_VERSION "${RELEASE_VERSION}")
    set(CPACK_PACKAGE_CONTACT "${GIT_DESCRIBE_PROJECT_REPOSITORY_URL}")
    include(CPack)
endif()
