#define _GNU_SOURCE
#define _POSIX_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "argtable3.h"

#define ARGS_MAX_NUMBER 256
#define ENV_MAX_NUMBER 256
#define SYSTEMERROR -1
#define UNLIMITED 10000000000

#define ACCEPT 0
#define TIME_OUT 1
#define RE 2
#define ME 3
#define SYS_CALL_ERR 5
#define OUT_LIMIT_OUT 6
#define EXIT_CODE_ERR 7

#define INT_PLACE_HOLDER "<n>"
#define STR_PLACE_HOLDER "<str>"

struct parameter {
    char *input_path;
    char *output_path;
    char *err_path;
    char *log_path;
    char *temp_file_path;
    long long time_limit;
    long long stack_limit;
    long long memory_limit;
    int process_number_limit;
    long long output_size_limit;
    int is_memory_limit;
    int is_root;
    int is_seccomp;

    char *cmd[ARGS_MAX_NUMBER];
    char *env[ENV_MAX_NUMBER];
};

void init(struct parameter *para) {
    para->input_path = NULL;
    para->output_path = NULL;
    para->err_path = "/home/log/err";
    para->log_path = "/home/log/log";
    para->time_limit = UNLIMITED;
    para->stack_limit = 268435456;
    para->memory_limit = UNLIMITED;
    para->process_number_limit = 100;
    para->output_size_limit = 268435456;
    para->is_memory_limit = 1;
    para->is_root = 0;
    para->is_seccomp = 1;
    memset(para->cmd, 0, sizeof para->cmd);
    memset(para->env, 0, sizeof para->env);
    para->cmd[0] = "sexec";
    para->temp_file_path = NULL;
}


void assignment(char key[], char *val, struct parameter *para) {
    if (strcmp("input_path", key) == 0) {
        para->input_path = malloc(strlen(val) * sizeof(char));
        strcpy(para->input_path, val);
    } else if (strcmp("output_path", key) == 0) {
        para->output_path = malloc(strlen(val) * sizeof(char));
        strcpy(para->output_path, val);
    } else if (strcmp("err_path", key) == 0) {
        para->err_path = malloc(strlen(val) * sizeof(char));
        strcpy(para->err_path, val);
    } else if (strcmp("log_path", key) == 0) {
        para->log_path = malloc(strlen(val) * sizeof(char));
        strcpy(para->log_path, val);
    } else if (strcmp("time_limit", key) == 0) {
        sscanf(val, "%lld", &para->time_limit);
    } else if (strcmp("stack_limit", key) == 0) {
        sscanf(val, "%lld", &para->stack_limit);
    } else if (strcmp("memory_limit", key) == 0) {
        sscanf(val, "%lld", &para->memory_limit);
    } else if (strcmp("process_number_limit", key) == 0) {
        sscanf(val, "%d", &para->process_number_limit);
    } else if (strcmp("output_size_limit", key) == 0) {
        sscanf(val, "%lld", &para->output_size_limit);
    } else if (strcmp("cmd", key) == 0) {
        char ss[ARGS_MAX_NUMBER];
        int now = 1, len = 0;
        for (int i = 0; val[i]; i++) {
            if (val[i] == ' ') {
                ss[len] = 0;
                if (len != 0) {
                    para->cmd[now] = malloc(strlen(ss) * sizeof(char));
                    strcpy(para->cmd[now], ss);
                    now++;
                }
                len = 0;
                continue;
            }
            ss[len++] = val[i];
        }
        ss[len] = 0;
        para->cmd[now] = malloc(strlen(ss) * sizeof(char));
        strcpy(para->cmd[now], ss);
    } else if (strcmp("env", key) == 0) {
        char ss[ARGS_MAX_NUMBER];
        int now = 0, len = 0;
        for (int i = 0; val[i]; i++) {
            if (val[i] == ' ') {
                ss[len] = 0;
                if (len != 0) {
                    para->env[now] = malloc(strlen(ss) * sizeof(char));
                    strcpy(para->env[now], ss);
                    now++;
                }
                len = 0;
                continue;
            }
            ss[len++] = val[i];
        }
        ss[len] = 0;
        para->env[now] = malloc(strlen(ss) * sizeof(char));
        strcpy(para->env[now], ss);
    } else if (strcmp("is_memory_limit", key) == 0) {
        sscanf(val, "%d", &para->is_memory_limit);
    } else if (strcmp("is_seccomp", key) == 0) {
        sscanf(val, "%d", &para->is_seccomp);
    } else if (strcmp("is_root", key) == 0) {
        sscanf(val, "%d", &para->is_root);
    } else {
        printf("错误：%s参数未找到\n", key);
        exit(SYSTEMERROR);
    }
    if (para->stack_limit > para->memory_limit) para->stack_limit = para->memory_limit;
}


