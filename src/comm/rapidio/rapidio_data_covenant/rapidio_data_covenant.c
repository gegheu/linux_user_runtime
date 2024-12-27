#include <stdio.h>
#include <string.h>
#include "log/log.h"
#include "queue/queue.h"
#include "memory/share_memory.h"
#include "comm/rapidio/ubuntu/ubuntu_rapidio.h"
#include "comm/rapidio/rapidio_data_covenant/rapidio_data_covenant.h"

unsigned char slice_img_buff[IMGE_LENGTH][IMGE_WIDTH] = { 0x0 };
unsigned int result_buff[RESULT_BUFFER_LENGTH];
struct rapidio_results_frame_info rrfi[RAPIDIO_RESULTS_FRAME_NUM];


int image_data_save_file(char *file_name, unsigned int file_size, unsigned char *data)
{
	FILE *fp = fopen(file_name, "abw");
	if (fp == NULL) {
		loge("open file error\n");
		return RET_ERROR;
	} else {
		fwrite(data, 1, file_size, fp);
		fflush(fp);
		fclose(fp);
	}
	return 0;
}

int rapidio_image_frame_info_obtain(struct rapidio_image_frame * rif)
{
	virtaddr_parameter vp;
	int ret = 0;

        if (pointer_check(rif)) {
                loge("rapidio_image_frame_info_obtain fail, rif is null, please check rif\n");
                return RET_ERROR;
	}
        vp.map_size = MAP_SIZE;	
	ret = ubuntu_inbound_addr_read((unsigned char *)rif, sizeof(struct rapidio_image_frame), &vp, RAPIDIO_FRAME_DATA_PHY_ADDR);
	if (ret < 0) {
		loge("rapidio_image_frame_info_obtain fail, ubuntu_inbound_addr_read fail\n");
		return RET_ERROR;
	}
	return 0; 
}

int rapidio_image_frame_save_file(char *file_name, unsigned int file_size)
{
	virtaddr_parameter vp;
        int ret = 0;

        if (pointer_check(file_name)) {
                loge("rapidio_image_frame_save_file fail, file_name is null, please check file_name\n");
                return RET_ERROR;
        }
        vp.map_size = MAP_SIZE; 
        ret = ubuntu_inbound_addr_read((unsigned char *)&(slice_img_buff[0][0]), sizeof(struct rapidio_image_frame), &vp, RAPIDIO_VALID_DATA_PHY_ADDR);
        if (ret < 0) {
                loge("rapidio_image_frame_save_file fail, ubuntu_inbound_addr_read fail\n");
                return RET_ERROR;
        }
	ret = image_data_save_file(file_name, file_size, (unsigned char *)&(slice_img_buff[0][0]));
        if (ret < 0) {
                loge("rapidio_image_frame_save_file fail, image_data_save_file fail\n");
                return RET_ERROR;
        }
	return 0;
}

int reasoning_done_result_num_get(unsigned int* result_num)
{
        virtaddr_parameter vp;
        int ret = 0;

        if (pointer_check(result_num)) {
                loge("reasoning_done_result_num_get fail, result_num is null, please check result_num\n");
                return RET_ERROR;
        }
	vp.map_size = MAP_SIZE;
	ret = ubuntu_inbound_addr_read((unsigned char *)result_num, sizeof(int), &vp, RAPIDIO_VALID_DATA_PHY_ADDR);
	if (ret < 0) {
                loge("reasoning_done_result_num_get fail, ubuntu_inbound_addr_read fail\n");
                return RET_ERROR;
	}
	return 0;
}

