#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <comm/can/can.h>
#include "memory/share_memory.h"
#include "queue/queue.h"
#include "log/log.h"
#include "comm/rs422/ubuntu/ubuntu_rs422.h"
#include "comm/can/can_data_covenant/can_data_covenant.h"
#include "comm/rs422/rs422_data_covenant/can_data_covenant.h"
#include "comm/rapidio/ubuntu/ubuntu_rapidio.h"
#if OS_UBUNTU
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>

#define PCIE_XDMA_DEV_LENGTH         64
#define PIC_MEM_SIZE               4096
#define DMA_MEM_ADDR          0x4000000
#define FPGA_DDR_START_ADDR           0
#define FPGA_BRAM_START_ADDR 0xC0000000

#define PCITEST_DMA_WRITE	_IOW('P', 0xa, unsigned long)
#define PCITEST_DMA_READ	_IOW('P', 0xb, unsigned long)

enum DDR_TYPE {
	FPGA_DDR_V7TO810A,
	FPGA_DDR_810ATOV7,
	FPGA_BRAM,
};

typedef struct pcie_xdma_file {
	const char* file_path;
	const char* dev_name;
	int fd;
} pcie_xdma_file;

typedef struct pcie_parameter {
	pcie_xdma_file xdma_c2h;
	pcie_xdma_file xdma_h2c;
	pcie_xdma_file xdma_user;
	pcie_xdma_file pcie_ep;
	pcie_xdma_file pcie_rc;
	unsigned char *h2c_align_mem;
	unsigned char *c2h_align_mem;
	unsigned char pci_dev_num;
} pcie_parameter;

int ubuntu_pcie_init(pcie_parameter* pp)
{
        int ret = RET_OK;
        char dev_name[PCIE_XDMA_DEV_LENGTH];

        if (pointer_check(pp)) {
                loge("ubuntu_pcie_init fail, pp is null, please check pp\n");
                return RET_ERROR;
        }
        bzero(dev_name, PCIE_XDMA_DEV_LENGTH);
        ret = sprintf(dev_name, "%s%s", pp->xdma_c2h.file_path, pp->xdma_c2h.dev_name);//stdio.h
        if (ret < 0) {
                loge("ubuntu_pcie_init fail, sprintf fail, return ret.%d\n", ret);
                return RET_ERROR;
        }
        pp->xdma_c2h.fd =  open(dev_name,  O_RDWR | O_NONBLOCK);
        if (pp->xdma_c2h.fd < 0) {
                loge("ubuntu_pcie_init open %s error, error info:%s(%d)\n", dev_name, strerror(errno), errno);//string.h
                return RET_ERROR;
        }
	bzero(dev_name, PCIE_XDMA_DEV_LENGTH);
        ret = sprintf(dev_name, "%s%s", pp->xdma_h2c.file_path, pp->xdma_h2c.dev_name);//stdio.h
        if (ret < 0) {
                loge("ubuntu_pcie_init fail, sprintf fail, return ret.%d\n", ret);
                return RET_ERROR;
        }
        pp->xdma_h2c.fd =  open(dev_name,  O_RDWR);
        if (pp->xdma_h2c.fd < 0) {
                loge("ubuntu_pcie_init open %s error, error info:%s(%d)\n", dev_name, strerror(errno), errno);//string.h
                return RET_ERROR;
        }
	ret = posix_memalign((void *)&pp->h2c_align_mem, PIC_MEM_SIZE, DMA_MEM_ADDR);
	if (ret < 0) {
		loge("ubuntu_pcie_init fail, posix_memalign fail, return ret.%d\n", ret);
		return RET_ERROR;
	}
	ret = posix_memalign((void *)&pp->c2h_align_mem, PIC_MEM_SIZE, DMA_MEM_ADDR);
	if (ret < 0) {
		loge("ubuntu_pcie_init fail, posix_memalign fail, return ret.%d\n", ret);
		return RET_ERROR;
	}
	if (NULL == pp->h2c_align_mem || NULL == pp->c2h_align_mem) {
		loge("ubuntu_pcie_init fail, h2c_align_mem or c2h_align_mem is NULL \n");
		return RET_ERROR;
	}
        return ret;
}

