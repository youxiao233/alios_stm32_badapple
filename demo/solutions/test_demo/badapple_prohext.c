/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */

#include "aos/init.h"
#include "board.h"
#include <aos/errno.h>
#include <aos/kernel.h>
#include <k_api.h>
#include <stdio.h>
#include <stdlib.h>
//添加头文件
#include <aos/hal/gpio.h>
#include <hal_iomux_haas1000.h>
#include <aos/hal/pwm.h>
#include "define.h"

gpio_dev_t led[3] = {0};
gpio_dev_t key[4] = {0};
pwm_dev_t beep = {0};
//初始化按键

void beep_init(int HZ)
{
   beep.port = BEEP_PORT;
   beep.config.duty_cycle = 0.5;
   //频率范围 20-20000Hz
   beep.config.freq =HZ;
   hal_pwm_init(&beep);
}

void key4_callback();

void all_key_init()
{
    //OUTPUT_PUSH_PULL     IRQ_MODE
    key[0].port = HAL_IOMUX_PIN_P2_7;
    key[0].config = OUTPUT_PUSH_PULL;
    key[0].priv = NULL;
    hal_gpio_init(&key[0]);    

    key[1].port = HAL_IOMUX_PIN_P2_4;
    key[1].config = OUTPUT_PUSH_PULL;
    key[1].priv = NULL;
    hal_gpio_init(&key[1]);    

    key[2].port = HAL_IOMUX_PIN_P2_5;
    key[2].config = OUTPUT_PUSH_PULL;
    key[2].priv = NULL;
    hal_gpio_init(&key[2]);

    key[3].port = HAL_IOMUX_PIN_P3_2;
    key[3].config = IRQ_MODE;
    key[3].priv = NULL;
    hal_gpio_init(&key[3]);    
    hal_gpio_enable_irq(&key[3],IRQ_TRIGGER_RISING_EDGE,key4_callback,(void *)0);
}
//初始化led
void all_led_init(){
    led[2].port = HAL_GPIO_PIN_P4_2;
    led[1].port = HAL_GPIO_PIN_P4_3;
    led[0].port = HAL_GPIO_PIN_P4_4;
    //推挽输出，在高电平和低电平都有较强的输出能力
    for(int i=0;i<2;i++)led[i].config = OUTPUT_PUSH_PULL;
    //私有数据没使用
    for(int i=0;i<=2;i++)led[i].priv = NULL;
    //蓝灯初始化
    for(int i=0;i<=2;i++)hal_gpio_init(&led[i]);
}
//led随机闪烁
void randled(){
    int a=rand()%3;
    hal_gpio_output_high(&led[a]);
    //毫秒延时
    aos_msleep(200);

    hal_gpio_output_low(&led[a]);

    aos_msleep(200);
}
//流水灯
void flow_led(){
    for(int i=0;i<3;i++){
            //亮1s，灭1s
        hal_gpio_output_high(&led[i]);
        //毫秒延时
        aos_msleep(200);

        hal_gpio_output_low(&led[i]);

        aos_msleep(200);
    }
}

//---------------------------------------------------
/*
：
按键    K1      K2      K3      K4
led灯   蓝：0   红：1   绿：2

*/
int ledmod=0;
//led闪烁
void blink(int pos){
    hal_gpio_output_high(&led[pos]);
    //毫秒延时
    aos_msleep(200);

    hal_gpio_output_low(&led[pos]);

    aos_msleep(200);
}
//按键中断实现切换功能
int key_scan(int pos){
    uint32_t val=1;
    hal_gpio_input_get(&key[pos],&val);
    if(val == 0)
    {
        aos_msleep(10);
        hal_gpio_input_get(&key[pos],&val);
        if(val == 0)
        {
            while(val == 0)
            {
                hal_gpio_input_get(&key[pos],&val);
            }
            return 1;
            printf("key down\n");
        }
        else return 0;
    }
    else return 0;
}

