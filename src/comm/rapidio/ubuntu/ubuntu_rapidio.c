#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <comm/can/can.h>
#include "log/log.h"
#include "queue/queue.h"
#include "memory/share_memory.h"
#include "comm/rapidio/ubuntu/ubuntu_rapidio.h"
#include "comm/rapidio/rapidio_data_covenant/rapidio_data_covenant.h"
#if OS_UBUNTU
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#define RIO_MPORT_DRV_MAGIC      'm'
#define RIO_DIRECT_DMA           _IOW(RIO_MPORT_DRV_MAGIC, 27, struct rio_transaction)
#define RIO_DME_INIT             _IOW(RIO_MPORT_DRV_MAGIC, 33, unsigned int)
#define RIO_DMA_INIT             _IOW(RIO_MPORT_DRV_MAGIC, 34, unsigned int)
#define RIO_ID_SET_INIT          _IOW(RIO_MPORT_DRV_MAGIC, 35, struct id_set_info)
#define CHECK_DMA_STATUS         _IOW(RIO_MPORT_DRV_MAGIC, 36, unsigned int)
#define CHECK_DOORBELL_STATUS    _IOW(RIO_MPORT_DRV_MAGIC, 37, unsigned int)
#define RIO_DOORBELL_INIT        _IOW(RIO_MPORT_DRV_MAGIC, 39, unsigned int)
#define RIO_DOORBELL_RECEIVE     _IOW(RIO_MPORT_DRV_MAGIC, 40, unsigned int)
#define CHECK_PACKAGE_TYPE       _IOW(RIO_MPORT_DRV_MAGIC, 41, unsigned int)
#define RIO_DIRECT_DOORBELL      _IOW(RIO_MPORT_DRV_MAGIC, 47,struct rio_transfer_doorbell)

rapidio_parameter rp;
struct rio_transaction_doorbell_receive rio_doorbell_receive_info[DOORBELL_BUFFER_SIZE] = { 0 };
volatile unsigned char doorbell_arrival_flag = 0;


int ubuntu_rapidio_dma(rapidio_parameter* rp, struct rio_transaction* rio_tran, struct rio_transfer_io* rio_xfer)
{
	int ret = 0;

        if (pointer_check(rp) || pointer_check(rio_tran) || pointer_check(rio_xfer)) {
                loge("ubuntu_rapidio_dma fail, rp, rio_tran or rio_xfer is null, please check rp, rio_tran, rio_xfer\n");
                return RET_ERROR;
        }
	ret = ioctl(rp->fd, CHECK_DMA_STATUS, &(rio_tran->dir));
	if (ret < 0) {
		loge("ubuntu_rapidio_dma fail, ioctl CHECK_DMA_STATUS fail, DMA BUSY\n");
		close(rp->fd);
		return RET_ERROR;
	} else {
		logi("DAM state.%d\n", ret);
		switch (ret) {
			case DMA_WRITE_OK:
			case DMA_ALL_PASS:
				logi("DMA ok\n");
				break;
			case FETCH_ERROR:
			case UPDATA_ERROR:
			case AXI_FETCH_ERROR:
				loge("DMA data error\n");
				return RET_ERROR;
				break;
			default:
				loge("DMA absent state\n");
				return RET_ERROR;
				break;
		}
		ret = ioctl(rp->fd, RIO_DIRECT_DMA, rio_tran);
		if (ret < 0) {
			loge("ubuntu_rapidio_dma fail, ioctl RIO_DIRECT_DMA error\n");
			close(rp->fd);
			return RET_ERROR;
		}
		logi("ubuntu_rapidio_dma dma successful!\n");
	}
	return ret;
}

/*
 *      rp->rio_xfer.rioid = GRIO_RM_HOST_ID;               //dest_id
 *      rp->rio_xfer.rio_addr = APIO_remote_rio_ADDR_MAP;   //remote dev srio space start address
 *      rp->rio_xfer.loc_addr = APIO_DMA_RIO_ADDR_MAP;      //DMA send start address
 *      rp->rio_xfer.length = len;                          //sizeof *inout_buf length
 *
 *      rp->rio_tran.dir = RIO_TRANSFER_DIR_WRITE;          //RIO_TRANSFER_DIR_READ
 *      rp->rio_tran.dma_intr_mode = 0;                     //1 - enable dma interrupt
 *      rp->rio_tran.count = 1;                             //Must be 1 requested by RapidIO subsystem
 *      rp->rio_tran.block = (unsigned int)&(rp->rio_xfer);
 *      rp->rio_tran.check_flag = 1;                        //0-不在dma传输接口判断 1-在dma传输接口判断
 *
 *
 */
