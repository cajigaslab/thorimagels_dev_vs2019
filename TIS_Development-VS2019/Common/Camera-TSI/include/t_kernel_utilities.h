#pragma once

#include "tsi_log_object.h"

Tsi_log_object* Logger();

auto t_time_utc_str(void) -> char*;

auto t_log_config(const char *var, int type, void *data) -> int;

auto t_log(const char *file, const char *func, const int line, const char *str, ...) -> void;

auto t_log_module_initialize(const void *_kernel_handle, const char *module_name_for_logger, const char *module_group_id_for_logger) -> int;

auto t_log_module_uninitialize(const char *module_name_for_logger, const char *module_group_id_for_logger) -> int;

// TODO: Implement these w/o the kernel. db 2021-12-13
#define KERNEL_LOG_TRACE(x, ...)
#define KERNEL_LOG_INFORMATION(x, ...)
#define KERNEL_LOG_ERROR(x, ...)

//#define KERNEL_LOG_TRACE(x, ...) Logger()->log (TSI_TRACE, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__)
//#define KERNEL_LOG_INFORMATION(x, ...) Logger()->log (TSI_INFORMATION, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__)
//#define KERNEL_LOG_ERROR(x, ...) Logger()->log (TSI_ERROR, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__)

// TODO: Implement these w/o the kernel. db 2021-12-13
#define LOG_TRACE(x, ...)
#define LOG_INFORMATION(x, ...)
#define LOG_ERROR(x, ...)

//#define LOG_TRACE(x, ...) kernel->log_trace (__FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__)  // NOLINT(cppcoreguidelines-macro-usage, clang-diagnostic-gnu-zero-variadic-macro-arguments)
//#define LOG_INFORMATION(x, ...) kernel->log_information (__FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__)  // NOLINT(cppcoreguidelines-macro-usage, clang-diagnostic-gnu-zero-variadic-macro-arguments)
//#define LOG_ERROR(x, ...) kernel->log_error (__FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__)  // NOLINT(cppcoreguidelines-macro-usage, clang-diagnostic-gnu-zero-variadic-macro-arguments)