void parse_argv_old(int argc, char *argv[], struct parameter *para) {
    for (int i = 1; i < argc; i++) {
        char *s = argv[i];
        char key[ARGS_MAX_NUMBER], value[ARGS_MAX_NUMBER];
        int len_key = 0, len_value = 0;
        int flag = 0;
        for (int k = 0; s[k]; k++) {
            if (s[k] == ':') {
                flag = 1;
                continue;
            }
            if (flag == 0) {
                if (len_key == 0 && s[k] == ' ') continue;
                key[len_key++] = s[k];
            } else {
                if (len_value == 0 && s[k] == ' ') continue;
                value[len_value++] = s[k];
            }
        }
        key[len_key] = value[len_value] = 0;
        for (int k = len_key - 1; k >= 0; k--) {
            if (key[k] == ' ') key[k] = 0;
            else break;
        }
        for (int k = len_value - 1; k >= 0; k--) {
            if (value[k] == ' ') value[k] = 0;
            else break;
        }
        assignment(key, value, para);
    }
}

struct arg_str *input_path, *output_path, *err_path, *log_path, *cmd, *env, *temp_file_path;
struct arg_int *is_memory_limit, *is_root, *is_seccomp, *process_number_limit;
struct arg_int *arg_time_limit, *stack_limit, *memory_limit, *output_size_limit;
struct arg_end *endd;

void parse_argv(int argc, char *argv[], struct parameter *para) {
    void *arg_table[] = {
            arg_time_limit = arg_intn(NULL, "time_limit", INT_PLACE_HOLDER, 0, 1, "Max Time Limit (ms)"),
            memory_limit = arg_intn(NULL, "memory_limit", INT_PLACE_HOLDER, 0, 1, "Max Memory (byte)"),
            is_memory_limit = arg_intn(NULL, "is_memory_limit", INT_PLACE_HOLDER, 0, 1,
                                       "only check memory usage, do not setrlimit (default True)"),
            stack_limit = arg_intn(NULL, "stack_limit", INT_PLACE_HOLDER, 0, 1, "Max Stack (byte, default 256M)"),
            process_number_limit = arg_intn(NULL, "process_number_limit", INT_PLACE_HOLDER, 0, 1, "Max Process Number"),
            output_size_limit = arg_intn(NULL, "output_size_limit", INT_PLACE_HOLDER, 0, 1, "Max Output Size (byte)"),

            err_path = arg_strn(NULL, "err_path", STR_PLACE_HOLDER, 0, 1, "Exe Path"),
            input_path = arg_strn(NULL, "input_path", STR_PLACE_HOLDER, 1, 1, "Input Path(*)"),
            output_path = arg_strn(NULL, "output_path", STR_PLACE_HOLDER, 1, 1, "Output Path(*)"),

            cmd = arg_strn(NULL, "cmd", STR_PLACE_HOLDER, 0, 255, "Arg(*)"),
            env = arg_strn(NULL, "env", STR_PLACE_HOLDER, 0, 255, "Env"),

            log_path = arg_strn(NULL, "log_path", STR_PLACE_HOLDER, 0, 1, "Log Path"),
            is_seccomp = arg_intn(NULL, "is_seccomp", INT_PLACE_HOLDER, 0, 1, "open Seccomp(default True)"),
            is_root = arg_intn(NULL, "is_root", INT_PLACE_HOLDER, 0, 1, "Root(default False)"),
            temp_file_path = arg_strn(NULL, "temp_file_path", STR_PLACE_HOLDER, 1, 1, "temp_file_path(*)"),

            endd = arg_end(10),
    };

    int nerrors = arg_parse(argc, argv, arg_table);
    if (nerrors > 0) {
        printf("input_path  output_path  temp_file_path均不能为空\n");
        exit(SYSTEMERROR);
    }

    if (arg_time_limit->count > 0) {
        para->time_limit = *arg_time_limit->ival;
    }

    if (memory_limit->count > 0) {
        para->memory_limit = *memory_limit->ival;
    }

    if (is_memory_limit->count > 0) {
        para->is_memory_limit = *is_memory_limit->ival == 0 ? 0 : 1;
    }

    if (stack_limit->count > 0) {
        para->stack_limit = *stack_limit->ival;
    }

    if (process_number_limit->count > 0) {
        para->process_number_limit = *process_number_limit->ival;
    }

    if (output_size_limit->count > 0) {
        para->output_size_limit = *output_size_limit->ival;
    }

    if (err_path->count > 0) {
        para->err_path = (char *) err_path->sval[0];
    }
    if (is_root->count > 0) {
        para->is_root = *is_root->ival;
    }
    if (is_seccomp->count > 0) {
        para->is_seccomp = *is_seccomp->ival;
    }

    if (input_path->count > 0) {
        para->input_path = (char *) input_path->sval[0];
    }
    if (output_path->count > 0) {
        para->output_path = (char *) output_path->sval[0];
    }

    if (temp_file_path->count > 0) {
        para->temp_file_path = (char *) temp_file_path->sval[0];
    }

    int i = 1;
    if (cmd->count > 0) {
        for (; i < cmd->count + 1; i++) {
            para->cmd[i] = (char *) cmd->sval[i - 1];
        }
    }
    para->cmd[i] = NULL;

    i = 0;
    if (env->count > 0) {
        for (; i < env->count; i++) {
            para->env[i] = (char *) env->sval[i];
        }
    }
    para->env[i] = NULL;

    if (log_path->count > 0) {
        para->log_path = (char *) log_path->sval[0];
    }
}