int ubuntu_rapidio_dma_desddr(rapidio_parameter* rp,  unsigned char *inout_buf,  virtaddr_parameter* vp, unsigned int phyaddr)
{
	int ret = 0;

        if (pointer_check(rp) || pointer_check(inout_buf) || pointer_check(vp)) {
                loge("ubuntu_rapidio_dma_desddr fail, rp , inout_buf or vp is null, please check rp, inout, vp\n");
                return RET_ERROR;
	}
	if (rp->rio_tran.dir == RIO_TRANSFER_DIR_WRITE) {//todo write macro change
		ret = ubuntu_phyaddr2virtaddr(vp, phyaddr);
		if (ret < 0) {
			loge("ubuntu_rapidio_dma_desddr write fail, ubuntu_phyaddr2virtaddr error, ret.%d\n", ret);
			return RET_ERROR;
		}
		memcpy(vp->virtaddr, inout_buf, rp->rio_xfer.length);
		ubuntu_virtaddr_free(vp);
		ret = ubuntu_rapidio_dma(rp, &(rp->rio_tran), &(rp->rio_xfer));
		if (ret < 0) {
			loge("ubuntu_rapidio_dma_desddr write fail, ubuntu_rapidio_dma error, ret.%d\n", ret);
			return RET_ERROR;
		}
	} else if(rp->rio_tran.dir == RIO_TRANSFER_DIR_READ) {//todo read macro change
                ret = ubuntu_rapidio_dma(rp, &(rp->rio_tran), &(rp->rio_xfer));
                if (ret < 0) {
                        loge("ubuntu_rapidio_dma_desddr read fail, ubuntu_rapidio_dma error, ret.%d\n", ret);
                        return RET_ERROR;
                }
                ret = ubuntu_phyaddr2virtaddr(vp, phyaddr);
                if (ret < 0) {
                        loge("ubuntu_rapidio_dma_desddr read fail, ubuntu_phyaddr2virtaddr error, ret.%d\n", ret);
                        return RET_ERROR;
                }
                memcpy(inout_buf, vp->virtaddr, rp->rio_xfer.length);
		ubuntu_virtaddr_free(vp);
	} else {
		loge("ubuntu_rapidio_dma_desddr fail, rio_tran.dir isnot write or read\n");
		return RET_ERROR;
	}
	return ret;

}

int ubuntu_inbound_addr_read(unsigned char* recv_buf, unsigned int len, virtaddr_parameter* vp, unsigned int phyaddr)
{
	int ret = 0;

        if (pointer_check(recv_buf) || pointer_check(vp)) {
                loge("ubuntu_inbound_addr_read fail, recv_buf or vp is null, please check recv_buf, vp\n");
                return RET_ERROR;
        }
	ret = ubuntu_phyaddr2virtaddr(vp, phyaddr);
	if (ret < 0) {
		loge("ubuntu_inbound_addr_read fail, ubuntu_phyaddr2virtaddr error\n");
		return RET_ERROR;
	}
	memcpy(recv_buf, vp->virtaddr, len);
	ubuntu_virtaddr_free(vp);
	return ret;
}

