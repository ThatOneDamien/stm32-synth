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
    // Power the GPIO A group
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    
    // Power the SPI1 peripheral
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // Enable HSE (8MHz on my board the Nucleo F446RE) located externally from the MCU, and disable PLL/PLLI2S
    RCC->CR = (RCC->CR & ~(RCC_CR_PLLI2SON_Msk | RCC_CR_PLLON_Msk | RCC_CR_HSEON_Msk)) 
                | RCC_CR_HSEON; 
    while (1)
    {
        if((RCC->CR & (RCC_CR_HSERDY | RCC_CR_PLLRDY | RCC_CR_PLLI2SRDY)) == RCC_CR_HSERDY) // Continue when HSE is ready, PLL and PLLI2S are disabled.
            break;
    }

    // Select the HSE as the entry clock source for PLL and PLLI2S
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC;

    // Select the I2S1 source clock to be PLLI2S as configured
    RCC->DCKCFGR &= ~RCC_DCKCFGR_I2S1SRC_Msk;

    uint32_t needed_bits = RCC->PLLI2SCFGR & ~(RCC_PLLI2SCFGR_PLLI2SM_Msk | 
                                               RCC_PLLI2SCFGR_PLLI2SN_Msk |
                                               RCC_PLLI2SCFGR_PLLI2SR_Msk);

    // We take 8MHz from the HSE, then divide it by 4 so that VCOin is 2MHz
    // multiply it by 96 to get 192MHz for VCOout, then divide that by 4 to get 48MHz
    // as the input clock for the I2S1 peripheral.
    RCC->PLLI2SCFGR = needed_bits |
        (4 << RCC_PLLI2SCFGR_PLLI2SM_Pos) |
        (96 << RCC_PLLI2SCFGR_PLLI2SN_Pos) |
        (4 << RCC_PLLI2SCFGR_PLLI2SR_Pos);

    // Enable PLLI2S
    RCC->CR |= RCC_CR_PLLI2SON;

    while (1)
    {
        if(RCC->CR & RCC_CR_PLLI2SRDY) // Continue when PLLI2S is ready
            break;
    }

    // Set all of the required pins to be alternate function
    set_pin_mode(GPIOA, 4, PIN_MODE_AF);
    set_pin_mode(GPIOA, 5, PIN_MODE_AF);
    set_pin_mode(GPIOA, 7, PIN_MODE_AF);

    // Set the alternate function of those pins to be the SPI/I2S function(AF5)
    set_pin_af(GPIOA, 4, 5);
    set_pin_af(GPIOA, 5, 5);
    set_pin_af(GPIOA, 7, 5);

    // Configure I2S for master + transmitter
    SPI1->I2SCFGR = (SPI1->I2SCFGR & 0xE040) |
                        SPI_I2SCFGR_I2SMOD |
                        SPI_I2SCFGR_I2SCFG_0 |
                        SPI_I2SCFGR_I2SCFG_1;

    // Configure the divider such that we get an accurate 44100Hz output
    // This sets the divider to 17, ODD to 0, so the effective divider is 34
    // and the master clock output to disabled 
    SPI1->I2SPR = (SPI1->I2SPR & 0xFC00) |
                    (17 << SPI_I2SPR_I2SDIV_Pos); // Divider value

    // Enable I2S mode
    SPI1->I2SCFGR |= SPI_I2SCFGR_I2SE;
    while(1)
    {

    }
}