int results_convert_rapidio_frame(unsigned char* data, unsigned int result_num, struct rapidio_image_frame * rif)
{
	int frame_cnt, frame_data_cnt, i;
	unsigned int frame_num = result_num / NEURAL_NETWORK_RESULTS_NUM;
	unsigned int frame_data_num = result_num % NEURAL_NETWORK_RESULTS_NUM;
	unsigned int real_data_size;

        if (pointer_check(data)) {
                loge("results_convert_rapidio_frame fail, data is null, please check data\n");
                return RET_ERROR;
        }
	if ((frame_num == 0) && (frame_data_num == 0)) {
		loge("results_convert_rapidio_frame fail, frame_num = 0 && frame_data_num = 0\n");
		return RET_ERROR;
	}
	if (frame_data_num > 0) {
		frame_num = frame_num + 1;
	}
	real_data_size = sizeof(struct annotation_box_coordinate) * REAL_RESULTS_NUM;
	for (frame_cnt = 0; frame_cnt < frame_num; frame_cnt++) {
		rrfi[frame_cnt].sync_type = FRAME_SYNC_TYPE;
		rrfi[frame_cnt].frame_type = rif->frame_type;
		rrfi[frame_cnt].frame_cnt = i;
		if ((i == frame_num - 1) && frame_data_num > 0) {
			rrfi[frame_cnt].frame_len = real_data_size * frame_data_num;
		} else {
			rrfi[frame_cnt].frame_len = real_data_size * NEURAL_NETWORK_RESULTS_NUM;
		}
		rrfi[frame_cnt].image_name = rif->image_name;
		rrfi[frame_cnt].slice_cnt = rif->slice_cnt;
		rrfi[frame_cnt].row_coordinate = rif->row_coordinate;
		rrfi[frame_cnt].col_coordinate = rif->col_coordinate;
		rrfi[frame_cnt].result_num = result_num;
		for (i = 0; i < FILL_REGION_NUM; i++) {
			rrfi[frame_cnt].fill_region[i] = REGION_FILL_NUM;
		}
		if ((frame_cnt == (frame_num - 1)) && frame_data_num > 0) {
			for (frame_data_cnt = 0; frame_data_cnt < frame_data_num; frame_data_cnt++) {
				rrfi[frame_cnt].nnri[frame_data_cnt].results_name = i;//todo real name 
				rrfi[frame_cnt].nnri[frame_data_cnt].confidence_level = i;//todo real level
				rrfi[frame_cnt].nnri[frame_data_cnt].annotation_box_type = i;//todo real box type
				memcpy(&(rrfi[frame_cnt].nnri[frame_data_cnt].abc[0]), &(data[frame_cnt * real_data_size * NEURAL_NETWORK_RESULTS_NUM + frame_data_cnt * real_data_size]), real_data_size);
				rrfi[frame_cnt].nnri[frame_data_cnt].box_length = 0x55; //todo real box length
				rrfi[frame_cnt].nnri[frame_data_cnt].box_width = 0xAA;  //todo real box width
				rrfi[frame_cnt].nnri[frame_data_cnt].box_area = 0xCCDD; //todo real box area
				rrfi[frame_cnt].nnri[frame_data_cnt].box_angle = 0xEE;  //todo real box angle
			}
			for (frame_data_cnt = frame_data_num; frame_data_cnt < FILL_REGION_NUM; frame_data_cnt++) {
                                rrfi[frame_cnt].nnri[frame_data_cnt].results_name = i;//todo real name 
                                rrfi[frame_cnt].nnri[frame_data_cnt].confidence_level = i;//todo real level
                                rrfi[frame_cnt].nnri[frame_data_cnt].annotation_box_type = i;//todo real box type
				memset(&(rrfi[frame_cnt].nnri[frame_data_cnt].abc[0]), 0, real_data_size);
                                rrfi[frame_cnt].nnri[frame_data_cnt].box_length = 0x0; //todo real box length
                                rrfi[frame_cnt].nnri[frame_data_cnt].box_width = 0x0;  //todo real box width
                                rrfi[frame_cnt].nnri[frame_data_cnt].box_area = 0x0; //todo real box area
                                rrfi[frame_cnt].nnri[frame_data_cnt].box_angle = 0x0;  //todo real box angle
			}
		} else {
			for (frame_data_cnt = 0; frame_data_cnt < NEURAL_NETWORK_RESULTS_NUM; frame_data_cnt++) {
				rrfi[frame_cnt].nnri[frame_data_cnt].results_name = i;//todo real name 
                                rrfi[frame_cnt].nnri[frame_data_cnt].confidence_level = i;//todo real level
                                rrfi[frame_cnt].nnri[frame_data_cnt].annotation_box_type = i;//todo real box type
                                memcpy(&(rrfi[frame_cnt].nnri[frame_data_cnt].abc[0]), &(data[frame_cnt * real_data_size * NEURAL_NETWORK_RESULTS_NUM + frame_data_cnt * real_data_size]), real_data_size);
                                rrfi[frame_cnt].nnri[frame_data_cnt].box_length = 0x55; //todo real box length
                                rrfi[frame_cnt].nnri[frame_data_cnt].box_width = 0xAA;  //todo real box width
                                rrfi[frame_cnt].nnri[frame_data_cnt].box_area = 0xCCDD; //todo real box area
                                rrfi[frame_cnt].nnri[frame_data_cnt].box_angle = 0xEE;  //todo real box angle
			}
		}
	}
	return frame_num;
}

int rapidio_results_frame_fill(unsigned int result_num, struct rapidio_image_frame * rif, unsigned int* result_frame_num)
{
        virtaddr_parameter vp;
        int ret = 0;

        if (pointer_check(rif) || pointer_check(result_frame_num)) {
                loge("rapidio_results_frame_fill fail, rif or result_frame_num is null, please check rif or result_frame_num\n");
                return RET_ERROR;
        }
	vp.map_size = MAP_SIZE;
        ret = ubuntu_inbound_addr_read((unsigned char *)result_buff, sizeof(struct annotation_box_coordinate) * REAL_RESULTS_NUM * result_num, &vp, RAPIDIO_SEND_INFO_START_ADDR);
        if (ret < 0) {
                loge("rapidio_results_frame_fill fail, ubuntu_inbound_addr_read fail\n");
                return RET_ERROR;
        }
	*result_frame_num = results_convert_rapidio_frame((unsigned char*)result_buff, result_num, rif);
	if (*result_frame_num < 0) {
                loge("rapidio_results_frame_fill fail, results_convert_rapidio_frame fail\n");
                return RET_ERROR;
	}
	return 0;
}

