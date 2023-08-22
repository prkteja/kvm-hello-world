#include <stddef.h>
#include <stdint.h>
#include <guest.h>
// #include <stdlib.h>

struct file_t{
	char* path;
	int fd;
	int flags;
	int mode;
	void* buf;
	uint32_t size;
};

static void outb(uint16_t port, uint32_t value) {
	asm("out %0,%1" : /* empty */ : "a" (value), "Nd" (port) : "memory");
}

static void outb8(uint16_t port, uint8_t value) {
	asm("outb %0,%1" : /* empty */ : "a" (value), "Nd" (port) : "memory");
}

static inline uint32_t inb(uint16_t port) {
	uint32_t ret;
	asm("in %1, %0" : "=a"(ret) : "Nd"(port) : "memory" );
	return ret;
}

static void printVal(uint32_t val) {
	outb(0xEA, val);
}

static uint32_t getNumExits() {
	return (uint32_t) inb(0xEB);
}

static void display(const char *str){
	outb(0xEC, (uintptr_t)str);
}

static int open(char *path, int flags){
	struct file_t file;
	file.path = path;
	file.fd = 0;
	file.flags = flags;
	file.mode = 0644;
	file.buf = NULL;
	file.size = 0;
	outb(0xED, (uintptr_t) &file);
	printVal(file.fd);
	return file.fd;
}

// static void test(){
// 	int a = 0;
// 	int *b = &a;
// 	outb(0xEA, (uint32_t) b);
// }

static int read(int fd, void* buf, uint32_t size){
	struct file_t file;
	file.fd = fd;
	file.buf = buf;
	file.size = size;
	outb(0xEE, (uintptr_t) &file);
	return file.size;
}

static int write(int fd, void* buf, uint32_t size){
	struct file_t file;
	file.fd = fd;
	file.buf = buf;
	file.size = size;
	outb(0xEF, (uintptr_t) &file);
	return file.size;
}

static int creat(char* path, int mode){
	struct file_t file;
	file.path = path;
	file.fd = 0;
	file.flags = O_WRONLY | O_CREAT | O_TRUNC;
	file.mode = mode;
	file.buf = NULL;
	file.size = 0;
	outb(0xED, (uintptr_t) &file);
	printVal(file.fd);
	return file.fd;
}

static int close(int fd){
	int ret = fd;
	outb(0xF0, (uintptr_t) &ret);
	return ret;
}

static int lseek(int fd, int offset, int ref){
	int args[3] = {fd, offset, ref};
	outb(0xF1, (uintptr_t) args);
	return args[0];
}

void
__attribute__((noreturn))
__attribute__((section(".start")))
_start(void) {
	const char *p;

	for (p = "Hello, world!\n"; *p; ++p)
		outb8(0xE9, *p);
/*------------------------------------------------------*/
	int fd = open("test", O_RDWR);
	char buffer[100];
	int num_bytes = read(fd, buffer, 100);
	display(buffer);
	printVal(num_bytes);
	char *buffer1 = "aaaabbbbccccddddeeeeffffgggghhhh\n";
	int ret = lseek(fd, 0, SEEK_END);
	printVal(ret);
	int num_bytes1 = write(fd, buffer1, 20);
	printVal(num_bytes1);
	// char buf[100];
	// read(fd, buf, 100);
	// display(buf);
	close(fd);
	fd =creat("test2", 0644);
	close(fd);
	int numExits = getNumExits();
	printVal(numExits);
/*------------------------------------------------------*/
	*(long *) 0x400 = 42;

	for (;;)
		asm("hlt" : /* empty */ : "a" (42) : "memory");
}

