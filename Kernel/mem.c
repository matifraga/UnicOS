#include <stdint.h>
#include <mem.h>
#include "video.h"
#include "kernel.h"

static const uint64_t page_size = 0x1000;

static uint64_t* pmm_stack_start = (uint64_t*)(11*0x100000);
static uint64_t* pmm_stack_end = (uint64_t*)(12*0x100000);
static uint64_t* pmm_stack_current;

static void* mallocBuffer = (6*0x100000);

static void* lastMalloc;

void* malloc(int len) {
	lastMalloc = mallocBuffer;
	//video_write_string(KERNEL_CONSOLE, "***malloc: 0x");
	//video_write_hex(KERNEL_CONSOLE, (uint64_t)lastMalloc);
	//video_write_nl(KERNEL_CONSOLE);
	//video_write_string(KERNEL_CONSOLE, "******: 0x");
	//video_write_hex(KERNEL_CONSOLE, (uint64_t)(mallocBuffer + len));
	//video_write_nl(KERNEL_CONSOLE);
	if((mallocBuffer + len) > pmm_stack_start){
		mem_panic();
	}
	mallocBuffer += len;

	return lastMalloc;
}

void* calloc(int len) {
	char* space = (char*)malloc(len);

	for (int i = 0; i < len; i++) {
		space[i] = (char)0;
	}

	return (void*)space;
}

void free(void* m) {

	if (m == lastMalloc) {
		mallocBuffer = m;
	}

}

void pmm_initialize() {
	// End of the system memory is 512 MB
	video_write_string(KERNEL_CONSOLE, "Initializing PMM");
	video_write_nl(KERNEL_CONSOLE);
	uint64_t end_of_memory = 512*0x100000;
	video_write_string(KERNEL_CONSOLE, "  End of total memory: 0x");
	video_write_hex(KERNEL_CONSOLE, (uint64_t)end_of_memory);
	video_write_nl(KERNEL_CONSOLE);
	video_write_string(KERNEL_CONSOLE, "  Kernel heap start: 0x");
	video_write_hex(KERNEL_CONSOLE, (uint64_t)0x600000);
	video_write_nl(KERNEL_CONSOLE);
	video_write_string(KERNEL_CONSOLE, "  Kernel heap end: 0x");
	video_write_hex(KERNEL_CONSOLE, (uint64_t)pmm_stack_start);
	video_write_nl(KERNEL_CONSOLE);
	video_write_string(KERNEL_CONSOLE, "  PMM start: 0x");
	video_write_hex(KERNEL_CONSOLE, (uint64_t)pmm_stack_start);
	video_write_nl(KERNEL_CONSOLE);
	video_write_string(KERNEL_CONSOLE, "  PMM end: 0x");
	video_write_hex(KERNEL_CONSOLE, (uint64_t)pmm_stack_end);
	video_write_nl(KERNEL_CONSOLE);

	uint64_t start_of_free_memory = (uint64_t)pmm_stack_end;
	pmm_stack_current = pmm_stack_start;
	uint64_t nframes = (end_of_memory - start_of_free_memory)/page_size;
	uint64_t i;
	for(i=0 ; i < nframes; i++){
		*pmm_stack_current = start_of_free_memory+(i*page_size);
		pmm_stack_current++;
	}
	video_write_string(KERNEL_CONSOLE, "  PMM current: 0x");
	video_write_hex(KERNEL_CONSOLE, (uint64_t)pmm_stack_current);
	video_write_nl(KERNEL_CONSOLE);

	//page_frame_test();
}

void* pmm_page_alloc(){
	pmm_stack_current--;
	return (void*)(*pmm_stack_current);
}

void pmm_page_free(void* dir){
	*pmm_stack_current = (uint64_t)dir;
	pmm_stack_current++;
}

void mem_panic() {
	intsoff();
	video_write_string(KERNEL_CONSOLE, "PANIC: OUT OF MEMORY!");
	while(1);
}

void page_frame_test() {
	int i;
	video_write_string(KERNEL_CONSOLE, "Printing last frames in stack:");
	video_write_nl(KERNEL_CONSOLE);
	for(i=0;i<3;i++){
		video_write_string(KERNEL_CONSOLE, "  frame: 0x");
		video_write_hex(KERNEL_CONSOLE, (uint64_t)*(pmm_stack_start+i));
		video_write_nl(KERNEL_CONSOLE);
	}
	video_write_string(KERNEL_CONSOLE, "Printing first frames in stack:");
	video_write_nl(KERNEL_CONSOLE);
	for(i=0;i<3;i++){
		video_write_string(KERNEL_CONSOLE, "  frame: 0x");
		video_write_hex(KERNEL_CONSOLE, (uint64_t)*(pmm_stack_current-i-1));
		video_write_nl(KERNEL_CONSOLE);
	}

	video_write_string(KERNEL_CONSOLE, "Pop and push some pages:");
	video_write_nl(KERNEL_CONSOLE);
	void* page1 = pmm_page_alloc();
	void* page2 = pmm_page_alloc();
	void* page3 = pmm_page_alloc();
	video_write_string(KERNEL_CONSOLE, "  pop: 0x");
	video_write_hex(KERNEL_CONSOLE, (uint64_t)(page1));
	video_write_nl(KERNEL_CONSOLE);
	video_write_string(KERNEL_CONSOLE, "  pop: 0x");
	video_write_hex(KERNEL_CONSOLE, (uint64_t)(page2));
	video_write_nl(KERNEL_CONSOLE);
	video_write_string(KERNEL_CONSOLE, "  pop: 0x");
	video_write_hex(KERNEL_CONSOLE, (uint64_t)(page3));
	video_write_nl(KERNEL_CONSOLE);

	video_write_string(KERNEL_CONSOLE, "  push: 0x");
	video_write_hex(KERNEL_CONSOLE, (uint64_t)(page1));
	video_write_nl(KERNEL_CONSOLE);
	pmm_page_free(page1);
	video_write_string(KERNEL_CONSOLE, "  push: 0x");
	video_write_hex(KERNEL_CONSOLE, (uint64_t)(page2));
	video_write_nl(KERNEL_CONSOLE);
	pmm_page_free(page2);
	video_write_string(KERNEL_CONSOLE, "  push: 0x");
	video_write_hex(KERNEL_CONSOLE, (uint64_t)(page3));
	video_write_nl(KERNEL_CONSOLE);
	pmm_page_free(page3);

	page1 = pmm_page_alloc();
	page2 = pmm_page_alloc();
	page3 = pmm_page_alloc();
	video_write_string(KERNEL_CONSOLE, "  pop: 0x");
	video_write_hex(KERNEL_CONSOLE, (uint64_t)(page1));
	video_write_nl(KERNEL_CONSOLE);
	video_write_string(KERNEL_CONSOLE, "  pop: 0x");
	video_write_hex(KERNEL_CONSOLE, (uint64_t)(page2));
	video_write_nl(KERNEL_CONSOLE);
	video_write_string(KERNEL_CONSOLE, "  pop: 0x");
	video_write_hex(KERNEL_CONSOLE, (uint64_t)(page3));
	video_write_nl(KERNEL_CONSOLE);
}