/*
 * timer.c
 *
 *  Created on: Apr 4, 2025
 *      Author: admin
 */

/*
 * timer.c
 *
 *  Created on: Apr 4, 2025
 *  Author: admin
 */

#include "timer.h"
#include "stm32f4xx.h"

#define TIM_PR 16   // Prescaler value

void TimerPwmInit(void)
{
    /* ---------- GPIO PC6 CONFIG (TIM8_CH1) ---------- */

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

    // Clear mode bits first
    GPIOC->MODER &= ~(3 << (2 * 6));
    // Set Alternate Function mode
    GPIOC->MODER |=  (2 << (2 * 6));

    // No pull-up / pull-down
    GPIOC->PUPDR &= ~(3 << (2 * 6));

    // AF3 = TIM8_CH1
    GPIOC->AFR[0] &= ~(0xF << (6 * 4));
    GPIOC->AFR[0] |=  (3 << (6 * 4));

    /* ---------- TIM8 PWM CONFIG ---------- */

    RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;

    // Timer clock = 16 MHz / 16 = 1 MHz
    TIM8->PSC = TIM_PR - 1;

    // PWM frequency = 1 MHz / 100 = 10 kHz
    TIM8->ARR = 100 - 1;

    // Initial duty cycle = 50%
    TIM8->CCR1 = 50;

    // Channel 1 output mode
    TIM8->CCMR1 &= ~(TIM_CCMR1_CC1S);
    TIM8->CCMR1 |= (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2); // PWM mode 1
    TIM8->CCMR1 |= TIM_CCMR1_OC1PE;

    // Active high polarity
    TIM8->CCER &= ~TIM_CCER_CC1P;
    TIM8->CCER |=  TIM_CCER_CC1E;

    // Auto-reload preload enable
    TIM8->CR1 |= TIM_CR1_ARPE;

    // Enable main output (ADVANCED TIMER requirement)
    TIM8->BDTR |= TIM_BDTR_MOE;

    // Start timer
    TIM8->CR1 |= TIM_CR1_CEN;
}
