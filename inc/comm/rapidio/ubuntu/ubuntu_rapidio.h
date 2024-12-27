#ifndef __UBUNTU_RAPIDIO__
#define __UBUNTU_RAPIDIO__

#include <stdio.h>
#include <sys/types.h>
#include "memory/memory.h"
#if OS_UBUNTU

#define RAPIDIO_DEV_PATH        "/dev"
#define RAPIDIO_DEV_NAME        "rio_mport0"
#define RAPIDIO_DEV_LENGTH      64
#define DOORBELL_BUFFER_SIZE    32 * 2
#define DOOLBELL_PENDING        5
#define DOORBELL_OK             1
#define GRIO_LOC_HOST_ID        (0x00)//local rio dev ID
#define GRIO_RM_HOST_ID         (0x01)//remote rio dev ID

struct rio_transfer_io {
        unsigned long long rio_addr; /* Address in target's RIO mem space */
        unsigned long long loc_addr;
        unsigned long long handle;
        unsigned long long offset;   /* Offset in buffer */
        unsigned long long length;   /* Length in bytes */
        unsigned short     rioid;    /* Target destID */
        unsigned short     method;   /* Data exchange method, one of rio_exchange enum */
        unsigned int       completion_code;  /* Completion code for this transfer */
};

struct rio_transaction {
        unsigned long long block;  /* Pointer to array of <count> transfers */
        unsigned int       count;  /* Number of transfers */
        unsigned int       transfer_mode;   /* Data transfer mode */
        unsigned int       dma_intr_mode;   /* 1 dma interrupt check mode */
        unsigned short     sync;  /* Synch method, one of rio_transfer_sync enum */
        unsigned short     dir;   /* Transfer direction, one of rio_transfer_dir enum */
        unsigned int       pad0;
        unsigned char      check_flag;
};

struct rio_transfer_doorbell {
        unsigned short rioid;   /* Target destID */
        unsigned short data_p;
        unsigned int ob_db_priority;
        unsigned int ob_db_crf;
        unsigned int ob_db_vc;
        unsigned int db_num;
};

struct rio_transaction_doorbell_receive {
        unsigned char receive_flag;
        unsigned char doorbell_num;
        unsigned short src_id;
        unsigned short data;
};

struct rio_transaction_doorbell {
        unsigned int block;    /* Pointer to array of <count> transfers */
        unsigned int count;    /* Number of transfers */
};

struct id_set_info{
        unsigned short device_id;
        unsigned int device_id_width;
};

typedef struct rapidio_parameter {
        const char* file_path;
        const char* dev_name;
        int fd;
        struct id_set_info id_info;
        unsigned int dma_intr_mode;
        struct rio_transfer_io rio_xfer;
        struct rio_transaction rio_tran;
        struct rio_transfer_doorbell rio_tran_db;
} rapidio_parameter;

enum PACKAGE_TYPE {
        DOORBELL_TYPE = 1,
        DME_TYPE,
        DOORBELL_DME_TYPE,
        MAX_TYPE,
};

enum DMA_STATE {
        DMA_WRITE_OK = 1,
        FETCH_ERROR,
        UPDATA_ERROR,
        AXI_FETCH_ERROR,
        DMA_ALL_PASS = 6,
};

enum rio_transfer_dir {
        RIO_TRANSFER_DIR_READ,  /* Read operation */
        RIO_TRANSFER_DIR_WRITE, /* Write operation */
};

int ubuntu_inbound_addr_read(unsigned char* recv_buf, unsigned int len, virtaddr_parameter* vp, unsigned int phyaddr);
int ubuntu_doorbell_send(unsigned short int dest_id,unsigned char info, rapidio_parameter* rp);
int ubuntu_rapidio_dma_desddr(rapidio_parameter* rp,  unsigned char *inout_buf,  virtaddr_parameter* vp, unsigned int phyaddr);
void *ubuntu_rapidio_pthread_handle(void * arg);
void test_ubuntu_rapidio(void);
#endif
#endif
