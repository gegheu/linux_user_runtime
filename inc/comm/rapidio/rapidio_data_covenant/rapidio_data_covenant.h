#ifndef __RAPIDIO_DATA_COVENANT__
#define __RAPIDIO_DATA_COVENANT__

#include <stdio.h>
#include <sys/types.h>
#include "comm/rapidio/ubuntu/ubuntu_rapidio.h"

#define RAPIDIO_FRAME_DATA_PHY_ADDR  0x30000000
#define RAPIDIO_VALID_DATA_PHY_ADDR  0x30000010
#define RAPIDIO_SEND_INFO_START_ADDR 0x30000014
#define IMGE_LENGTH   5056
#define IMGE_WIDTH    5056
#define RESULT_BUFFER_LENGTH    1024*1024
#define ANNOTATION_BOX_COORDINATE_NUM   5
#define FILL_REGION_NUM    18
#define NEURAL_NETWORK_RESULTS_NUM   6
#define REAL_RESULTS_NUM   4
#define RAPIDIO_RESULTS_FRAME_NUM  100
#define FRAME_SYNC_TYPE     0xEB90
#define REGION_FILL_NUM     0xAA

#pragma pack(push, 1)
struct rapidio_image_frame {
        unsigned short frame_type;
        unsigned int image_name;
        unsigned int slice_cnt;
        unsigned short row_coordinate;
        unsigned short col_coordinate;
        unsigned short slice_size;
};
#pragma pack(pop)

struct annotation_box_coordinate {
        unsigned int x;
        unsigned int y;
};

struct neural_network_results_info {
        unsigned char results_name;
        unsigned short confidence_level;
        unsigned short annotation_box_type;
        struct annotation_box_coordinate abc[ANNOTATION_BOX_COORDINATE_NUM];
        unsigned short box_length;
        unsigned short box_width;
        unsigned short box_area;
        unsigned short box_angle;
};

struct rapidio_results_frame_info {
        unsigned short sync_type;
        unsigned short frame_type;
        unsigned int frame_cnt;
        unsigned int frame_len;
        unsigned int rsv;
        unsigned int image_name;
        unsigned int slice_cnt;
        unsigned short row_coordinate;
        unsigned short col_coordinate;
        unsigned short result_num;
        unsigned char fill_region[FILL_REGION_NUM];
        struct neural_network_results_info nnri[NEURAL_NETWORK_RESULTS_NUM];
};

int neural_network_results_rapidio_send(rapidio_parameter* rp);
int test_rapidio_send(rapidio_parameter* rp);
#endif