int ubuntu_pcie_write(pcie_parameter* pp, unsigned long data_size)
{
        int ret = RET_OK;
        char dev_name[PCIE_XDMA_DEV_LENGTH];

        if (pointer_check(pp)) {
                loge("ubuntu_pcie_write fail, pp is null, please check pp\n");
                return RET_ERROR;
        }
        bzero(dev_name, PCIE_XDMA_DEV_LENGTH);
        ret = sprintf(dev_name, "%s%s%d", pp->pcie_ep.file_path, pp->pcie_ep.dev_name, pp->pci_dev_num);//stdio.h
        if (ret < 0) {
                loge("ubuntu_pcie_write fail, sprintf fail, return ret.%d\n", ret);
                return RET_ERROR;
        }
        pp->pcie_ep.fd =  open(dev_name,  O_RDWR);
        if (pp->pcie_ep.fd < 0) {
                loge("ubuntu_pcie_write open %s error, error info:%s(%d)\n", dev_name, strerror(errno), errno);//string.h
                return RET_ERROR;
        }
	ret = ioctl(pp->pcie_ep.fd, PCITEST_DMA_WRITE, data_size);
	if (ret < 0) {
		loge("ubuntu_pcie_write fail, ioctl cmd PCITEST_DMA_WRITE fail, return ret.%d\n", ret);
		return RET_ERROR;
	}
	close(pp->pcie_ep.fd);
	return 0;
}

int ubuntu_pcie_read(pcie_parameter* pp, unsigned long data_size)
{
        int ret = RET_OK;
        char dev_name[PCIE_XDMA_DEV_LENGTH];

        if (pointer_check(pp)) {
                loge("ubuntu_pcie_read fail, pp is null, please check pp\n");
                return RET_ERROR;
        }
        bzero(dev_name, PCIE_XDMA_DEV_LENGTH);
        ret = sprintf(dev_name, "%s%s%d", pp->pcie_ep.file_path, pp->pcie_ep.dev_name, pp->pci_dev_num);//stdio.h
        if (ret < 0) {
                loge("ubuntu_pcie_read fail, sprintf fail, return ret.%d\n", ret);
                return RET_ERROR;
        }
        pp->pcie_ep.fd =  open(dev_name,  O_RDWR);
        if (pp->pcie_ep.fd < 0) {
                loge("ubuntu_pcie_read open %s error, error info:%s(%d)\n", dev_name, strerror(errno), errno);//string.h
                return RET_ERROR;
        }
        ret = ioctl(pp->pcie_ep.fd, PCITEST_DMA_READ, data_size);
        if (ret < 0) {
                loge("ubuntu_pcie_read fail, ioctl cmd PCITEST_DMA_WRITE fail, return ret.%d\n", ret);
                return RET_ERROR;
        }
        close(pp->pcie_ep.fd);
        return 0;
}

int ubuntu_h2c_transfer(pcie_parameter* pp, enum DDR_TYPE ddr_type, unsigned int size, unsigned char *h2c_align_mem, unsigned int offset)
{
	unsigned int rc = 0;

        if (pointer_check(pp) || pointer_check(h2c_align_mem)) {
                loge("ubuntu_h2c_transfer fail, pp or h2c_align_mem is null, please check pp or h2c_align_mem\n");
                return RET_ERROR;
        }
	logi("ubuntu_h2c_transfer ddr_type.%d\n", ddr_type);
	switch (ddr_type) {
		case  FPGA_DDR_810ATOV7:
			memcpy(pp->h2c_align_mem, h2c_align_mem, size);
			lseek(pp->xdma_h2c.fd, FPGA_DDR_START_ADDR + offset, SEEK_SET);
			while (size > 0) {
				rc = write(pp->xdma_h2c.fd, pp->h2c_align_mem, size);
				if (size > 0) {
					size -= rc;
					h2c_align_mem += rc;
					memcpy(pp->h2c_align_mem, h2c_align_mem, size);
				} else {
					size -= rc;
				}
			}
			break;
		case  FPGA_BRAM:
			memcpy(pp->h2c_align_mem, h2c_align_mem, size);
			lseek(pp->xdma_h2c.fd, FPGA_BRAM_START_ADDR + offset, SEEK_SET);
			write(pp->xdma_h2c.fd, pp->h2c_align_mem, size);
			break;
		default:
			loge("ubuntu_h2c_transfer error:ddr_type.%d\n", ddr_type);
			break;
	}
	return 0;
}

