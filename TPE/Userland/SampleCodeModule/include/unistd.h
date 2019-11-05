#ifndef UNISTD_H_
#define UNISTD_H_

#include "process.h"

pid_t newProcessArgs(char * name, int(* foo)(int argc, char** argv), int argc, char * argv[], t_mode mode);
pid_t newProcess(char * name, int(* foo)(int argc, char** argv), t_mode mode);
void freeProcess(pid_t pid);
int getPid();
int waitpid();
void killProcess(pid_t pid);
void setProcessMode(pid_t pid, t_mode mode);
t_state toogleProcessLock(pid_t pid);
void sleep(uint64_t ms);
void setProcessPriority(pid_t pid, t_priority priority);

#endif
