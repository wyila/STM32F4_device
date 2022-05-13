#include "sys.h"
#include "cmd.h"
#include "i2c.h"
#include "iap.h"
#include "usart.h"
#include <string.h>

int cmd_proc(unsigned char *cmd, unsigned int len)
{
    unsigned char data = 0;
    switch(len)
    {
        case 6:
            if(!memcmp(cmd, "update", len))
            {
                data = 1;
                IIC_WriteData(0, &data, 1);
                iap_load_app(0x8000000);
                printf("I sent you a thousand miles away\n");
                return 1;
            }
            break;
        case 11:
            if(!memcmp(cmd, "turn on led", len))
            {
                __HAL_RCC_TIM3_CLK_DISABLE();//关闭定时器3，定时器3用于LED闪烁
                GPIOC->ODR |= GPIO_PIN_13;//LED挂在PC13上
                printf("Turn on the LED success\n");
                return 1;
            }
            break;
        case 12:
            if(!memcmp(cmd, "turn off led", len))
            {
                __HAL_RCC_TIM3_CLK_DISABLE();//关闭定时器3，定时器3用于LED闪烁
                GPIOC->ODR &= ~GPIO_PIN_13;//LED挂在PC13上
                printf("Turn off the LED success\n");
                return 1;
            }
            break;
        case 20:
            if(!memcmp(cmd, "turn on flashing led", len))
            {
                __HAL_RCC_TIM3_CLK_ENABLE();//打开定时器3，定时器3用于LED闪烁
                printf("Turn on the flashing LED success\n");
                return 1;
            }
            break;
        case 21:
            if(!memcmp(cmd, "turn off flashing led", len))
            {
                __HAL_RCC_TIM3_CLK_DISABLE();//关闭定时器3，定时器3用于LED闪烁
                GPIOC->ODR &= ~GPIO_PIN_13;//LED挂在PC13上
                printf("Turn off the flashing LED success\n");
                return 1;
            }
            break;
        default:
            printf("You send data is: %s\n", USART_RX_BUF);
            return 0;
    }
    printf("You send data is: %s\n", USART_RX_BUF);
    return 0;
}

