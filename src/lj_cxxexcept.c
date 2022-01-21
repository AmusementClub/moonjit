#include <string.h>

#define LUA_CORE
#include "lj_arch.h"

// I don't want to consider x86 -- who use!
#if LJ_TARGET_WINDOWS && LJ_TARGET_X64

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Copied from Wine
// Copyright (c) 1993-2022 the Wine project authors

struct type_info
{
    const void* vtable;
    char* name;
    char mangled[128];
};

struct this_ptr_offsets {
    int this_offset;
    int vbase_descr;
    int vbase_offset;
};

struct cxx_type_info {
    unsigned flags;
    unsigned type_info;
    struct this_ptr_offsets offsets;
    unsigned size;
    unsigned copy_ctor;
};

struct cxx_type_info_table {
    unsigned count;
    unsigned info[3];
};

struct cxx_exception_type {
    unsigned flags;
    unsigned destructor;
    unsigned custom_handler;
    unsigned type_info_table;
};

const char* lj_cxxexcept_what_from_seh(const EXCEPTION_RECORD* rec) {
    if (rec->ExceptionCode != 0xe06d7363) return NULL;
    if (rec->NumberParameters != 4) return NULL;
    UINT_PTR magic = rec->ExceptionInformation[0];
    if (magic < 0x19930520 || magic > 0x19930522) return NULL;
    ULONG_PTR object = rec->ExceptionInformation[1];
    struct cxx_exception_type* info = (struct cxx_exception_type*)rec->ExceptionInformation[2];
    ULONG_PTR base = rec->ExceptionInformation[3];
    struct cxx_type_info_table* ti_table = (struct cxx_type_info_table*)(base + info->type_info_table);
    for (unsigned i = 0; i < ti_table->count; ++i) {
        struct cxx_type_info* ti_item = (struct cxx_type_info*)(base + ti_table->info[i]);
        struct type_info* ti = (struct type_info*)(base + ti_item->type_info);
        if (strcmp(ti->mangled + 1, "?AVexception@std@@") == 0)
            return ((char*(*)(void*))(*(*(void***)object + 1)))((void*)object);
    }
    return NULL;
}

#endif
