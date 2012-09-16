#include "internal.h"

#if defined(OP_ENABLE_ASSERTIONS)
void op_fatal_impl(const char *_str,const char *_file,int _line){
  fprintf(stderr,"Fatal (internal) error in %s, line %i: %s\n",
   _file,_line,_str);
  abort();
}
#endif
