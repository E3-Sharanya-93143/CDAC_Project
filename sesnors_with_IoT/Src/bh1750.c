/*
 * bh1750.c
 *
 * Fully commented version
 * Created on: Nov 27, 2025
 * Author: sunbeam
 */

#include "bh1750.h"   // Header file containing function prototypes & macros
#include "i2c.h"      // Low-level register I2C driver
#include"timer.h"
#include <stdio.h>
#include "uart.h"
// ============================================================================
// Function: BH1750_WriteCommand
// Purpose : Sends one 8-bit command to BH1750 (datasheet: one command per STOP)
// ============================================================================
void BH1750_WriteCommand(uint8_t command)
{
    I2CStart();                             // Generate START condition on I2C bus
    I2CSendSlaveAddr(BH1750_ADDR_WRITE);    // Send BH1750 slave address (0x46 → write)
    I2CSendData(command);                   // Send the instruction byte (ex. 0x10)
    I2CStop();                              // Generate STOP condition (BH1750 requires STOP after every opcode)
}


// ============================================================================
// Function: BH1750_Init
// Purpose : Properly initialize BH1750 according to datasheet state machine:
//           Power On → Reset → Measurement Mode → Wait 180ms
// ============================================================================
void BH1750_Init(void)
{
	DelayMs(5);                            // Ensure DVI reset section >1µs (datasheet timing requirement)
    BH1750_WriteCommand(BH1750_POWER_ON);   // 0x01 → Put sensor in 'waiting for measurement' mode
    DelayMs(10);                           // Small delay after power on
    BH1750_WriteCommand(BH1750_RESET);      // 0x07 → Clear internal data registers
    DelayMs(10);                           // Reset must be followed by delay
    BH1750_WriteCommand(BH1750_CONT_HR_MODE); // 0x10 → Continuous high-resolution mode (1 lux resolution)
    DelayMs(180);                           // Wait for first measurement (max 180ms per datasheet)
}


// ============================================================================
// Function: BH1750_ReadLux
// Purpose : Reads 16-bit light data from BH1750 and converts it to lux.
//
// Datasheet Read Format:
//   START → ADDRESS(R) → ACK
//   High Byte  → ACK
//   Low Byte   → NACK
//   STOP
//
// Returned value = raw / 1.2   (datasheet formula)
// ============================================================================
uint16_t BH1750_ReadLux(void)
{
    uint8_t highByte;                       // MSB of measurement
    uint8_t lowByte;                        // LSB of measurement
    uint16_t raw;                           // Combined 16-bit raw ADC value

    DelayMs(180);                          // Wait for BH1750 to finish measurement (120–180ms)
    I2CStart();                             // Send START condition
    I2CSendSlaveAddr(BH1750_ADDR_READ);     // Send BH1750 read address (0x47)
    highByte = I2CRecvDataAck();            // Read high byte, master sends ACK to continue
    lowByte  = I2CRecvDataNAck();           // Read low byte, master sends NACK (end of read)
    I2CStop();                              // Send STOP condition
    raw = (highByte << 8) | lowByte;        // Combine MSB + LSB → 16-bit raw measurement
    float lux = (float)raw / 1.2f;          // Convert raw value to lux (datasheet: lux = raw / 1.2)
    return (uint16_t)lux;                   // Return rounded lux value as integer
}

uint8_t GrowLightControl(void)
{
    uint16_t lux = BH1750_ReadLux();
    uint8_t duty = 0;
    uint8_t lamp_state;
    char msg[32];

    /* Decide PWM duty based on light intensity */
    if (lux < 2000)       duty = 90;
    else if (lux < 5000)  duty = 70;
    else if (lux < 10000) duty = 40;
    else if (lux < 15000) duty = 20;
    else                  duty = 0;

    /* Apply PWM (TIM8 CH1) */
    TIM8->CCR1 = duty;

    /* Lamp ON/OFF status */
    lamp_state = (duty > 0) ? 1 : 0;

    /* Send UART data to ESP8266
       Format: <lux>,<lamp_state>\n
       Example: 3456,1
    */
//    sprintf(msg, "%u,%u\n", lux, lamp_state);
//    UartPuts(msg);

    return lamp_state;
}
