#include <stdbool.h>
#include "seccomp_rules.h"


int c_cpp_file_io_seccomp_rules(char * exe_path) {
    return _c_cpp_seccomp_rules(exe_path,true);
}
