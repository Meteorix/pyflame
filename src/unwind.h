#pragma once

#include <libunwind-ptrace.h>
#include <stdarg.h>  // For va_start, etc.
#include <string>
#include <iostream>
#include <vector>
#include "./frame.h"


namespace pyflame{

class Unwinder{
  public:
  Unwinder(): pid_(-1) {}

  void InitUnwind(pid_t pid);

  void DestoryUnwind();

  void GetCStack(pid_t pid, std::vector<Frame> *stack);

  void MergeStack(std::vector<Frame> *stack, std::vector<Frame> *py_stack, std::vector<Frame> *c_stack, std::string py_evalframe);

  private:

  pid_t pid_;
  unw_addr_space_t unw_as_;
  void *unw_context_;
};


std::string string_format(const std::string fmt, ...);
}  // namespace pyflame

