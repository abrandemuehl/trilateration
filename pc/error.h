#ifndef ERROR_H
#define ERROR_H
#include <unistd.h>
#include <stdlib.h>

#define STRING(x) #x
#define STRINGIFY(x) STRING(x)

#define ERROR(fmt, args...) \
  do { \
    fprintf(stderr, __FILE__ ":" STRINGIFY(__LINE__) " Error " fmt "\n", args); \
    exit(1); \
  } while(0)



#endif
