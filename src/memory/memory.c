#include <memory/memory.h>
#include "queue/queue.h"
#include "log/log.h"
#include "memory/share_memory.h"
#if OS_UBUNTU
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

void* space_malloc(size_t size)
{
	return malloc(size);
}

void space_free(void* ptr)
{
	free(ptr);
}

int ubuntu_phyaddr2virtaddr(virtaddr_parameter* vp, unsigned int phyaddr)
{
        if (pointer_check(vp)) {
                loge("phyaddr2virtaddr fail, vp is null, please check vp\n");
                return RET_ERROR;
        }
        vp->fd = open(MEM_DEV_NAME, O_RDWR | O_SYNC);
        if (vp->fd < 0) {
                loge("phyaddr2virtaddr fail, open /dev/mem error\n");
                return RET_ERROR;
        }
        vp->map_base = mmap(0, vp->map_size, PROT_READ | PROT_WRITE, MAP_SHARED, vp->fd, phyaddr & ~MAP_MASK);
        if (vp->map_base == (void *)-1) {
                loge("phyaddr2virtaddr fail, mmap error\n");
                close(vp->fd);
                return RET_ERROR;
        }
        vp->virtaddr = vp->map_base + (phyaddr & MAP_MASK);
        return 0;
}

void ubuntu_virtaddr_free(virtaddr_parameter* vp)
{
        int ret = 0;

        if (pointer_check(vp)) {
                loge("virtaddr_free fail, vp is null, please check vp\n");
                return;
        }
        ret = munmap(vp->map_base, vp->map_size);
        if (ret < 0) {
                loge("virtaddr_free fail, munmap fail\n");
                close(vp->fd);
                return;
        }
        close(vp->fd);
        return;
}

int ubuntu_phymem_write(virtaddr_parameter* vp, unsigned int phyaddr, unsigned char *data, unsigned int len)
{
        int ret = RET_OK;
        int i;
        int cnt_num = 0;

        if (pointer_check(vp) || pointer_check(data)) {
                loge("ubuntu_phymem_write fail, vp or data is null, please check vp or data\n");
                return RET_ERROR;
        }
        if (len <= 0) {
                loge("ubuntu_phymem_write fail, len = 0, please check len\n");
                return RET_ERROR;
        }
        ret = ubuntu_phyaddr2virtaddr(vp, phyaddr);
        if (ret < 0) {
                loge("ubuntu_phymem_write fail, ubuntu_phyaddr2virtaddr fail, return ret.%d\n", ret);
                return RET_ERROR;
        }
        for (i = 0; i < len; i++) {
                if (cnt_num >= MAP_MASK) {
                        cnt_num = 0;
                        ret = munmap(vp->map_base, MAP_MASK);
                        if (ret < 0) {
                                loge("ubuntu_phymem_write fail, munmap fail, return ret.%d\n", ret);
                                close(vp->fd);
                                return RET_ERROR;
                        }
                        vp->map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, vp->fd, phyaddr & ~MAP_MASK);
                        if (vp->map_base == (void *)-1) {
                                loge("ubuntu_phymem_write fail, mmap fail\n");
                                close(vp->fd);
                                return RET_ERROR;
                        }
                        vp->virtaddr = vp->map_base + (phyaddr & MAP_MASK);
                }
                *((unsigned char *)vp->virtaddr) = data[i];
                phyaddr += 1;
                cnt_num += 1;
        }
        ubuntu_virtaddr_free(vp);
        return ret;
}

int ubuntu_phymem_read(virtaddr_parameter* vp, unsigned int phyaddr, unsigned char *data, unsigned int len)
{
        int ret = RET_OK;
        int i;
        int cnt_num = 0;

        if (pointer_check(vp) || pointer_check(data)) {
                loge("ubuntu_phymem_write fail, vp or data is null, please check vp or data\n");
                return RET_ERROR;
        }
        if (len <= 0) {
                loge("ubuntu_phymem_write fail, len = 0, please check len\n");
                return RET_ERROR;
        }
        ret = ubuntu_phyaddr2virtaddr(vp, phyaddr);
        if (ret < 0) {
                loge("ubuntu_phymem_write fail, ubuntu_phyaddr2virtaddr fail, return ret.%d\n", ret);
                return RET_ERROR;
        }
        for (i = 0; i < len; i++) {
                if (cnt_num >= MAP_MASK) {
                        cnt_num = 0;
                        ret = munmap(vp->map_base, MAP_MASK);
                        if (ret < 0) {
                                loge("ubuntu_phymem_write fail, munmap fail, return ret.%d\n", ret);
                                close(vp->fd);
                                return RET_ERROR;
                        }
                        vp->map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, vp->fd, phyaddr & ~MAP_MASK);
                        if (vp->map_base == (void *)-1) {
                                loge("ubuntu_phymem_write fail, mmap fail\n");
                                close(vp->fd);
                                return RET_ERROR;
                        }
                        vp->virtaddr = vp->map_base + (phyaddr & MAP_MASK);
                }
                data[i] = *((unsigned char *)vp->virtaddr);
                phyaddr += 1;
                cnt_num += 1;
        }
        ubuntu_virtaddr_free(vp);
        return ret;
}
#endif
