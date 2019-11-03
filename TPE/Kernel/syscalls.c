#include <stdint.h>
#include "include/keyboard-Driver.h"
#include "include/videoDriver.h"
#include "include/time.h"
#include "include/memManager.h"
#include "include/processHandler.h"
#include "include/interrupts.h"
#include "include/scheduler.h"
#include "include/ipc.h"
#include "include/semaphore.h"


typedef uint64_t(*systemCall)();

int sys_ticks(int * ticks);
int sys_ticksPerSecond(int * ticks);
int sys_clear();
int sys_read(uint64_t fd, char *buffer, uint64_t size);
int sys_write(uint64_t fd, char *buffer, uint64_t size);
int sys_draw(uint64_t x, uint64_t y, unsigned char r, unsigned char g, unsigned char b);
int * sys_time(int * time);
uint64_t sys_usedMem();
uint64_t sys_freeMem(void);
void * sys_malloc(uint64_t size, uint64_t pid);
void sys_free(void * address, uint64_t pid);
void * sys_newProcess(char * name, int(* foo)(int argc, char** argv), int ppid, int argc, char * argv[], void * returnPosition);
void sys_freeProcess(void * process);
int sys_getPid(void * process);
int sys_readPipe(t_pipeADT pipe, char *buffer, uint64_t size);
int sys_writePipe(t_pipeADT pipe, char *buffer, uint64_t size);
t_sem * sys_createSem(char *name);
t_sem * sys_openSem(char *name);
void sys_closeSem(t_sem * sem);
void sys_wait(t_sem * sem, uint64_t pid);
void sys_post(t_sem * sem);
void sys_printSems();


systemCall sysCalls[] = { 0, 0, 0,
        (systemCall) sys_read, 				// 3
		(systemCall) sys_write, 			// 4
        (systemCall) sys_clear,				// 5
		(systemCall) sys_draw,				// 6
		(systemCall) sys_time,				// 7
		(systemCall) sys_getPid,			// 8
		(systemCall) sys_newProcess,		// 9
		(systemCall) sys_freeProcess,		// 10
		(systemCall) sys_free,				// 11
		(systemCall) sys_ticks,				// 12
		(systemCall) sys_ticksPerSecond,	// 13
		(systemCall) sys_usedMem,			// 14
		(systemCall) sys_freeMem,			// 15
		(systemCall) sys_malloc,			// 16
		(systemCall) sys_readPipe,			// 17
		(systemCall) sys_writePipe,			// 18
		(systemCall) sys_createSem,			// 19
		(systemCall) sys_openSem,			// 20
		(systemCall) sys_closeSem,			// 21
		(systemCall) sys_wait,				// 22
		(systemCall) sys_post,				// 23
		(systemCall) sys_printSems,			// 24
};

int syscallHandler(unsigned long rdi, unsigned long rsi, unsigned long rdx, unsigned long rcx, unsigned long r8, unsigned long r9, unsigned long r10){
	return sysCalls[rdi](rsi, rdx, rcx, r8, r9, r10);
}

int sys_ticks(int * ticks) {
	*ticks = ticks_elapsed();
	return *ticks;
}

int sys_ticksPerSecond(int * ticks) {
	*ticks = seconds_elapsed();
	return *ticks;
}

int sys_clear() {
	clearAll();
	return 0;
}

/*
 * https://jameshfisher.com/2018/02/19/how-to-syscall-in-c/
 * fd = 0 (stdin)
 */
int sys_read(uint64_t fd, char *buffer, uint64_t size){
	uint64_t i = 0;
	char c;
	if (fd == 0){
		while(size > 0 && (c = get_key_input())) {	    //get_key_input devuelvo 0 si el buffer esta vacio
			buffer[i++] = c;
			size--;
		}
	}
	return i;
}

//fd = 1 (stdout)
int sys_write(uint64_t fd, char *buffer, uint64_t size){
	uint64_t i = 0;

	if (fd == 1) {
		while(size--) {
			char c = *buffer;
			if (c == '\n') {
				newLine();
			} else if (c == '\b') {
				backspace();
			} else {
				printChar(c,0,255,0);
			}
			buffer++;
			i++;
		}
	}

	return i;
}


int sys_draw(uint64_t x, uint64_t y, unsigned char r, unsigned char g, unsigned char b) {
	putPixel(x,y,r,g,b);
	return 0;
}

int * sys_time(int * time) {
	uint64_t hour = getHour();
	uint64_t min = getMin();
	uint64_t sec = getSec(); 

	switch(hour){
		case 0: hour = 21;
				break;
		case 1: hour = 22;
				break;
		case 2: hour = 23;
				break;
		default: hour -= 3;
	}

	time[0] = hour/10;
	time[1] = hour%10;
	time[2] = min/10;
	time[3] = min%10;
	time[4] = sec/10;
	time[5] = sec%10;;
	return time;
}

uint64_t sys_usedMem(){
	return usedMemory();
}

uint64_t sys_freeMem(void){
	return freeMemoryLeft();
}

void * sys_malloc(uint64_t size, uint64_t pid){
	return pmalloc(size, pid);
}
void sys_free(void * address, uint64_t pid){
	pfree(address, pid);
}

int sys_readPipe(t_pipeADT pipe, char *buffer, uint64_t size){
	return read(pipe, buffer, size);
}

int sys_writePipe(t_pipeADT pipe, char *buffer, uint64_t size){
	return write(pipe, buffer, size);
}
void * sys_newProcess(char * name, int(* foo)(int argc, char** argv), int ppid, int argc, char * argv[], void *trash){
	return newProcess(name, foo, ppid, argc, argv, S_P_LOW, S_M_FOREGROUND);
}

void sys_freeProcess(void * process){
	freeProcess(process);
}

int sys_getPid(void * process){
	return getProcessPid(process);
}

t_sem * sys_createSem(char *name){
	return createSem(name);
}

t_sem * sys_openSem(char *name){
	return openSem(name);
}

void sys_closeSem(t_sem * sem){
	return closeSem(sem);
}

void sys_wait(t_sem * sem, uint64_t pid){
	wait(sem, 0); // GET CURRENT PID
}

void sys_post(t_sem * sem){
	post(sem);
}

void sys_printSems(){
	char * str = semListString();
	sys_write(1, str, strlen(str));
	pfree(str, 0);
}