int ubuntu_c2h_transfer(pcie_parameter* pp, enum DDR_TYPE ddr_type, unsigned int size, unsigned char *c2h_align_mem, unsigned int offset)
{
        if (pointer_check(pp) || pointer_check(c2h_align_mem)) {
                loge("ubuntu_c2h_transfer fail, pp or c2h_align_mem, is null, please check pp or c2h_align_mem\n");
                return RET_ERROR;
        }
        logi("ubuntu_c2h_transfer ddr_type.%d\n", ddr_type);
        switch (ddr_type) {
                case  FPGA_DDR_V7TO810A:
                        lseek(pp->xdma_c2h.fd, FPGA_DDR_START_ADDR + offset, SEEK_SET);
			read(pp->xdma_c2h.fd, pp->c2h_align_mem, size);
			memcpy(pp->c2h_align_mem, c2h_align_mem, size);
                        break;
                case  FPGA_BRAM:
                        lseek(pp->xdma_c2h.fd, FPGA_DDR_START_ADDR + offset, SEEK_SET);
                        read(pp->xdma_c2h.fd, pp->c2h_align_mem, size);
                        memcpy(pp->c2h_align_mem, c2h_align_mem, size);
                        break;
                default:
                        loge("ubuntu_c2h_transfer error:ddr_type.%d\n", ddr_type);
                        break;
        }
        return 0;
}

int ubuntu_pcie_rc_to_ep_transfer(pcie_parameter* pp, virtaddr_parameter* vp,  unsigned int phyaddr, unsigned char ep_devnum, 
		                  unsigned char *data, unsigned int size)   //todo check code , open dev name need right
{
        int ret = RET_OK;
	char dev_name[PCIE_XDMA_DEV_LENGTH];
	unsigned int phyaddr_tmp = phyaddr;

        if (pointer_check(pp) || pointer_check(vp) || pointer_check(data)) {
                loge("ubuntu_pcie_rc_to_ep_transfer fail, pp vp or data is null, please check pp vp data\n");
                return RET_ERROR;
        }
	ret = ubuntu_phyaddr2virtaddr(vp, phyaddr_tmp);
	if (ret < 0) {
                loge("ubuntu_pcie_rc_to_ep_transfer fail, ubuntu_pcie_rc_to_ep_transfer fail, return ret.%d\n", ret);
                return RET_ERROR;
	}
        bzero(dev_name, PCIE_XDMA_DEV_LENGTH);
        ret = sprintf(dev_name, "%s%s%d", pp->pcie_ep.file_path, pp->pcie_ep.dev_name, ep_devnum);
        if (ret < 0) {
                loge("ubuntu_pcie_rc_to_ep_transfer fail, sprintf fail, return ret.%d\n", ret);
		close(vp->fd);
                return RET_ERROR;
        }
	pp->pcie_ep.fd = open(dev_name, O_RDWR);
        if (pp->pcie_ep.fd < 0) {
                loge("ubuntu_pcie_rc_to_ep_transfer open %s error, error info:%s(%d)\n", dev_name, strerror(errno), errno);
		close(vp->fd);
                return RET_ERROR;
        }
	*((unsigned char *)vp->virtaddr) = 0;
	phyaddr += 1;
	vp->virtaddr = vp->map_base + (phyaddr_tmp & MAP_MASK);
	*((unsigned char *)vp->virtaddr) = 0;
	phyaddr_tmp = phyaddr;
	phyaddr_tmp += 2;
	vp->virtaddr = vp->map_base + (phyaddr_tmp & MAP_MASK);
	memcpy(vp->virtaddr, data, size);
	ret = ioctl(pp->pcie_ep.fd , PCITEST_DMA_WRITE, size + 2);
        if (ret < 0) {
                loge("ubuntu_pcie_rc_to_ep_transfer fail, ioctl PCITEST_DMA_WRITE fail, return ret.%d\n", ret);
		close(pp->pcie_ep.fd);
		close(vp->fd);
                return RET_ERROR;
        }
	phyaddr_tmp = phyaddr;
	vp->virtaddr = vp->map_base + (phyaddr_tmp & MAP_MASK);
	*((unsigned char *)vp->virtaddr) = 1;
        ret = ioctl(pp->pcie_ep.fd , PCITEST_DMA_WRITE, 1);
        if (ret < 0) {
                loge("ubuntu_pcie_rc_to_ep_transfer fail, ioctl PCITEST_DMA_WRITE fail, return ret.%d\n", ret);
		close(pp->pcie_ep.fd);
		close(vp->fd);
                return RET_ERROR;
        }
	ubuntu_virtaddr_free(vp);
	close(pp->pcie_ep.fd);
	return 0;
}