int flag=0;
//中断函数
void key4_callback(){
    /*
    0 按键灯  1流水灯  2数据采集及上传  3音乐  4视频播放1 5视频播放2
    */
    flag=0;
    if(!ledmod)
        for(int i=0;i<=2;i++)
            hal_gpio_output_low(&led[i]);
    ledmod++;

    ledmod%=6;
     
}
//蜂鸣器使能
void tone(uint16_t frequency, uint16_t duration)
{
    pwm_dev_t pwm = {0, {0.8, frequency}, NULL};        // 设定pwm 频率为设定频率
    if (frequency > 0)                                                                // 频率值合法才会初始化pwm
    {
        hal_pwm_init(&pwm);
        hal_pwm_start(&pwm);
    }
    if (duration != 0)                                        
    {
        aos_msleep(duration);
    }
    if (frequency > 0 && duration > 0)                // 如果设定了 duration，则在该延时后停止播放
    {
        hal_pwm_stop(&pwm);
        hal_pwm_finalize(&pwm);
    }
}
void noTone()
{
    pwm_dev_t pwm = {0, {0.8, 1}, NULL};        // 关闭对应端口的pwm输出
    hal_pwm_stop(&pwm);
    hal_pwm_finalize(&pwm);
}

//音频播放
void play_melody()
{
    uint32_t bpm = 137;
    // 12000 = 60 * 1000 * 4 * 0.8 / 16 quarter note = one beat
    uint32_t ndms = 12000;
    // notes in the melody:
    // 1 = #F to 1 = G 4/4
    uint32_t melody[] = {
        // Intro
        NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2,
        NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2,
        NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2,
        NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2,
        NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2,
        NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2,
        NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2,
        NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_DS2,
        NOTE_DS2, 0, NOTE_DS2, NOTE_CS2, NOTE_DS2, NOTE_DS2, 0, NOTE_DS2, NOTE_CS2, NOTE_DS2,
        NOTE_DS2, 0, NOTE_DS2, NOTE_CS2, NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_FS2, NOTE_GS2, NOTE_FS2, NOTE_GS2,
        NOTE_DS2, 0, NOTE_DS2, NOTE_CS2, NOTE_DS2, NOTE_DS2, 0, NOTE_DS2, NOTE_CS2, NOTE_DS2,
        NOTE_DS2, 0, NOTE_DS2, NOTE_CS2, NOTE_DS2, NOTE_GS2, NOTE_FS2, NOTE_GS2, NOTE_FS2, NOTE_DS2, NOTE_FS2,
        NOTE_DS2, 0, NOTE_DS2, NOTE_CS2, NOTE_DS2, NOTE_DS2, 0, NOTE_DS2, NOTE_CS2, NOTE_DS2,
        NOTE_DS2, 0, NOTE_DS2, NOTE_CS2, NOTE_DS2, NOTE_DS2, NOTE_DS2, NOTE_FS2, NOTE_GS2, NOTE_FS2, NOTE_GS2,
        NOTE_DS2, 0, NOTE_DS2, NOTE_CS2, NOTE_DS2, NOTE_DS2, 0, NOTE_DS2, NOTE_CS2, NOTE_DS2,
        NOTE_DS2, 0, NOTE_DS2, NOTE_CS2, NOTE_DS2, NOTE_GS2, NOTE_FS2, NOTE_GS2, NOTE_FS2, NOTE_DS2, NOTE_FS2,
        // Verse 1 - 16
        NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_DS5, NOTE_CS5,
        NOTE_AS4, NOTE_DS4, NOTE_AS4, NOTE_GS4, NOTE_FS4, NOTE_F4,
        NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_FS4,
        NOTE_F4, NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_F4, NOTE_DS4, NOTE_D4, NOTE_F4,
        NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_DS5, NOTE_CS5,
        NOTE_AS4, NOTE_DS4, NOTE_AS4, NOTE_GS4, NOTE_FS4, NOTE_F4,
        NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_FS4,
        NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4,
        NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_DS5, NOTE_CS5,
        NOTE_AS4, NOTE_DS4, NOTE_AS4, NOTE_GS4, NOTE_FS4, NOTE_F4,
        NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_FS4,
        NOTE_F4, NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_F4, NOTE_DS4, NOTE_D4, NOTE_F4,
        NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_DS5, NOTE_CS5,
        NOTE_AS4, NOTE_DS4, NOTE_AS4, NOTE_GS4, NOTE_FS4, NOTE_F4,
        NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_FS4,
        NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4,
        // Verse 17 - 32
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_GS4, NOTE_FS4, NOTE_F4, NOTE_CS4, NOTE_DS4, NOTE_CS4, NOTE_DS4,
        NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_DS4, NOTE_AS4, NOTE_CS5,
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_GS4, NOTE_FS4, NOTE_F4, NOTE_CS4, NOTE_DS4, NOTE_CS4, NOTE_DS4,
        NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_DS4, NOTE_AS4, NOTE_CS5,
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_GS4, NOTE_FS4, NOTE_F4, NOTE_CS4, NOTE_DS4, NOTE_CS4, NOTE_DS4,
        NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_DS4, NOTE_AS4, NOTE_CS5,
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_DS5, NOTE_F5,
        NOTE_FS5, NOTE_F5, NOTE_DS5, NOTE_CS5, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_GS4, NOTE_FS4, NOTE_F4, NOTE_CS4, NOTE_DS4, NOTE_AS4, NOTE_CS5,
        // Verse 33 - 48
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_GS4, NOTE_FS4, NOTE_F4, NOTE_CS4, NOTE_DS4, NOTE_CS4, NOTE_DS4,
        NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_DS4, NOTE_AS4, NOTE_CS5,
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_GS4, NOTE_FS4, NOTE_F4, NOTE_CS4, NOTE_DS4, NOTE_CS4, NOTE_DS4,
        NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_DS4, NOTE_AS4, NOTE_CS5,
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_GS4, NOTE_FS4, NOTE_F4, NOTE_CS4, NOTE_DS4, NOTE_CS4, NOTE_DS4,
        NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_DS4, NOTE_AS4, NOTE_CS5,
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_DS5, NOTE_F5,
        NOTE_FS5, NOTE_F5, NOTE_DS5, NOTE_CS5, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_GS4, NOTE_FS4, NOTE_F4, NOTE_CS4, NOTE_DS4, 0,
        // Interlude
        NOTE_DS3, 0, NOTE_DS3, NOTE_CS3, NOTE_DS3, NOTE_DS3, 0, NOTE_DS3, NOTE_CS3, NOTE_DS3,
        NOTE_DS3, 0, NOTE_DS3, NOTE_CS3, NOTE_DS3, NOTE_DS3, NOTE_DS3, NOTE_FS3, NOTE_GS3, NOTE_FS3, NOTE_GS3,
        NOTE_DS3, 0, NOTE_DS3, NOTE_CS3, NOTE_DS3, NOTE_DS3, 0, NOTE_DS3, NOTE_CS3, NOTE_DS3,
        NOTE_DS3, 0, NOTE_DS3, NOTE_CS3, NOTE_DS3, NOTE_GS3, NOTE_FS3, NOTE_GS3, NOTE_FS3, NOTE_DS3, NOTE_FS3,
        NOTE_DS3, 0, NOTE_DS3, NOTE_CS3, NOTE_DS3, NOTE_DS3, 0, NOTE_DS3, NOTE_CS3, NOTE_DS3,
        NOTE_DS3, 0, NOTE_DS3, NOTE_CS3, NOTE_DS3, NOTE_DS3, NOTE_DS3, NOTE_FS3, NOTE_GS3, NOTE_FS3, NOTE_GS3,
        NOTE_DS3, 0, NOTE_DS3, NOTE_CS3, NOTE_DS3, NOTE_DS3, 0, NOTE_DS3, NOTE_CS3, NOTE_DS3,
        NOTE_DS3, 0, NOTE_DS3, NOTE_CS3, NOTE_DS3, NOTE_GS3, NOTE_FS3, NOTE_GS3, NOTE_FS3, NOTE_DS3, NOTE_FS3,
        // Verse(2) 1 - 16
        NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_DS5, NOTE_CS5,
        NOTE_AS4, NOTE_DS4, NOTE_AS4, NOTE_GS4, NOTE_FS4, NOTE_F4,
        NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_FS4,
        NOTE_F4, NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_F4, NOTE_DS4, NOTE_D4, NOTE_F4,
        NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_DS5, NOTE_CS5,
        NOTE_AS4, NOTE_DS4, NOTE_AS4, NOTE_GS4, NOTE_FS4, NOTE_F4,
        NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_FS4,
        NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4,
        NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_DS5, NOTE_CS5,
        NOTE_AS4, NOTE_DS4, NOTE_AS4, NOTE_GS4, NOTE_FS4, NOTE_F4,
        NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_FS4,
        NOTE_F4, NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_F4, NOTE_DS4, NOTE_D4, NOTE_F4,
        NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_DS5, NOTE_CS5,
        NOTE_AS4, NOTE_DS4, NOTE_AS4, NOTE_GS4, NOTE_FS4, NOTE_F4,
        NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_FS4,
        NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4,
        // Verse(2) 17 - 32
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_GS4, NOTE_FS4, NOTE_F4, NOTE_CS4, NOTE_DS4, NOTE_CS4, NOTE_DS4,
        NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_DS4, NOTE_AS4, NOTE_CS5,
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_GS4, NOTE_FS4, NOTE_F4, NOTE_CS4, NOTE_DS4, NOTE_CS4, NOTE_DS4,
        NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_DS4, NOTE_AS4, NOTE_CS5,
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_GS4, NOTE_FS4, NOTE_F4, NOTE_CS4, NOTE_DS4, NOTE_CS4, NOTE_DS4,
        NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, NOTE_DS4, NOTE_AS4, NOTE_CS5,
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        NOTE_CS5, NOTE_DS5, NOTE_AS4, NOTE_GS4, NOTE_AS4, NOTE_DS5, NOTE_F5,
        NOTE_FS5, NOTE_F5, NOTE_DS5, NOTE_CS5, NOTE_AS4, NOTE_GS4, NOTE_AS4,
        //                                               to 1=G
        NOTE_GS4, NOTE_FS4, NOTE_F4, NOTE_CS4, NOTE_DS4, NOTE_B4, NOTE_D5,
        // Verse(2) 33 - 48 1=G
        NOTE_D5, NOTE_E5, NOTE_B4, NOTE_A4, NOTE_B4, NOTE_A4, NOTE_B4,
        NOTE_D5, NOTE_E5, NOTE_B4, NOTE_A4, NOTE_B4, NOTE_A4, NOTE_B4,
        NOTE_A4, NOTE_G4, NOTE_FS4, NOTE_D4, NOTE_E4, NOTE_D4, NOTE_E4,
        NOTE_FS4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_E4, NOTE_B4, NOTE_D5,
        NOTE_D5, NOTE_E5, NOTE_B4, NOTE_A4, NOTE_B4, NOTE_A4, NOTE_B4,
        NOTE_D5, NOTE_E5, NOTE_B4, NOTE_A4, NOTE_B4, NOTE_A4, NOTE_B4,
        NOTE_A4, NOTE_G4, NOTE_FS4, NOTE_D4, NOTE_E4, NOTE_D4, NOTE_E4,
        NOTE_FS4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_E4, NOTE_B4, NOTE_D5,
        NOTE_D5, NOTE_E5, NOTE_B4, NOTE_A4, NOTE_B4, NOTE_A4, NOTE_B4,
        NOTE_D5, NOTE_E5, NOTE_B4, NOTE_A4, NOTE_B4, NOTE_A4, NOTE_B4,
        NOTE_A4, NOTE_G4, NOTE_FS4, NOTE_D4, NOTE_E4, NOTE_D4, NOTE_E4,
        NOTE_FS4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_E4, NOTE_B4, NOTE_D5,
        NOTE_D5, NOTE_E5, NOTE_B4, NOTE_A4, NOTE_B4, NOTE_A4, NOTE_B4,
        NOTE_D5, NOTE_E5, NOTE_B4, NOTE_A4, NOTE_B4, NOTE_E5, NOTE_FS5,
        NOTE_G5, NOTE_FS5, NOTE_E5, NOTE_D5, NOTE_B4, NOTE_A4, NOTE_B4,
        NOTE_A4, NOTE_G4, NOTE_FS4, NOTE_D4, NOTE_E4, 0,
        // Outro 1=G
        NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4,
        NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4,
        NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4,
        NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4, 0,
        0
        };
    // note durations: unit: 1/16 note, time = number * (1/16), ex: 1/4 note, number: 4, 1/8 note, number: 2:
    uint32_t noteDurations[] = {
        // Intro
        4, 4, 4, 1, 1, 1, 1,
        4, 4, 4, 2, 2,
        4, 4, 4, 1, 1, 1, 1,
        4, 4, 4, 2, 2,
        4, 4, 4, 1, 1, 1, 1,
        4, 4, 4, 2, 2,
        4, 4, 4, 1, 1, 1, 1,
        4, 4, 4, 2, 2,
        4, 1, 1, 1, 1, 4, 1, 1, 1, 1,
        4, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1,
        4, 1, 1, 1, 1, 4, 1, 1, 1, 1,
        4, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1,
        4, 1, 1, 1, 1, 4, 1, 1, 1, 1,
        4, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1,
        4, 1, 1, 1, 1, 4, 1, 1, 1, 1,
        4, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1,
        // Verse 1 - 16
        2, 2, 2, 2, 4, 2, 2,
        4, 4, 2, 2, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        4, 4, 2, 2, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        4, 4, 4, 4,
        2, 2, 2, 2, 4, 2, 2,
        4, 4, 2, 2, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        4, 4, 2, 2, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        4, 4, 4, 4,
        // Verse 17 - 32
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        // Verse 33 - 48
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 4,
        // Interlude
        4, 1, 1, 1, 1, 4, 1, 1, 1, 1,
        4, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1,
        4, 1, 1, 1, 1, 4, 1, 1, 1, 1,
        4, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1,
        4, 1, 1, 1, 1, 4, 1, 1, 1, 1,
        4, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1,
        4, 1, 1, 1, 1, 4, 1, 1, 1, 1,
        4, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1,
        // Verse(2) 1 - 16
        2, 2, 2, 2, 4, 2, 2,
        4, 4, 2, 2, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        4, 4, 2, 2, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        4, 4, 4, 4,
        2, 2, 2, 2, 4, 2, 2,
        4, 4, 2, 2, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        4, 4, 2, 2, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        4, 4, 4, 4,
        // Verse(2) 17 - 32
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        // Verse(2) 33 - 48
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 2, 2,
        2, 2, 2, 2, 4, 4,
        // Outro
        3, 3, 2, 3, 3, 2,
        3, 3, 2, 3, 3, 2,
        3, 3, 2, 3, 3, 2,
        3, 3, 2, 3, 3, 2,
        16
        };
    for (uint32_t i = 0; i < 895; i++) {

        int a=rand()%3;
        hal_gpio_output_high(&led[a]);
        //毫秒延时
        uint32_t noteDuration = ndms * noteDurations[i] / bpm;
        tone(melody[i], noteDuration);
        uint32_t pauseBetweenNotes = noteDuration / 4;
        // HAL_Delay(pauseBetweenNotes);
        aos_msleep(pauseBetweenNotes);
        hal_gpio_output_low(&led[a]);
        if(ledmod!=3)break;
    }
}


