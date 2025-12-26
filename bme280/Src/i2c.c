/*
 * i2c.c
 *
 *  Created on: Apr 2, 2024
 *      Author: Nilesh
 */

#include "i2c.h"

void I2CStart() {
    I2C2->CR1 |= I2C_CR1_START;
    while ((I2C2->SR1 & I2C_SR1_SB) == 0);
}

void I2CRepeatStart() {
    I2CStart();
}

void I2CStop() {
    I2C2->CR1 |= I2C_CR1_STOP;
    while (I2C2->SR2 & I2C_SR2_BUSY);
}

void I2CSendSlaveAddress(uint8_t slaveaddr) {
    I2C2->DR = slaveaddr;
    while (!(I2C2->SR1 & I2C_SR1_ADDR));
    (void)I2C2->SR1;
    (void)I2C2->SR2;
}

void I2CSendData(uint8_t val) {
    while (!(I2C2->SR1 & I2C_SR1_TXE));
    I2C2->DR = val;
    while (!(I2C2->SR1 & I2C_SR1_BTF));
}

uint8_t I2CRecvDataAck(void) {
    uint8_t val;
    I2C2->CR1 |= I2C_CR1_ACK | I2C_CR1_POS;
    while (!(I2C2->SR1 & I2C_SR1_RXNE));
    val = (uint8_t)I2C2->DR;
    return val;
}

uint8_t I2CRecvDataNAck(void) {
    uint8_t val;
    I2C2->CR1 &= ~(I2C_CR1_ACK | I2C_CR1_POS);
    while (!(I2C2->SR1 & I2C_SR1_RXNE));
    val = (uint8_t)I2C2->DR;
    return val;
}

int I2CIsDeviceReady(uint8_t slaveaddr) {
    slaveaddr &= ~(1 << 0); // make sure LSB = 0 for write
    I2C2->DR = slaveaddr;
    while (!(I2C2->SR1 & I2C_SR1_ADDR));
    (void)I2C2->SR2;
    return 1;
}

void I2CInit() {
    // Enable GPIOB clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    DelayMs(50);

    /* ---- Configure PB10 (SCL) ---- */
    GPIOB->MODER &= ~(3UL << (10 * 2));   // clear
    GPIOB->MODER |=  (2UL << (10 * 2));   // AF mode
    GPIOB->OTYPER |= (1UL << 10);         // open-drain
    GPIOB->PUPDR  &= ~(3UL << (10 * 2));  // no pull-up/pull-down
    GPIOB->AFR[1] &= ~(0xF << ((10 - 8) * 4));
    GPIOB->AFR[1] |=  (4UL << ((10 - 8) * 4)); // AF4 → I2C2

    /* ---- Configure PB11 (SDA) ---- */
    GPIOB->MODER &= ~(3UL << (11 * 2));
    GPIOB->MODER |=  (2UL << (11 * 2));
    GPIOB->OTYPER |= (1UL << 11);
    GPIOB->PUPDR  &= ~(3UL << (11 * 2));
    GPIOB->AFR[1] &= ~(0xF << ((11 - 8) * 4));
    GPIOB->AFR[1] |=  (4UL << ((11 - 8) * 4)); // AF4 → I2C2

    // Enable I2C2 clock
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
    DelayMs(50);

    // Reset I2C2
    I2C2->CR1 = I2C_CR1_SWRST;
    I2C2->CR1 = 0;

    // Disable error interrupts
    I2C2->CR2 &= ~(I2C_CR2_ITERREN);

    // Standard mode (100 kHz)
    I2C2->CCR &= ~(1 << I2C_CCR_FS_Pos);

    // Enable ACK
    I2C2->CR1 |= I2C_CR1_ACK;

    // APB1 = 16 MHz
    I2C2->CR2 |= (16 << I2C_CR2_FREQ_Pos);

    // 100 kHz CCR
    I2C2->CCR |= (80 << I2C_CCR_CCR_Pos);

    // Rise time
    I2C2->TRISE = 17;

    // Enable I2C2
    I2C2->CR1 |= I2C_CR1_PE;
}
