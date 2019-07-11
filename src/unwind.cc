#include "./unwind.h"


namespace pyflame{

std::string string_format(const std::string fmt, ...){
  int size = ((int)fmt.size()) * 2 + 50;   // Use a rubric appropriate for your code
  std::string str;
  va_list ap;
  while (1) {     // Maximum two passes on a POSIX system...
    str.resize(size);
    va_start(ap, fmt);
    int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
    va_end(ap);
    if (n > -1 && n < size) {  // Everything worked
      str.resize(n);
      return str;
    }
    if (n > -1)  // Needed size returned
      size = n + 1;   // For null char
    else
      size *= 2;      // Guess at a larger size (OS specific)
  }
  return str;
}

void Unwinder::InitUnwind(pid_t pid){
  unw_as_ = unw_create_addr_space(&_UPT_accessors, 0);
  unw_context_ = _UPT_create(pid);  // todo: reuse context & as?

  int r = unw_set_caching_policy(unw_as_, UNW_CACHE_GLOBAL);
  if (r != 0){
    std::cerr << "ERROR: cannot set caching policy\n";
  }
}

void Unwinder::DestoryUnwind(){
  _UPT_destroy(unw_context_);
}

void Unwinder::GetCStack(pid_t pid, std::vector<Frame> *stack){
  // get c frames

  unw_cursor_t cursor;
  int r = unw_init_remote(&cursor, unw_as_, unw_context_);
  if (r != 0){
    //printf("unw_init_remote:%d\n", r);
    std::cerr << "ERROR: cannot initialize cursor for remote unwinding\n";
    return;
  }

  do {
    unw_word_t offset, pc;
    char sym[4096];
    if (unw_get_reg(&cursor, UNW_REG_IP, &pc)){
      std::cerr << "ERROR: cannot read program counter\n";
      break;
    }
    // printf("0x%lx: ", pc);

    if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0){
      std::string filename = string_format("0x%lx", pc);  // actually it is the address
      std::string name = string_format("%s", sym);
      size_t line = 0;  //(size_t)offset;  // actually it is the inmemory offset
      // printf("(%s+0x%lx)\n", sym, offset);
      stack->push_back({"0xc", name, line});
    }
    else{
      printf("-- no symbol name found\n");
      stack->push_back({"0xc", "unknown frame", 0});
    }

  } while (unw_step(&cursor) > 0);  // todo: unw_step is too slow
}

void Unwinder::MergeStack(std::vector<Frame> *stack, std::vector<Frame> *py_stack, std::vector<Frame> *c_stack, const std::string py_eval_frame){
  std::vector<Frame>::iterator cur_py_stack = py_stack->begin();

  for (auto cur_c_stack = c_stack->begin(); cur_c_stack != c_stack->end(); cur_c_stack++){
    if (cur_c_stack->name() == py_eval_frame){
      stack->push_back(*cur_py_stack);
      if (cur_py_stack != (py_stack->end() - 1)){
        cur_py_stack++;
      }
    }
    stack->push_back(*cur_c_stack);
  }

}


}  // namespace pyflame