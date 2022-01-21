#include <typeinfo>
#include <exception>

// I just use marco definition in this header. Hope alright.
#define LUA_CORE
#include "lj_arch.h"

// I don't want to consider x86 -- who use!
#if LJ_TARGET_WINDOWS && LJ_TARGET_X64

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Copied from Wine
// Copyright (c) 1993-2022 the Wine project authors

struct this_ptr_offsets {
    int this_offset;
    int vbase_descr;
    int vbase_offset;
};

struct cxx_type_info {
    unsigned flags;
    unsigned type_info;
    this_ptr_offsets offsets;
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

extern "C" const char* lj_cxxexcept_what_from_seh(const EXCEPTION_RECORD* rec) {
    if (rec->ExceptionCode != 0xe06d7363) return nullptr;
    if (rec->NumberParameters != 4) return nullptr;
    if (auto magic = rec->ExceptionInformation[0]; magic < 0x19930520 || magic > 0x19930522) return nullptr;
    auto object = reinterpret_cast<std::exception*>(rec->ExceptionInformation[1]);
    auto info = reinterpret_cast<cxx_exception_type*>(rec->ExceptionInformation[2]);
    auto base = rec->ExceptionInformation[3];
    auto ti_table = reinterpret_cast<cxx_type_info_table*>(base + info->type_info_table);
    for (unsigned i = 0; i < ti_table->count; ++i) {
        auto ti_item = reinterpret_cast<cxx_type_info*>(base + ti_table->info[i]);
        auto ti = reinterpret_cast<std::type_info*>(base + ti_item->type_info);
        if (*ti == typeid(std::exception))
            return object->what();
    }
    return nullptr;
}

#endif