int ubuntu_doorbell_send(unsigned short int dest_id,unsigned char info, rapidio_parameter* rp)
{
	int ret = 0;
	struct rio_transfer_doorbell db_tran;

        if (pointer_check(rp)) {
                loge("ubuntu_doorbell_send fail, rp is null, please check rp\n");
                return RET_ERROR;
        }
        db_tran.rioid = dest_id;
        db_tran.data_p = info;
	ret = ioctl(rp->fd, CHECK_DOORBELL_STATUS);
	if (ret != DOOLBELL_PENDING) {
		if (ret == DOORBELL_OK) {
			logi("ubuntu_doorbell_send ok\n");
		} else {
			loge("ubuntu_doorbell_send fail\n");
			return RET_ERROR;
		}
		ret = ioctl(rp->fd, RIO_DIRECT_DOORBELL, &db_tran);
#if 0
		if (ret < 0) {
			loge("ubuntu_doorbell_send fail, ioctl RIO_DIRECT_DOORBELL error \n");
			close(rp->fd);
			return RET_ERROR;
		}
#endif
		logi("Outbound doorbell (0x%x) to 0x%x has been transmitted\n",info, dest_id);
		logi("ubuntu_doorbell_send, ret.%d\n",ret);
	} else {
		loge("Last doorbell pending\n");
		return RET_ERROR;
	}
	return 0;
}

int ubuntu_inbound_doorbell_receive(void)
{
	int ret = 0;
	int i;
	struct rio_transaction_doorbell rio_doorbell_receive;

        rio_doorbell_receive.count = DOORBELL_BUFFER_SIZE;
        rio_doorbell_receive.block = (unsigned int)(rio_doorbell_receive_info);
	for (i = 0; i < DOORBELL_BUFFER_SIZE; i++)
	{
		rio_doorbell_receive_info[i].receive_flag = 0;
	}
	ret = ioctl(rp.fd, RIO_DOORBELL_RECEIVE, &rio_doorbell_receive);
        if (ret < 0) {
                close(rp.fd);
                loge("inbound_doorbell_receive fail, ioctl RIO_DOORBELL_RECEIVE fail, ret.%d.\n", ret);
                return ret;
        }
	for (i = 0; i < DOORBELL_BUFFER_SIZE; i++)
	{
		if (1 == rio_doorbell_receive_info[i].receive_flag) {
			logi("Msg:%d src_id = 0x%x, info = 0x%x\n", rio_doorbell_receive_info[i].doorbell_num,  \
					rio_doorbell_receive_info[i].src_id,rio_doorbell_receive_info[i].data);
			doorbell_arrival_flag = 1;
		}
	}
	return 0;

}

void sigint_handler(int signo)
{
	int ret = 0;
	enum PACKAGE_TYPE type = MAX_TYPE;

	logi("sigint_handler rapio\n");
	type = ioctl(rp.fd, CHECK_PACKAGE_TYPE); //todo rp->fd need fix
	logi("rapidio package type.%d, signo.%d\n", type, signo);
	switch (type) {
		case DOORBELL_TYPE:
			ret = ubuntu_inbound_doorbell_receive();
			if (ret < 0) {
				close(rp.fd);
				loge("sigint_handler fail, inbound_doorbell_receive fail, ret.%d.\n", ret);
				return;
			}
			break;
		case DME_TYPE:
			logi("inbound_dme_receive\n");
			break;
		case DOORBELL_DME_TYPE:
			logi("inbound_doorbell_receive\n");
			logi("inbound_dme_receive\n");
			break;
		default:
			loge("package type error type.%d\n", type);
			break;
	}
	return;
}