//播放badapple
void vedio(){
    int frame_index = 0; // 用于追踪当前帧的索引

    while(1)
    {
        // 显示当前帧
        OLED_Frame_Show_Pic(image_frames[frame_index], 0, 0, 85, 8);
        OLED_Refresh_GRAM();
        aos_msleep(100); // 设置每帧的持续时间

        // 更新帧索引，实现循环播放
        frame_index = (frame_index + 1) % 1095;
        if(ledmod!=4)break;
    }
}
//播放李翔宇视频
void lxy(){
    int frame_index = 0; // 用于追踪当前帧的索引

    while(1)
    {
        // 显示当前帧
        OLED_Frame_Show_Pic(image_frames2[frame_index], 0, 0, 113, 8);
        OLED_Refresh_GRAM();
        aos_msleep(50); // 设置每帧的持续时间

        // 更新帧索引，实现循环播放
        frame_index = (frame_index + 1) % 65;
        if(ledmod!=5)break;
    }
}
//主函数
int application_start(int argc, char *argv[])
{
    all_led_init() ;
    all_key_init();
    sh1106_init();
    OLED_Clear();
    OLED_Frame_Show_Pic(gImage_genshin, 0, 0, 132, 8);
    OLED_Refresh_GRAM();
    aos_msleep(2000);
    while(1){
        if(flag==0){
            OLED_Frame_Show_Pic(gImage_default, 0, 0, 132, 8);
            OLED_Refresh_GRAM();
            flag=0;
        }
        if(ledmod==0){
            if(flag==0){
                OLED_Frame_Show_Pic(gImage_led, 0, 0, 132, 8);
                OLED_Refresh_GRAM();
                flag=1;
            }
            for(int i=0;i<3;i++){
                if(key_scan(i)){
                    hal_gpio_output_toggle(&led[i]);
                }
            }

        }
        else if(ledmod==1){
            if(flag==0){
                OLED_Frame_Show_Pic(Image_1, 0, 0, 132, 8);
                OLED_Refresh_GRAM();
                flag=0;
            }
            flow_led();
            continue;
        }
        else if(ledmod==2){
            
        }
        else if(ledmod==3){
            play_melody();
        }
        else if(ledmod==4){
            vedio();
        }
        else if(ledmod==5){
            lxy();
        }
    }

}

