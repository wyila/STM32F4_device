#include "tim.h"

/****************************** map **********************************
 *  TIM3 控制 LED 闪烁
 *  TIM4 用于等待上位机的命令，延时用
 *  TIM6 用来当延时
 *  TIM7 暂时放着
 *  systick用来当系统心跳
 ****************************** end **********************************/


/************************* TIM3 ****************************/
int TIM3_IT_STATUS = 0;
TIM_HandleTypeDef TIM3_Handler;      //定时器句柄

//定时器时钟为90MHz
//通用定时器3中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器3!(定时器3挂在APB1上，时钟为sysclock/2)
void TIM3_Init(u16 arr, u16 psc)
{
    __HAL_RCC_TIM3_CLK_DISABLE();//关掉定时器，防止莫名其妙的事发生
    TIM3_Handler.Instance = TIM3;                          //通用定时器3
    TIM3_Handler.Init.Prescaler = psc;                     //分频系数
    TIM3_Handler.Init.CounterMode = TIM_COUNTERMODE_UP;    //向上计数器
    TIM3_Handler.Init.Period = arr;                        //自动装载值
    TIM3_Handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;//时钟分频因子
    HAL_TIM_Base_Init(&TIM3_Handler);

    HAL_TIM_Base_Start_IT(&TIM3_Handler); //使能定时器3和定时器3更新中断：TIM_IT_UPDATE
}

//定时器3中断服务函数
void TIM3_IRQHandler(void)
{
    if(TIM3->SR&0X0001)//溢出中断
	{
		TIM3_IT_STATUS = 1;
        GPIOC->ODR ^= GPIO_PIN_13;
	}
	TIM3->SR &= ~0x01;//清除中断标志位
}
/************************* end ****************************/

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

/***********************************************************/
/************************* TIM4 ****************************/
/***********************************************************/
TIM_HandleTypeDef TIM4_Handler;      //定时器句柄
unsigned int TIM4_interrupt_cnt = 0;
//定时器时钟为90MHz
//arr：自动重装值。
//psc：时钟预分频数
void TIM4_Init(u16 arr, u16 psc)
{
    __HAL_RCC_TIM4_CLK_DISABLE();//关掉定时器
    TIM4_Handler.Instance = TIM4;                          //通用定时器4
    TIM4_Handler.Init.Period = arr;                        //自动装载值
    TIM4_Handler.Init.Prescaler = psc;                     //分频系数
    TIM4_Handler.Init.CounterMode = TIM_COUNTERMODE_UP;    //向上计数器
    TIM4_Handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;//时钟分频因子
    HAL_TIM_Base_Init(&TIM4_Handler);

    HAL_TIM_Base_Start_IT(&TIM4_Handler); //使能定时器4和定时器4更新中断：TIM_IT_UPDATE
}

//定时器4中断服务函数
void TIM4_IRQHandler(void)
{
    if(TIM4->SR & 0X0001)//溢出中断
	{
		TIM4_interrupt_cnt ++;
        //GPIOC->ODR ^= GPIO_PIN_13;
	}
	TIM4->SR &= ~0x01;//清除中断标志位
}
/************************* end ****************************/

/****************************** TIM 6 ************************************/
TIM_HandleTypeDef TIM6_Handler;      //定时器句柄
unsigned int TIM6_interrupt_cnt = 0;
void TIM6_Init(u16 arr, u16 psc)
{
    __HAL_RCC_TIM6_CLK_DISABLE();//关掉定时器
    TIM6_Handler.Instance = TIM6;                          //通用定时器4
    TIM6_Handler.Init.Period = arr;                        //自动装载值
    TIM6_Handler.Init.Prescaler = psc;                     //分频系数
    TIM6_Handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;//时钟分频因子
    HAL_TIM_Base_Init(&TIM6_Handler);

    HAL_TIM_Base_Start_IT(&TIM6_Handler); //使能定时器6和定时器6更新中断：TIM_IT_UPDATE
}
//定时器6中断服务函数
void TIM6_DAC_IRQHandler(void)
{
    if(TIM6->SR & 0X0001)//溢出中断
	{
		TIM6_interrupt_cnt ++;
	}
	TIM6->SR = 0;//清除中断标志位
}

/******************************* end ************************************/


/****************************** TIM 7 ************************************/
TIM_HandleTypeDef TIM7_Handler;      //定时器句柄
unsigned int TIM7_interrupt_cnt = 0;
void TIM7_Init(u16 arr, u16 psc)
{
    __HAL_RCC_TIM7_CLK_DISABLE();//关掉定时器
    TIM7_Handler.Instance = TIM7;                          //通用定时器4
    TIM7_Handler.Init.Period = arr;                        //自动装载值
    TIM7_Handler.Init.Prescaler = psc;                     //分频系数
    TIM7_Handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;//时钟分频因子
    HAL_TIM_Base_Init(&TIM7_Handler);

    HAL_TIM_Base_Start_IT(&TIM7_Handler); //使能定时器7和定时器7更新中断：TIM_IT_UPDATE
}
//定时器7中断服务函数
void TIM7_IRQHandler(void)
{
    if(TIM7->SR & 0X0001)//溢出中断
	{
		TIM7_interrupt_cnt ++;
	}
    //基本定时器就第0位有用，不怕误杀友军
	TIM7->SR = 0;//清除中断标志位
}

/******************************* end ************************************/

//定时器底册驱动，开启时钟，设置中断优先级
//此函数会被HAL_TIM_Base_Init()函数调用
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    
    if(htim->Instance==TIM3)
	{
		__HAL_RCC_TIM3_CLK_ENABLE();            //使能TIM3时钟
		HAL_NVIC_SetPriority(TIM3_IRQn, 4, 0);    //设置中断优先级，抢占优先级1，子优先级3
		HAL_NVIC_EnableIRQ(TIM3_IRQn);          //开启ITM3中断
	}
    
    if(htim->Instance==TIM4)
	{
		__HAL_RCC_TIM4_CLK_ENABLE();            //使能TIM3时钟
		HAL_NVIC_SetPriority(TIM4_IRQn, 4, 0);    //设置中断优先级，抢占优先级1，子优先级3
		HAL_NVIC_EnableIRQ(TIM4_IRQn);          //开启ITM3中断
	}
    
    if(htim->Instance==TIM6)
	{
		__HAL_RCC_TIM6_CLK_ENABLE();            //使能TIM3时钟
		HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 2, 0);    //设置中断优先级，抢占优先级1，子优先级3
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);          //开启ITM3中断
	}
    
    if(htim->Instance==TIM7)
	{
		__HAL_RCC_TIM7_CLK_ENABLE();            //使能TIM3时钟
		HAL_NVIC_SetPriority(TIM7_IRQn, 5, 0);    //设置中断优先级，抢占优先级1，子优先级3
		HAL_NVIC_EnableIRQ(TIM7_IRQn);          //开启ITM3中断
	}
}


void TIM_Init(void)
{
    TIM3_Init(4999, SystemCoreClock / 2 / 10000);//设置定时器频率为10000Hz
    //TIM4_Init(4999, SystemCoreClock / 2 / 10000);
}



















