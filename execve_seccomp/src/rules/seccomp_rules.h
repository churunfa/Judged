#ifndef JUDGER_SECCOMP_RULES_H
#define JUDGER_SECCOMP_RULES_H
#include <stdbool.h>
#include "../runner.h"

int _c_cpp_seccomp_rules(char * exe_path,bool allow_write_file);
int c_cpp_seccomp_rules(char * exe_path);
int general_seccomp_rules(char * exe_path);
int c_cpp_file_io_seccomp_rules(char * exe_path);

#endif //JUDGER_SECCOMP_RULES_H
