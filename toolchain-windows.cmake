cmake_minimum_required(VERSION 3.20)

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Force Clang-cl and LLVM tools
set(CMAKE_C_COMPILER clang-cl)
set(CMAKE_CXX_COMPILER clang-cl)
set(CMAKE_LINKER lld-link)
set(CMAKE_AR llvm-lib)            # <-- ADDED: Archiver for static libraries
set(CMAKE_RC_COMPILER llvm-rc)    # <-- ADDED: Resource compiler for .rc files

# Point to the xwin-extracted Windows SDK and CRT directories
set(XWIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.xwin-cache")

# Compiler flags
set(CMAKE_C_FLAGS "-target x86_64-pc-windows-msvc /winsdkdir \"${XWIN_DIR}/sdk\" /vctoolsdir \"${XWIN_DIR}/crt\"" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "" FORCE)

set(CMAKE_SHARED_LINKER_FLAGS "/libpath:\"${XWIN_DIR}/crt/lib/x86_64\" /libpath:\"${XWIN_DIR}/sdk/lib/um/x86_64\" /libpath:\"${XWIN_DIR}/sdk/lib/ucrt/x86_64\"" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}" CACHE STRING "" FORCE)

# Skip compiler force checks since we are cross-compiling
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
