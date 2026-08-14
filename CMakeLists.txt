cmake_minimum_required(VERSION 3.20)
project(Sunrise CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Recursively gather all source, header, and resource files from src and vendor
file(GLOB_RECURSE SOURCES
    "Sunrise/src/*.cpp"
    "Sunrise/src/*.h"
    "Sunrise/vendor/detours/*.cpp"
    "Sunrise/vendor/detours/*.h"
    "Sunrise/vendor/imgui/*.cpp"
    "Sunrise/vendor/imgui/*.h"
    "Sunrise/vendor/imgui/backends/*.cpp"
    "Sunrise/vendor/imgui/backends/*.h"
)

file(GLOB_RECURSE RESOURCE_FILES
    "Sunrise/resources/*.rc"
    "Sunrise/resources/*.ico"
)

# Define the shared library target matching your project's TargetName (steam_api64)
add_library(steam_api64 SHARED
    ${SOURCES}
    ${RESOURCE_FILES}
)

# Include directories matching your .vcxproj ExternalIncludePath and AdditionalIncludeDirectories
target_include_directories(steam_api64 PRIVATE
    Sunrise/src
    Sunrise/vendor/detours
    Sunrise/vendor/imgui
    Sunrise/vendor/imgui/backends
    "${XWIN_DIR}/sdk/Include/10.0.26100/um"
    "${XWIN_DIR}/sdk/Include/10.0.26100/shared"
)

# Preprocessor definitions from your vcxproj
target_compile_definitions(steam_api64 PRIVATE
    WIN32
    _WINDOWS
    _USRDLL
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
    _WINSOCK_DEPRECATED_NO_WARNINGS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    IMGUI_USER_CONFIG="core/ui/imgui_user_config.h"
)

# Compiler warnings and options
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(steam_api64 PRIVATE
        /utf-8
        /W4
        #/WX # Treat warning as error
        -Wno-unused-const-variable
        -Wno-braced-scalar-init
        -Wno-missing-braces
        $<$<CONFIG:Debug>: /MTd>
        $<$<CONFIG:Release>: /MT>
    )
endif()



# Windows libraries to link against
target_link_libraries(steam_api64 PRIVATE
    kernel32
    bcrypt
    user32
    gdi32
    dwmapi
    d3dcompiler
    shell32
    ws2_32
    synchronization
)

# Output directory matching your original $(ProjectDir)..\build\$(Platform)\$(Configuration)\
set_target_properties(steam_api64 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../build/x64/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../build/x64/$<CONFIG>"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../build/x64/$<CONFIG>"
)
