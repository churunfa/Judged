#ifndef RUNNER_H
#define RUNNER_H

#include <sys/types.h>
#include <stdio.h>

enum {
    SUCCESS = 0,
    SEXEC_ARGV_ERR = -1,
    SET_UID_FAILED = -2,
    SET_GID_FAILED = -3,
    SEXEC_RUN_FAILED = -4,
    LOAD_SECCOMP_FAILED = -6,
};

#endif //RUNNER_H
