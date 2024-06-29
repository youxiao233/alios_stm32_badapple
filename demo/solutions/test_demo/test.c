#include "aos/init.h"
#include "board.h"
#include <aos/errno.h>
#include <aos/kernel.h>
#include <k_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <aos/hal/pwm.h>
#include <aos/hal/gpio.h>
#include <hal_iomux_haas1000.h>
#include <sh1106.h>

// 假设每张图像都是288字节
const unsigned char gImage_frame_0001[288] = { /* 图像数据 */ };
const unsigned char gImage_frame_0002[288] = { /* 图像数据 */ };
// ...包含所有其他图像的声明...
const unsigned char gImage_frame_0016[288] = { /* 图像数据 */ };

// 将所有图像的指针存储在数组中
const unsigned char* image_frames[16] = {
    gImage_frame_0001, gImage_frame_0002, gImage_frame_0003, gImage_frame_0004,
    gImage_frame_0005, gImage_frame_0006, gImage_frame_0007, gImage_frame_0008,
    gImage_frame_0009, gImage_frame_0010, gImage_frame_0011, gImage_frame_0012,
    gImage_frame_0013, gImage_frame_0014, gImage_frame_0015, gImage_frame_0016
};

int application_start(int argc, char *argv[])
{
    sh1106_init();
    OLED_Clear();
    int frame_index = 0; // 用于追踪当前帧的索引

    while(1)
    {
        // 显示当前帧
        OLED_Frame_Show_Pic(image_frames[frame_index], 0, 0, 80, 8);
        OLED_Refresh_GRAM();
        aos_msleep(500); // 设置每帧的持续时间

        // 更新帧索引，实现循环播放
        frame_index = (frame_index + 1) % 16;
    }
}