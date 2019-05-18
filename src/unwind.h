#pragma once

#include <string>
#include <libunwind-ptrace.h>
#include <stdarg.h>  // For va_start, etc.

namespace pyflame{

std::string string_format(const std::string fmt, ...);

}  // namespace pyflame