int ubuntu_pcie_check_ack(pcie_parameter* pp, virtaddr_parameter* vp, unsigned int phyaddr, unsigned char ep_devnum,
		          unsigned int check_value, unsigned char *data, unsigned int size)
{
        int ret = RET_OK;
        char dev_name[PCIE_XDMA_DEV_LENGTH];
	unsigned int phyaddr_tmp = phyaddr;

        if (pointer_check(pp) || pointer_check(vp) || pointer_check(data)) {
                loge("ubuntu_pcie_check_ack fail, pp vp or data is null, please check pp vp data\n");
                return RET_ERROR;
        }
        ret = ubuntu_phyaddr2virtaddr(vp, phyaddr_tmp);
        if (ret < 0) {
                loge("ubuntu_pcie_check_ack fail, ubuntu_pcie_rc_to_ep_transfer fail, return ret.%d\n", ret);
                return RET_ERROR;
        }
        bzero(dev_name, PCIE_XDMA_DEV_LENGTH);
        ret = sprintf(dev_name, "%s%s%d", pp->pcie_ep.file_path, pp->pcie_ep.dev_name, ep_devnum);
        if (ret < 0) {
                loge("ubuntu_pcie_check_ack fail, sprintf fail, return ret.%d\n", ret);
                close(vp->fd);
                return RET_ERROR;
        }
        pp->pcie_ep.fd = open(dev_name, O_RDWR);
        if (pp->pcie_ep.fd < 0) {
                loge("ubuntu_pcie_check_ack open %s error, error info:%s(%d)\n", dev_name, strerror(errno), errno);
                close(vp->fd);
                return RET_ERROR;
        }
	while (true) {
		ret = ioctl(pp->pcie_ep.fd, PCITEST_DMA_READ, 2);
		if (ret < 0) {
			loge("ubuntu_pcie_check_ack fail, ioctl PCITEST_DMA_READ, fail, return ret.%d\n", ret);
			close(pp->pcie_ep.fd);
			close(vp->fd);
			return RET_ERROR;
		}
		if ((*(unsigned char *)(vp->virtaddr + 1)) == check_value) { //todo need check
			break;
		}
	}
	ret = ioctl(pp->pcie_ep.fd, PCITEST_DMA_READ, size);
        if (ret < 0) {
                loge("ubuntu_pcie_check_ack fail, ioctl PCITEST_DMA_READ fail, return ret.%d\n", ret);
                close(pp->pcie_ep.fd);
                close(vp->fd);
                return RET_ERROR;
        }
	phyaddr_tmp += 2;
	vp->virtaddr = vp->map_base + (phyaddr_tmp & MAP_MASK);
	memcpy(data, vp->virtaddr, size - 2);
        ubuntu_virtaddr_free(vp);
        close(pp->pcie_ep.fd);
        return 0;
}

int ubuntu_pcie_deinit(pcie_parameter* pp)
{
        int ret = RET_OK;
        if (pointer_check(pp)) {
                loge("ubuntu_pcie_deinit fail, pp is null, please check pp\n");
                return RET_ERROR;
        }
	close(pp->xdma_h2c.fd);
	close(pp->xdma_c2h.fd);
	return ret;
}
#endif