int ubuntu_rapidio_init(rapidio_parameter* rp)
{
	int ret = RET_OK;
	int flags = 0;
	char dev_name[RAPIDIO_DEV_LENGTH];

        if (pointer_check(rp)) {
                loge("ubuntu_rapidio_init fail, rp is null, please check rp\n");
                return RET_ERROR;
        }
	bzero(dev_name, RAPIDIO_DEV_LENGTH);
        ret = sprintf(dev_name, "%s%s", rp->file_path, rp->dev_name);//stdio.h
        if (ret < 0) {
                loge("ubuntu_rapidio_init fail, sprintf fail, return ret.%d\n", ret);
                return RET_ERROR;
        }
        rp->fd =  open(dev_name,  O_RDWR | O_CLOEXEC);
        if (rp->fd < 0) {
                loge("ubuntu_rapidio_init open %s error, error info:%s(%d)\n", dev_name, strerror(errno), errno);//string.h
                return RET_ERROR;
        }
	ret = ioctl(rp->fd, RIO_ID_SET_INIT, &(rp->id_info));
	if (ret < 0) {
		close(rp->fd);
		loge("ubuntu_rapidio_init fail, ioctl RIO_ID_SET_INIT fail, ret.%d.\n", ret);
		return ret;
	}
	logi("ubuntu_rapidio_init device_id.%d\n", rp->id_info.device_id);
	ret = ioctl(rp->fd, RIO_DME_INIT, &(rp->id_info.device_id));
        if (ret < 0) {
                close(rp->fd);
                loge("ubuntu_rapidio_init fail, ioctl RIO_DME_INIT fail, ret.%d.\n", ret);
                return ret;
        }
	ret = ioctl(rp->fd, RIO_DMA_INIT, &(rp->dma_intr_mode));
        if (ret < 0) {
                close(rp->fd);
                loge("ubuntu_rapidio_init fail, ioctl RIO_DMA_INIT fail, ret.%d.\n", ret);
                return ret;
        }
	ret = ioctl(rp->fd, RIO_DOORBELL_INIT);
        if (ret < 0) {
                close(rp->fd);
                loge("ubuntu_rapidio_init fail, ioctl RIO_DOORBELL_INIT fail, ret.%d.\n", ret);
                return ret;
        }
	signal(SIGIO , sigint_handler);
	ret = fcntl(rp->fd, F_SETOWN, getpid());
	if (ret < 0) {
		close(rp->fd);
		loge("ubuntu_rapidio_init fail, fcntl F_SETOWN fail, ret.%d.\n", ret);
		return ret;
	}
	flags = fcntl(rp->fd, F_GETFL);
        if (ret < 0) {
                close(rp->fd);
                loge("ubuntu_rapidio_init fail, fcntl F_GETFL fail, ret.%d.\n", ret);
                return ret;
        }
	fcntl(rp->fd, F_SETFL, flags | FASYNC);
        if (ret < 0) {
                close(rp->fd);
                loge("ubuntu_rapidio_init fail, fcntl F_SETFL fail, ret.%d.\n", ret);
                return ret;
        }
	logi("ubuntu_rapidio_init ok, fd.%d flags.%d getpid.%d\n", rp->fd, flags, getpid());
	return ret;
}

void ubuntu_rapidio_deinit(rapidio_parameter* rp)
{
	close(rp->fd);
	return;
}

void *ubuntu_rapidio_pthread_handle(void * arg)
{
	int ret = RET_OK;

	rp.file_path = "/dev/rio_mport";
	rp.dev_name = "0";
	rp.id_info.device_id = GRIO_LOC_HOST_ID;
	rp.id_info.device_id_width = 8;
	rp.rio_tran_db.rioid = GRIO_RM_HOST_ID;
	rp.dma_intr_mode = 0;

	ret = ubuntu_rapidio_init(&rp);
	if (ret < 0) {
		close(rp.fd);
		loge("ubuntu_rapidio_pthread_handle fail, ret.%d.\n", ret);
		return NULL;
	}
	logi("ubuntu_rapidio_pthread_handle rp.fd.%d\n", rp.fd);
	while (1) 
	{
		test_rapidio_send(&rp);
//		ubuntu_doorbell_send(GRIO_RM_HOST_ID, 0x55, &rp);		
//		ubuntu_doorbell_send(GRIO_LOC_HOST_ID, 0x55, &rp);
		logi("ubuntu_rapidio_pthread_handle, rapio send\n");
		logi("ubuntu_rapidio_pthread_handle rp.fd.%d\n", rp.fd);
		while(doorbell_arrival_flag == 0)
		{
		}
		doorbell_arrival_flag = 0;
		logi("ubuntu_rapidio_pthread_handle, rapio done\n");
//		ret = neural_network_results_rapidio_send(&rp);
		if (ret < 0) {
			loge("neural_network_results_rapidio_send fail\n");
		}
	}
	ubuntu_rapidio_deinit(&rp);
}

void test_ubuntu_rapidio(void)
{
        int ret = 0;
        pthread_t rapidio_p;

	ret = pthread_create((pthread_t *)&rapidio_p, NULL, ubuntu_rapidio_pthread_handle, NULL);
        if (ret < 0) {
                loge("test_ubuntu_rapidio pthread_create fail, ret.%x\n",ret);
                return;
        }
        logi("test rapidio start ...\n");
        return;
}

#endif
