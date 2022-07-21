#include "main.h"

unsigned char *sdram = (u8 *)Bank5_SDRAM_ADDR;

int main(void)
{
    Stm32_Clock_Init(360,25,2,8);   //设置时钟,180Mhz
    delay_init();
    SDRAM_Init();
    usart_init(115200);
    LED_Init();
    TIM6_Init(4999, TIM_CLK / 10000 - 1);
    IIC_Init();
    
    printf("This is APP\r\n");
    
    while(1)
    {
        
        //delay_ms(2);
        //LED0 = !LED0;
    }
}





