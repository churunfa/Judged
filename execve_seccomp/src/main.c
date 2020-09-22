#define _DEFAULT_SOURCE
#define _POSIX_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <grp.h>
#include <dlfcn.h>

#include "runner.h"
#include "rules/seccomp_rules.h"

char *exe_env[] = {NULL};

//int setgroups(size_t size, const gid_t * list);

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf("错误：sexec少参数\n");
        return SEXEC_ARGV_ERR;
    }

    int isseccomp = 1, isroot = 0;
    int d = 1;
    for (int i = 1; i < argc; i++) {
        if (strcmp("root", argv[i])==0) isroot = 1, d++;
        else if (strcmp("unseccomp", argv[i])==0) isseccomp = 0, d++;
    }

    if (isseccomp) {
        if (general_seccomp_rules(argv[d]) != SUCCESS) {
            printf("错误：seccomp_rules加载失败\n");
            return LOAD_SECCOMP_FAILED;
        }
    }
    if(!isroot){
        gid_t group_list[] = {1234};
        if (setgid(1234) != 0 || setgroups(sizeof(group_list) / sizeof(gid_t), group_list) == -1) {
            printf("错误：组id设置失败，请用root权限运行程序\n");
            return SET_GID_FAILED;
        }

        if (setuid(1234) != 0) {
            printf("错误：用户id设置失败，请用root权限运行程序\n");
            return SET_UID_FAILED;
        }
    }
    execve(argv[d], argv + d, exe_env);
    printf("错误：sexec执行失败，请检查参数和权限\n");
    return SEXEC_RUN_FAILED;
}
