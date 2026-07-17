#include "stm32f4xx.h"

// I2S1_WS: PA4
// I2S1_CK: PA5
// I2S1_SD: PA7

typedef enum
{
    PIN_MODE_INPUT  = 0,
    PIN_MODE_OUTPUT = 1,
    PIN_MODE_AF     = 2,
    PIN_MODE_ANALOG = 3,
} PinMode;

static void set_pin_mode(GPIO_TypeDef* cluster, uint32_t pin_num, PinMode mode)
{
    pin_num &= 0xF;
    uint32_t bit = pin_num * 2;
    cluster->MODER &= ~(3u << (bit));
    cluster->MODER |= (mode & 3u) << bit;
}

static void set_pin_af(GPIO_TypeDef* cluster, uint32_t pin_num, uint32_t af_num)
{
    pin_num &= 0xF;
    af_num &= 0xF;
    volatile uint32_t* afr = cluster->AFR + (pin_num / 8);
    uint32_t bit = (pin_num & 0x7u) * 4;
    *afr &= ~(0xFu << bit); // clear AF selection
    *afr |= (af_num << bit); // select AF5
}

int main(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    set_pin_mode(GPIOA, 4, PIN_MODE_AF);
    set_pin_mode(GPIOA, 5, PIN_MODE_AF);
    set_pin_mode(GPIOA, 7, PIN_MODE_AF);
    set_pin_af(GPIOA, 4, 5);
    set_pin_af(GPIOA, 5, 5);
    set_pin_af(GPIOA, 7, 5);
    SPI1->I2SCFGR |= 0x0C00;
    while(1)
    {

    }
}
