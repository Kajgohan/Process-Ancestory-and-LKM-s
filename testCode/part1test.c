#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// These values must match the syscall table
#define __NR_open 5
#define __NR_close 6
#define __NR_read 3
#define __NR_cs3013_syscall1 377

long testCall1 (void) {
  return (long) syscall(__NR_cs3013_syscall1);
}

long testCallOpen (const char *filename, int flags, int mode) {
  return (long) syscall(__NR_open,filename,flags,mode);
}

long testCallClose (unsigned int fd) {
  return (long) syscall(__NR_close,fd);
}

long testCallRead(int fd, void *buf, size_t count){
	return (long) syscall(__NR_read, fd, buf, count);
}

int main () {
	void *virusBuf[1000];
	void *novirusBuf[1000];
  printf("The return values of the system calls are:\n\n");
  printf("\tcs3013_syscall1: %ld\n",testCall1());
  printf("\tcreating and opening a file: %ld\n",testCallOpen("foo.txt",O_CREAT,S_IRWXU));
	printf("\topening an existing file: %ld\n",testCallOpen("openclose.txt",O_RDWR,S_IRWXU));
	unsigned int fd = testCallOpen("openclose.txt",O_RDWR,S_IRWXU);
	printf("\tclosing a file: %ld\n",testCallClose(fd));
	unsigned int vFD = testCallOpen("virus.txt", O_RDWR, S_IRWXU);
	unsigned int novFD = testCallOpen("novirus.txt",O_RDWR, S_IRWXU);
	printf("\treading a file wih VIRUS: %ld\n",testCallRead(vFD, virusBuf, sizeof(*virusBuf)));
	printf("\treading a file wihout VIRUS: %ld\n",testCallRead(novFD, novirusBuf, sizeof(*novirusBuf)));
  return 0;
}
