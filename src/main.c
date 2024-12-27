#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "log/log.h"
#include "main.h"
#include "refactor/refactor.h"
#include "queue/queue.h"
#include "list/list.h"
#include "comm/comm.h"
#include "memory/share_memory.h"
#include "semaphore/semaphore.h"
#include "comm/can/can.h"
#if OS_UBUNTU
#include <termios.h>
#include "comm/can/ubuntu/can_ubuntu.h"
#include "comm/rs422/ubuntu/ubuntu_rs422.h"
#include "comm/rapidio/ubuntu/ubuntu_rapidio.h"
#include "driver/i2c/ubuntu_i2c.h"
#include "device/cdcm6208/cdcm6208.h"
#endif

#define EXIT_SUCCESS  0
#define EXIT_FAILURE  1

pthread_t can_p;
pthread_t rs422_p;
pthread_t rapidio_p;

can_parameter cp;
rs422_parameter rp;
i2c_parameter ip;

volatile bool refa_flag;
volatile bool comm_flag;
volatile bool image_recv_done;
volatile bool message_come_flag;
void signal_handler(int signal);

struct pthreadcreatparameter pcr[] = {
	{(pthread_t *)&can_p, NULL, ubuntu_can_pthread_handle, &cp},
	{(pthread_t *)&rs422_p, NULL, ubuntu_rs422_pthread_handle, &rp},
	{(pthread_t *)&rapidio_p, NULL, ubuntu_rapidio_pthread_handle, NULL},
};

void signal_handler(int signal) {
        if(signal == SIGINT) {
                logi("Ctrl + C press, thread will exit\n");
                refa_flag = false;
                comm_flag = false;
                exit(EXIT_SUCCESS);
        }
}

int main() 
{
    int ret = 0;
    int i = 0;
    refa_flag = true;
    comm_flag = true;
    image_recv_done = false;

 
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
	    logi("can not cat SIGINT signal\n");
	    exit(EXIT_FAILURE);
    }
    /* i2c set */
    memcpy(ip.dev, "/dev/i2c-1", I2C_DEV_LENGTH);
    ip.slave_addr = I2C_SLAVE_6208_ADDR;
    ip.slave_addr_bits = 7; 
    /* can set */
    cp.dev_name = "can0";
    cp.speed = 100000;
    /* rs422 set */
    rp.file_path = "/dev/tty";
    rp.dev_name = "S2";
    rp.baudrate = 115200;
    rp.databits = CS8;
    rp.stopbits = TWO_STOP_BIT;
    rp.parity = 'e';
    /* rs422 init */
    ubuntu_rs422_init(&rp);
    ubuntu_rs422_set_speed(&rp);
    ubuntu_rs422_set_parameter(&rp);
    /* i2c init */
    ubuntu_i2c_init(&ip, ip.dev, ip.slave_addr, ip.slave_addr_bits);
    /* clk config */
    chip6208_set_clk_for_board(&ip, &rp);
    /* create pthread */
    for (i = 0; i < sizeof(pcr)/sizeof(struct pthreadcreatparameter); i++)
    {
	    ret = pthread_create(pcr[i].tidp, pcr[i].attr, pcr[i].start_rtn, pcr[i].arg);
	    if (ret != 0) {
	        loge("creat thread fail return ret.%d\n", ret);
		return ret;
	    }
	    logi("creat pthread %d\n",i);
    }
    pthread_exit(NULL);
    return 0;
}

