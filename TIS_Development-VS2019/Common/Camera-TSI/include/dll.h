/*
* Copyright 2017 by Thorlabs, Inc.  All rights reserved.  Unauthorized use (including,
* without limitation, distribution and copying) is strictly prohibited.  All use requires, and is
* subject to, explicit written authorization and nondisclosure agreements with Thorlabs, Inc.
*/

#ifndef DLL_H
#define DLL_H

//#ifdef _WIN32
//#include <Windows.h>
//#endif

#include "exported_funcs.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /// <summary>
    ///     This function returns something like "DEVICE" to say which type of dll it is.
    ///     It is used in Device_module_manager::Process_module to determine
    ///     whether a dll is of interest.
    /// </summary>
    /// <returns></returns>
    extern "C" DLL_EXPORT const char *tl_module_get_type();

    extern "C" DLL_EXPORT const char *tl_module_get_name();

    extern "C" DLL_EXPORT const char *tl_module_get_version();

    extern "C" DLL_EXPORT int tl_module_initialize(void* device_module_manager_param);

    extern "C" DLL_EXPORT int tl_module_uninitialize(void* device_module_manager_param);

    //DLL_EXPORT int tl_get_num_mod_attributes();

    //DLL_EXPORT const char *tl_get_mod_attribute_type(int index);

    //DLL_EXPORT const char *tl_get_mod_attribute_name(int index);

    //DLL_EXPORT const char *tl_get_mod_attribute_desc(int index);

    extern "C" DLL_EXPORT void *tl_get_mod_attribute_value(const char *name);

    //DLL_EXPORT int tl_get_mod_functions();

    extern "C" DLL_EXPORT const char *tl_get_function_name(int index);

    //DLL_EXPORT const char *tl_get_function_prototype(int index);

    //DLL_EXPORT const char *tl_get_function_description(int index);

    extern "C" DLL_EXPORT void *tl_get_function(const char *name);

    extern "C" DLL_EXPORT int tl_get_function_table(unsigned int n, void **ftable);

    extern "C" DLL_EXPORT int tl_set_function(const char *name, void *func);

#ifdef __cplusplus
}
#endif
#endif