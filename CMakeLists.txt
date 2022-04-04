cmake_minimum_required(VERSION 3.21)

project(moonjit
    LANGUAGES C
    VERSION 2.2.0
    DESCRIPTION "Just-In-Time Compiler for the Lua Programming language"
)

set(DASC_FLAGS -D P64)

option(LUAJIT_ENABLE_LUA52COMPAT "Enable Lua 5.2 features that might break" OFF)
option(LUAJIT_DISABLE_FFI "Disable the FFI extension" OFF)
option(LUAJIT_DISABLE_JIT "Disable the JIT compiler" OFF)
option(LUAJIT_ENABLE_GC64 "Enable GC64 mode for x64" OFF)

if(LUAJIT_ENABLE_LUA52COMPAT)
    add_compile_definitions(LUAJIT_ENABLE_LUA52COMPAT)
endif()
if(LUAJIT_DISABLE_FFI)
    add_compile_definitions(LUAJIT_DISABLE_FFI)
else()
    list(APPEND DASC_FLAGS -D FFI)
endif()
if(LUAJIT_DISABLE_JIT)
    add_compile_definitions(LUAJIT_DISABLE_JIT)
else()
    list(APPEND DASC_FLAGS -D JIT)
endif()
if(LUAJIT_ENABLE_GC64)
    add_compile_definitions(LUAJIT_ENABLE_GC64)
endif()

if(WIN32)
    list(APPEND DASC_FLAGS -D WIN)
endif()

add_compile_definitions(LUAJIT_ENABLE_OPTIMISED_HASH=1)

add_executable(luajit-minilua src/host/minilua.c)
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/src/gen/buildvm_arch.h"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/src/gen"
    COMMAND luajit-minilua ../dynasm/dynasm.lua -LN ${DASC_FLAGS} -o "${CMAKE_CURRENT_BINARY_DIR}/src/gen/buildvm_arch.h" vm_x86.dasc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_executable(luajit-buildvm
    src/host/buildvm.c
    src/host/buildvm_asm.c
    src/host/buildvm_fold.c
    src/host/buildvm_lib.c
    src/host/buildvm_peobj.c
    "${CMAKE_CURRENT_BINARY_DIR}/src/gen/buildvm_arch.h"
)
target_include_directories(luajit-buildvm PRIVATE src "${CMAKE_CURRENT_BINARY_DIR}/src/gen")
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/src/lj_vm.obj"
    COMMAND luajit-buildvm -m peobj -o "${CMAKE_CURRENT_BINARY_DIR}/src/lj_vm.obj"
)
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/src/gen/lj_ffdef.h"
    COMMAND luajit-buildvm -m ffdef -o "${CMAKE_CURRENT_BINARY_DIR}/src/gen/lj_ffdef.h" lib_base.c lib_math.c lib_bit.c lib_string.c lib_table.c lib_io.c lib_os.c lib_package.c lib_debug.c lib_jit.c lib_ffi.c
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/src/gen/lj_bcdef.h"
    COMMAND luajit-buildvm -m bcdef -o "${CMAKE_CURRENT_BINARY_DIR}/src/gen/lj_bcdef.h" lib_base.c lib_math.c lib_bit.c lib_string.c lib_table.c lib_io.c lib_os.c lib_package.c lib_debug.c lib_jit.c lib_ffi.c
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/src/gen/lj_folddef.h"
    COMMAND luajit-buildvm -m folddef -o "${CMAKE_CURRENT_BINARY_DIR}/src/gen/lj_folddef.h" lj_opt_fold.c
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/src/gen/lj_recdef.h"
    COMMAND luajit-buildvm -m recdef -o "${CMAKE_CURRENT_BINARY_DIR}/src/gen/lj_recdef.h" lib_base.c lib_math.c lib_bit.c lib_string.c lib_table.c lib_io.c lib_os.c lib_package.c lib_debug.c lib_jit.c lib_ffi.c
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/src/gen/lj_libdef.h"
    COMMAND luajit-buildvm -m libdef -o "${CMAKE_CURRENT_BINARY_DIR}/src/gen/lj_libdef.h" lib_base.c lib_math.c lib_bit.c lib_string.c lib_table.c lib_io.c lib_os.c lib_package.c lib_debug.c lib_jit.c lib_ffi.c
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/src/jit/vmdef.lua"
    COMMAND luajit-buildvm -m vmdef -o "${CMAKE_CURRENT_BINARY_DIR}/src/jit/vmdef.lua" lib_base.c lib_math.c lib_bit.c lib_string.c lib_table.c lib_io.c lib_os.c lib_package.c lib_debug.c lib_jit.c lib_ffi.c
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_library(luajit SHARED
    src/lib_base.c
    src/lib_math.c
    src/lib_bit.c
    src/lib_string.c
    src/lib_table.c
    src/lib_io.c
    src/lib_os.c
    src/lib_package.c
    src/lib_debug.c
    src/lib_jit.c
    src/lib_ffi.c
    src/lib_utf8.c
    src/lj_gc.c
    src/lj_err.c
    src/lj_char.c
    src/lj_bc.c
    src/lj_obj.c
    src/lj_buf.c
    src/lj_str.c
    src/lj_str_hash.c
    src/lj_assert.c
    src/lj_init.c
    src/lj_tab.c
    src/lj_func.c
    src/lj_udata.c
    src/lj_meta.c
    src/lj_debug.c
    src/lj_state.c
    src/lj_dispatch.c
    src/lj_vmevent.c
    src/lj_vmmath.c
    src/lj_strscan.c
    src/lj_strfmt.c
    src/lj_strfmt_num.c
    src/lj_api.c
    src/lj_profile.c
    src/lj_lex.c
    src/lj_parse.c
    src/lj_bcread.c
    src/lj_bcwrite.c
    src/lj_load.c
    src/lj_ir.c
    src/lj_opt_mem.c
    src/lj_opt_fold.c
    src/lj_opt_narrow.c
    src/lj_opt_dce.c
    src/lj_opt_loop.c
    src/lj_opt_split.c
    src/lj_opt_sink.c
    src/lj_mcode.c
    src/lj_snap.c
    src/lj_record.c
    src/lj_crecord.c
    src/lj_ffrecord.c
    src/lj_asm.c
    src/lj_trace.c
    src/lj_gdbjit.c
    src/lj_ctype.c
    src/lj_cdata.c
    src/lj_cconv.c
    src/lj_ccall.c
    src/lj_ccallback.c
    src/lj_carith.c
    src/lj_clib.c
    src/lj_cparse.c
    src/lj_lib.c
    src/lj_alloc.c
    src/lj_cxxexcept.c
    src/lib_aux.c
    src/lib_init.c
    "${CMAKE_CURRENT_BINARY_DIR}/src/gen/lj_ffdef.h"
    "${CMAKE_CURRENT_BINARY_DIR}/src/gen/lj_bcdef.h"
    "${CMAKE_CURRENT_BINARY_DIR}/src/gen/lj_folddef.h"
    "${CMAKE_CURRENT_BINARY_DIR}/src/gen/lj_recdef.h"
    "${CMAKE_CURRENT_BINARY_DIR}/src/gen/lj_libdef.h"
)
target_compile_definitions(luajit PRIVATE LUA_BUILD_AS_DLL)
target_include_directories(luajit PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/src/gen" PUBLIC "src")
target_sources(luajit PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/src/lj_vm.obj")

add_executable(moonjit src/luajit.c)
target_link_libraries(moonjit PRIVATE luajit)

if(MSVC)
    target_compile_options(luajit INTERFACE /EHsc-)
endif()