void parameter_out(struct parameter *para) {
    printf("\n------------------------------评测信息------------------------------\n");
    printf("input_path: %s\n", para->input_path);
    printf("output_path: %s\n", para->output_path);
    printf("err_path: %s\n", para->err_path);
    printf("log_path: %s\n", para->log_path);

    if (para->time_limit != UNLIMITED) printf("time_limit: %lld\n", para->time_limit);
    else printf("time_limit: 不限制\n");

    printf("stack_limit: %lld\n", para->stack_limit);

    if (para->memory_limit != UNLIMITED) printf("memory_limit: %lld\n", para->memory_limit);
    else printf("memory_limit: 不限制\n");

    printf("process_number_limit: %d\n", para->process_number_limit);
    printf("output_size_limit: %lld\n", para->output_size_limit);
    printf("is_memory_limit: %d\n", para->is_memory_limit);
    printf("is_seccomp: %d\n", para->is_seccomp);
    printf("is_root: %d\n", para->is_root);
    printf("temp_file_path: %s\n", para->temp_file_path);

    printf("cmd: [");
    for (int i = 0; para->cmd[i]; i++) printf("%s%s", para->cmd[i], para->cmd[i + 1] == NULL ? "" : ",");
    printf("]\n");
    printf("env: [");
    for (int i = 0; para->env[i]; i++) printf("%s%s", para->env[i], para->env[i + 1] == NULL ? "" : ",");
    printf("]\n");
    printf("--------------------------------------------------------------------\n\n");
}

FILE *err_fp;
pid_t out_fp, in_fp;
FILE *out_sta_fp;

void err(char *err_str) {
//    time_t t;
    printf("%s -- %s\n", err_str, strerror(errno));
    printf("end code : %d\n\n", SYSTEMERROR);
    fprintf(err_fp, "--------------------------------------------\n");
    unsigned long long a = 0;
    char timeStr[14];

    time_t timer;
    struct tm *tblock;
    time(&timer);
    tblock = gmtime(&timer);


    a = (tblock->tm_year+1900)*100;
    a = (a+tblock->tm_mon+1)*100;
    a = (a+tblock->tm_mday)*100;
    a = (a+tblock->tm_hour+8)*100;
    a = (a+tblock->tm_min)*100;
    a = (a+tblock->tm_sec);
    sprintf(timeStr, "%llu", a);

    fprintf(err_fp,"%d 年 %d 月 %d 日 %d 时 %d 分 %d 秒.\n",\
            tblock->tm_year+1900, tblock->tm_mon+1, tblock->tm_mday,\
            tblock->tm_hour+8, tblock->tm_min, tblock->tm_sec);


//    fprintf(err_fp, "%s\n", ctime(&t));
    fprintf(err_fp, "-- code :%s %s\n", err_str, strerror(errno));
    fclose(err_fp);
    exit(SYSTEMERROR);
}