int neural_network_results_rapidio_send(rapidio_parameter* rp)
{
	struct rapidio_image_frame rif;
	virtaddr_parameter vp;
	unsigned int result_num = 0;
	unsigned int result_frame_num = 0;
	int ret = 0;

	ret = rapidio_image_frame_info_obtain(&rif);
	if (ret < 0) {
                loge("neural_network_results_rapidio_send fail, rapidio_image_frame_info_obtain fail\n");
                return RET_ERROR;
	}
	ret = rapidio_image_frame_save_file("neural_network_image.raw", 640 * 640); //todo real raw imge and size
        if (ret < 0) {
                loge("neural_network_results_rapidio_send fail, rapidio_image_frame_save_file fail\n");
                return RET_ERROR;
        }
	//system() todo start neural_network
	//todo check neural_network done
	ret = reasoning_done_result_num_get(&result_num);
        if (ret < 0) {
                loge("neural_network_results_rapidio_send fail, reasoning_done_result_num_get fail\n");
                return RET_ERROR;
        }
	ret = rapidio_results_frame_fill(result_num, &rif, &result_frame_num);
        if (ret < 0) {
                loge("neural_network_results_rapidio_send fail, rapidio_results_frame_fill fail\n");
                return RET_ERROR;
        }
	vp.map_size = MAP_SIZE;

        rp->rio_xfer.rioid = GRIO_RM_HOST_ID;
        rp->rio_xfer.rio_addr = RAPIDIO_FRAME_DATA_PHY_ADDR;
        rp->rio_xfer.loc_addr = RAPIDIO_FRAME_DATA_PHY_ADDR;
        rp->rio_xfer.length = result_frame_num * sizeof(struct rapidio_results_frame_info);

        rp->rio_tran.dir = RIO_TRANSFER_DIR_WRITE;
        rp->rio_tran.dma_intr_mode = 0;
        rp->rio_tran.count = 1;
        rp->rio_tran.block = (unsigned int)&(rp->rio_xfer);
        rp->rio_tran.check_flag = 1;

	ret = ubuntu_rapidio_dma_desddr(rp, (unsigned char *)&rrfi[0], &vp, RAPIDIO_FRAME_DATA_PHY_ADDR);
        if (ret < 0) {
                loge("neural_network_results_rapidio_send fail, ubuntu_rapidio_dma_desddr fail\n");
                return RET_ERROR;
        }
	ret = ubuntu_doorbell_send(GRIO_RM_HOST_ID, 0x55, rp);
        if (ret < 0) {
                loge("neural_network_results_rapidio_send fail, ubuntu_doorbell_send fail\n");
                return RET_ERROR;
        }
	memset(&rrfi[0], 0, sizeof(struct rapidio_results_frame_info) * RAPIDIO_RESULTS_FRAME_NUM);
	return 0;
}

unsigned char test_buff[1024] = { 0 };
int test_rapidio_send(rapidio_parameter* rp)
{
	virtaddr_parameter vp;
	int cnt, ret;

	for(cnt = 0; cnt < 1024; cnt++) {
		test_buff[cnt] = cnt;
	}
	vp.map_size = MAP_SIZE;
        rp->rio_xfer.rioid = GRIO_RM_HOST_ID;
        rp->rio_xfer.rio_addr = RAPIDIO_FRAME_DATA_PHY_ADDR;
        rp->rio_xfer.loc_addr = RAPIDIO_FRAME_DATA_PHY_ADDR;
        rp->rio_xfer.length = sizeof(test_buff);

        rp->rio_tran.dir = RIO_TRANSFER_DIR_WRITE;
        rp->rio_tran.dma_intr_mode = 0;
        rp->rio_tran.count = 1;
        rp->rio_tran.block = (unsigned int)&(rp->rio_xfer);
        rp->rio_tran.check_flag = 1;

        ret = ubuntu_rapidio_dma_desddr(rp, (unsigned char *)test_buff, &vp, RAPIDIO_FRAME_DATA_PHY_ADDR);
        if (ret < 0) {
                loge("test_rapidio_send fail, ubuntu_rapidio_dma_desddr fail\n");
                return RET_ERROR;
        }
#if 0	
        ret = ubuntu_doorbell_send(GRIO_RM_HOST_ID, 0x55, rp);
        if (ret < 0) {
                loge("test_rapidio_send fail, ubuntu_doorbell_send fail\n");
                return RET_ERROR;
        }
#endif
        memset(test_buff, 0, sizeof(test_buff));
	return 0;

}
