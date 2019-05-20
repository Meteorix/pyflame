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


void GetCStack(pid_t pid, std::vector<Frame> *stack){
  // follow c frames
  unw_addr_space_t as = unw_create_addr_space(&_UPT_accessors, 0);

  void *context = _UPT_create(pid);  // todo: reuse context & as?
  unw_cursor_t cursor;
  int r = unw_init_remote(&cursor, as, context);
  if (r != 0){
    //printf("unw_init_remote:%d\n", r);
    std::cerr << "ERROR: cannot initialize cursor for remote unwinding\n";
    _UPT_destroy(context);
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
      stack->push_back({"0xcframe", name, line});
    }
    else{
      printf("-- no symbol name found\n");
    }

  } while (unw_step(&cursor) > 0);
  _UPT_destroy(context);
}


}  // namespace pyflame