pid_t child_pid;
int res, kill_sign;
struct itimerval new_value, old_value;

void get_res(int kill) {
    if (kill == SIGSYS) res = SYS_CALL_ERR;
    else if (kill == SIGSEGV) res = RE;
    else if (kill == SIGVTALRM) res = TIME_OUT;
    else if (kill == SIGXFSZ) res = OUT_LIMIT_OUT;
}

void out_res(int end_sta) {
    printf("程序结束状态:");
    if (end_sta == ACCEPT) printf("正常结束\n\n");
    else if (end_sta == TIME_OUT) printf("运行超时\n\n");
    else if (end_sta == RE) printf("运行错误\n\n");
    else if (end_sta == ME) printf("内存超限\n\n");
    else if (end_sta == SYS_CALL_ERR) printf("错误的系统调用\n\n");
    else if (end_sta == OUT_LIMIT_OUT) printf("输出超限\n\n");
    else if (end_sta == EXIT_CODE_ERR) printf("返回值非0\n\n");
    else printf("未知错误\n\n");
}

void time_out(int sign) {
    kill(child_pid, SIGKILL);
    printf("kill : %d\n", child_pid);
    res = TIME_OUT;
    new_value.it_value.tv_sec = 0;
    new_value.it_value.tv_usec = 0;
    new_value.it_interval.tv_sec = 0;
    new_value.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &new_value, &old_value);
}

