# Regression test for https://github.com/DaveGamble/cJSON/issues/1094
# CMake config files must not hardcode the build-time install prefix, so the
# installed package stays usable after being moved to another directory.
#
# Steps:
#   1. Install the freshly built package into a staging directory.
#   2. Simulate relocation by moving the staging directory elsewhere.
#   3. Assert the generated config files contain no build-time absolute paths.
#   4. Configure, build and run a downstream consumer that uses
#      find_package(cJSON CONFIG) against the relocated prefix.

# The top-level build directory (first positional argument, see add_test in CMakeLists.txt).
set(project_binary_dir "${CMAKE_ARGV3}")

set(staging_dir  "${CMAKE_CURRENT_BINARY_DIR}/install-test/prefix")
set(relocated_dir "${CMAKE_CURRENT_BINARY_DIR}/install-test/relocated")

# 1. Install the package to a staging directory. Overriding CMAKE_INSTALL_PREFIX
#    on the generated install script relocates the install the same way
#    `cmake --install . --prefix` does, without requiring a recent CMake.
file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/install-test")
file(MAKE_DIRECTORY "${staging_dir}")
execute_process(
    COMMAND "${CMAKE_COMMAND}"
            "-DCMAKE_INSTALL_PREFIX=${staging_dir}"
            -P "${project_binary_dir}/cmake_install.cmake"
    RESULT_VARIABLE install_result
    OUTPUT_VARIABLE install_output
    ERROR_VARIABLE install_error)
if(NOT install_result EQUAL 0)
    message(FATAL_ERROR "Installing cJSON failed: ${install_error}\n${install_output}")
endif()

# 2. Move the installed package to simulate relocation.
file(RENAME "${staging_dir}" "${relocated_dir}")

# The config files live in <prefix>/lib/cmake/cJSON/.
set(cmake_config_dir "${relocated_dir}/lib/cmake/cJSON")
if(NOT EXISTS "${cmake_config_dir}")
    message(FATAL_ERROR "CMake config files not found under ${cmake_config_dir}")
endif()

# 3. Assert that no build-time absolute path leaks into the config files.
foreach(config_file
        cJSONConfig.cmake
        cjson.cmake
        cjson-noconfig.cmake
        cjson_utils.cmake
        cjson_utils-noconfig.cmake)
    if(NOT EXISTS "${cmake_config_dir}/${config_file}")
        message(WARNING "Skipping ${config_file}: not installed")
        continue()
    endif()
    file(STRINGS "${cmake_config_dir}/${config_file}" config_lines)
    foreach(line IN LISTS config_lines)
        if(line MATCHES "(^|[^$])\"${staging_dir}/")
            message(FATAL_ERROR
                "${config_file} contains the build-time install prefix:\n  ${line}")
        endif()
    endforeach()
endforeach()

# 4. Consume the relocated package from a downstream CMake project.
set(consumer_dir "${CMAKE_CURRENT_BINARY_DIR}/install-test/consumer")
file(MAKE_DIRECTORY "${consumer_dir}")
file(WRITE "${consumer_dir}/CMakeLists.txt" [=[
cmake_minimum_required(VERSION 3.5)
project(install_test_consumer C)
find_package(cJSON CONFIG REQUIRED)
add_executable(install_test_consumer main.c)
target_link_libraries(install_test_consumer cjson)
]=])
file(WRITE "${consumer_dir}/main.c" [=[
#include <stdio.h>
#include <stdlib.h>
#include "cjson/cJSON.h"
int main(void)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "hello", "world");
    char *out = cJSON_PrintUnformatted(root);
    printf("%s\n", out);
    free(out);
    cJSON_Delete(root);
    return 0;
}
]=])
set(consumer_build_dir "${consumer_dir}/build")
execute_process(
    COMMAND "${CMAKE_COMMAND}"
            "-DCMAKE_PREFIX_PATH=${relocated_dir}"
            -S "${consumer_dir}"
            -B "${consumer_build_dir}"
    RESULT_VARIABLE configure_result
    OUTPUT_VARIABLE configure_output
    ERROR_VARIABLE configure_error)
if(NOT configure_result EQUAL 0)
    message(FATAL_ERROR "Consumer configuration failed: ${configure_error}\n${configure_output}")
endif()
execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${consumer_build_dir}"
    RESULT_VARIABLE build_result
    OUTPUT_VARIABLE build_output
    ERROR_VARIABLE build_error)
if(NOT build_result EQUAL 0)
    message(FATAL_ERROR "Consumer build failed: ${build_error}\n${build_output}")
endif()
execute_process(
    COMMAND "${consumer_build_dir}/install_test_consumer"
    RESULT_VARIABLE run_result
    OUTPUT_VARIABLE run_output
    ERROR_VARIABLE run_error)
if(NOT run_result EQUAL 0)
    message(FATAL_ERROR "Consumer run failed: ${run_error}\n${run_output}")
endif()
string(STRIP "${run_output}" run_output)
if(NOT run_output STREQUAL "{\"hello\":\"world\"}")
    message(FATAL_ERROR "Consumer produced unexpected output: '${run_output}'")
endif()

message(STATUS "install test passed: package is relocatable")