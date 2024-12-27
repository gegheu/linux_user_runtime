#ifndef __MEMORY__
#define __MEMORY__

#include <stdio.h>
#include <stdlib.h>

#define MEM_DEV_NAME            "/dev/mem"
#define MAP_SIZE                0x800000 //8M big page
#define MAP_MM_SIZE             0x10000
#define MAP_MASK                (MAP_MM_SIZE - 1) //MAP_MASK = 0XFFFF

typedef struct virtaddr_parameter {
        void* map_base;
        void* virtaddr;
        int fd;
        int map_size;
} virtaddr_parameter;

void* space_malloc(size_t size);
void space_free(void* ptr);
int ubuntu_phyaddr2virtaddr(virtaddr_parameter* vp, unsigned int phyaddr);
void ubuntu_virtaddr_free(virtaddr_parameter* vp);
int ubuntu_phymem_write(virtaddr_parameter* vp, unsigned int phyaddr, unsigned char *data, unsigned int len);
int ubuntu_phymem_read(virtaddr_parameter* vp, unsigned int phyaddr, unsigned char *data, unsigned int len);
#endif