int main(int argc, char *argv[]) {
    struct parameter para;
    init(&para);
    parse_argv(argc, argv, &para);

    int len = 0;
    for (len = 0; para.cmd[len]; len++);

    if (para.is_root) {
        for (int i = len; i >= 2; i--) {
            para.cmd[i] = malloc(strlen(para.cmd[i - 1]) * sizeof(char));
            strcpy(para.cmd[i], para.cmd[i - 1]);
        }
        para.cmd[1] = "root";
        len++;
    }
    if (!para.is_seccomp) {
        for (int i = len; i >= 2; i--) {
            para.cmd[i] = malloc(strlen(para.cmd[i - 1]) * sizeof(char));
            strcpy(para.cmd[i], para.cmd[i - 1]);
        }
        para.cmd[1] = "unseccomp";
    }
    parameter_out(&para);

    err_fp = fopen(para.err_path, "a+");
    if (err_fp == NULL) {
        printf("%s\n", strerror(errno));
        return SYSTEMERROR;
    }

//    if ( para.input_path == NULL || para.output_path == NULL ) err("未指定输入或输出路径");
    mode_t new_umask = 0;
    umask(new_umask);

    out_fp = open(para.output_path, O_WRONLY | O_CREAT | O_TRUNC, 0772);
    if (out_fp == -1) {
        err("open时出错");
    }

    in_fp = open(para.input_path, O_RDONLY);
    if (in_fp == -1) {
        err("open时出错");
    }

    out_sta_fp = fopen(para.temp_file_path, "w");
    if (out_sta_fp == NULL) {
        err("open temp_file时出错");
    }
    chmod(para.output_path, S_IWUSR | S_IWOTH);
    // 获取开始时间
    struct timeval start, end;
    gettimeofday(&start, NULL);

    child_pid = fork();
    if (child_pid < 0) {
        err("fork子进程失败");
    } else if (child_pid == 0) { //子进程
        if (dup2(out_fp, STDOUT_FILENO) == -1) {
            close(out_fp);
            err("dup2时出错");
        }
        if (dup2(out_fp, STDERR_FILENO) == -1) {
            close(out_fp);
            err("dup2时出错");
        }
        if (dup2(in_fp, STDIN_FILENO) == -1) {
            close(out_fp);
            err("dup2时出错");
        }

        struct rlimit max_stack;
        max_stack.rlim_cur = max_stack.rlim_max = (rlim_t) (para.stack_limit);
        if (setrlimit(RLIMIT_STACK, &max_stack) != 0) {
            err("set max_stack时出错");
        }

        struct rlimit max_memory;
        if (para.is_memory_limit) {
            max_memory.rlim_cur = max_memory.rlim_max = (rlim_t) (para.memory_limit) * 2;
            if (setrlimit(RLIMIT_AS, &max_memory) != 0) {
                err("set max_memory时出错");
            }
        }

        struct rlimit max_cpu_time;
        max_cpu_time.rlim_cur = max_cpu_time.rlim_max = (rlim_t) ((para.time_limit + 1000) / 1000);
        if (setrlimit(RLIMIT_CPU, &max_cpu_time) != 0) {
            err("set max_cpu_time时出错");
        }

        struct rlimit max_process_number;
        max_process_number.rlim_cur = max_process_number.rlim_max = (rlim_t) para.process_number_limit;
        if (setrlimit(RLIMIT_NPROC, &max_process_number) != 0) {
            err("set max_process_number时出错");
        }

        struct rlimit max_output_size;
        max_output_size.rlim_cur = max_output_size.rlim_max = (rlim_t) para.output_size_limit;
        if (setrlimit(RLIMIT_FSIZE, &max_output_size) != 0) {
            err("set max_output_size时出错");
        }

        execv("/usr/bin/sexec", para.cmd);
        err("sexec时出错");
    } else {//父进程
        signal(SIGALRM, time_out);

        int time_limit = para.time_limit;

        new_value.it_value.tv_sec = time_limit / 1000;
        new_value.it_value.tv_usec = (time_limit % 1000) * 1000;
        new_value.it_interval.tv_sec = time_limit / 1000;
        new_value.it_interval.tv_usec = (time_limit % 1000) * 1000;
        setitimer(ITIMER_REAL, &new_value, &old_value);

        int status;
        struct rusage resource_usage;
        pid_t t = wait4(child_pid, &status, WSTOPPED, &resource_usage);
        if (t == -1) err("wait4时出错");
        gettimeofday(&end, NULL);
        new_value.it_value.tv_sec = 0;
        new_value.it_value.tv_usec = 0;
        new_value.it_interval.tv_sec = 0;
        new_value.it_interval.tv_usec = 0;
        setitimer(ITIMER_REAL, &new_value, &old_value);

        int real_time = (int) (end.tv_sec * 1000 + end.tv_usec / 1000 - start.tv_sec * 1000 - start.tv_usec / 1000);

        printf("real_time = %d ms\n", real_time);
        printf("real_memory = %lld \n", resource_usage.ru_maxrss * 1024ll);

        int return_code = WEXITSTATUS(status);
        if (return_code != 0) res = EXIT_CODE_ERR;

        printf("return code:%d\n\n", return_code);
        if (return_code >= 256-6 && return_code <= 256-1){
            printf("系统错误 sexec 出错\n");
            fprintf(err_fp, "--------------------------------------------\n");
            unsigned long long a = 0;
            char timeStr[50];

            time_t timer;
            struct tm *tblock;
            time(&timer);
            tblock = gmtime(&timer);


            a = (tblock->tm_year+1900)*100;
            a = (a+tblock->tm_mon+1)*100;
            a = (a+tblock->tm_mday)*100;
            a = (a+tblock->tm_hour+8)*100;
            a = (a+tblock->tm_min)*100;
            a = (a+tblock->tm_sec);
            sprintf(timeStr, "%llu", a);

            fprintf(err_fp,"%d 年 %d 月 %d 日 %d 时 %d 分 %d 秒.\n",\
            tblock->tm_year+1900, tblock->tm_mon+1, tblock->tm_mday,\
            tblock->tm_hour+8, tblock->tm_min, tblock->tm_sec);
            fprintf(err_fp, "-- sexec 出错\n");
            fprintf(err_fp,"return code : %d\n",256-return_code);
            fclose(err_fp);
            exit(SYSTEMERROR);
        }

        if (!res) {
            if (WIFEXITED(status)) {
                printf("进程正常终止\n");
                printf("end status : %d\n\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                kill_sign = WTERMSIG(status);
                printf("进程异常终止\n");
                printf("end status : %d\n\n", WTERMSIG(status));
                get_res(kill_sign);
            } else if (WIFSTOPPED(status)) {
                printf("编号为%d的进程已暂停", WSTOPSIG(status));
            } else {
                res = SYSTEMERROR;
            }
        }

        long long real_memory = resource_usage.ru_maxrss * 1024ll;
        if (real_memory > para.memory_limit) res = ME;
        out_res(res);

        fprintf(out_sta_fp, "sta=%d\n", res);
        fprintf(out_sta_fp, "return=%d\n", return_code);
        fprintf(out_sta_fp, "real_time=%d\n", real_time);
        fprintf(out_sta_fp, "real_memory=%lld\n", real_memory);
        fclose(out_sta_fp);
        return res;
    }
}