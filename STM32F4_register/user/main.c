#include "main.h"
#include "ble.h"
#include "sdram.h"

unsigned char *sdram = (u8 *)Bank5_SDRAM_ADDR;

int main(void)
{
    unsigned char buf[64];
    Stm32_Clock_Init(360,25,2,8);   //设置时钟,180Mhz
    delay_init(180);
    SDRAM_Init();
    usart_init(115200);
    LED_Init();
    TIM6_Init(4999, TIM_CLK / 10000 - 1);
    IIC_Init();
    
    printf("This is APP\r\n");
    
    while(1)
    {
        
        //delay_ms(500);
        //LED0 = !LED0;
    }
}





