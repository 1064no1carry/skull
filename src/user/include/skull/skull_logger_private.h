#ifndef SKULL_LOGGER_PRIVATE_H
#define SKULL_LOGGER_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

// private:
//  internal macros
#define SKULL_TO_STR(x) #x
#define SKULL_EXTRACT_STR(x) SKULL_TO_STR(x)
#define SKULL_LOG_PREFIX __FILE__ ":" SKULL_EXTRACT_STR(__LINE__)

// private:
//  logging base functions, but user do not need to call them directly
void skull_log(const char* fmt, ...);

bool skull_log_enable_trace();
bool skull_log_enable_debug();
bool skull_log_enable_info();
bool skull_log_enable_warn();
bool skull_log_enable_error();
bool skull_log_enable_fatal();

// private:
//  get log msg and solution
const char* skull_log_info_msg(int log_id);
const char* skull_log_warn_msg(int log_id);
const char* skull_log_warn_solution(int log_id);
const char* skull_log_error_msg(int log_id);
const char* skull_log_error_solution(int log_id);
const char* skull_log_fatal_msg(int log_id);
const char* skull_log_fatal_solution(int log_id);

#ifdef __cplusplus
}
#endif

#endif

