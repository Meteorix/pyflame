#pragma once

#include <libunwind-ptrace.h>
#include <stdarg.h>  // For va_start, etc.
#include <string>
#include <iostream>
#include <vector>
#include "./frame.h"


namespace pyflame{

std::string string_format(const std::string fmt, ...);
void GetCStack(pid_t pid, std::vector<Frame> *stack);

}  // namespace pyflame

