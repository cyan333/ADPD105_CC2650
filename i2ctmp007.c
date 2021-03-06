/*
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *    ======== i2ctmp007.c ========
 */

/* XDCtools Header files */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

/* TI-RTOS Header files */
#include <ti/drivers/PIN.h>
#include <ti/drivers/I2C.h>

/* Example/Board Header files */
#include "Board.h"

#define TASKSTACKSIZE       1024
#define TMP007_OBJ_TEMP     0x0003  /* Object Temp Result Register */

/* Global memory storage for a PIN_Config table */
static PIN_State ledPinState;

/*
 * Application LED pin configuration table:
 *   - All LEDs board LEDs are off.
 */
PIN_Config ledPinTable[] = {
    Board_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_LED2 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

/*
 *  ======== echoFxn ========
 *  Task for this function is created statically. See the project's .cfg file.
 */
Void taskFxn(UArg arg0, UArg arg1)
{
    unsigned int    i;
    uint16_t        temperature;
    uint16_t        humidity;
    uint8_t         txBuffer[4];
    uint8_t         rxBuffer[4];
    I2C_Handle      i2c;
    I2C_Params      i2cParams;
    I2C_Transaction i2cTransaction;

    /* Create I2C for usage */
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2c = I2C_open(Board_I2C_TMP, &i2cParams);
    if (i2c == NULL) {
        System_abort("Error Initializing I2C\n");
    }
    else {
        System_printf("I2C Initialized!\n");
    }

    //Read HDC1080 device ID
    txBuffer[0] = 0xFF;
    txBuffer[1] = 0x10;
    txBuffer[2] = 0x00;
    i2cTransaction.slaveAddress = 0x64;//HDC1080 ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 2;

    if (I2C_transfer(i2c, &i2cTransaction)){
        System_printf("Device ID: %x%x\n", rxBuffer[0], rxBuffer[1]);
    } else {
        System_printf("Device ID fail!\n");
    }
    if (I2C_transfer(i2c, &i2cTransaction)){
        System_printf("Device ID: %x%x\n", rxBuffer[0], rxBuffer[1]);
    } else {
        System_printf("Device ID fail!\n");
    }


    //Read HDC1080 ADDR Manufacture ID
    txBuffer[0] = 0xFE;
    i2cTransaction.slaveAddress = 0x64;//HDC1080 ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 2;
    if (I2C_transfer(i2c, &i2cTransaction)) {
        System_printf("Manufacture ID: %x%x\n", rxBuffer[0], rxBuffer[1]);
    } else {
        System_printf("Manufacture ID fail!\n");
    }

    //Config HDC1080
    txBuffer[0] = 0x02;
    txBuffer[1] = 0x10;
    txBuffer[2] = 0x00;
    i2cTransaction.slaveAddress = 0x40;//Board_TMP006_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 3;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 0;

    if (I2C_transfer(i2c, &i2cTransaction)){
        System_printf("Config write!\n");
        //System_printf("Conf: 0x%x 0x%x\n", rxBuffer[0], rxBuffer[1]);
    } else {
        System_printf("Config fail!\n");
    }

    //Read HDC1080 Config
    txBuffer[0] = 0x02;
    txBuffer[1] = 0x10;
    txBuffer[2] = 0x00;
    i2cTransaction.slaveAddress = 0x40;//Board_TMP006_ADDR;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 2;
    if (I2C_transfer(i2c, &i2cTransaction)){
        System_printf("Config read!\n");
        System_printf("Conf: 0x%x 0x%x\n", rxBuffer[0], rxBuffer[1]);
    } else {
        System_printf("Config fail!\n");
    }


    /* Take 20 samples and print them out onto the console */
    for (i = 0; i < 20; i++) {
        txBuffer[0] = 0x00;
        i2cTransaction.slaveAddress = 0x40;//HDC1080 ADDR;
        i2cTransaction.writeBuf = txBuffer;
        i2cTransaction.writeCount = 1;
        i2cTransaction.readBuf = rxBuffer;
        i2cTransaction.readCount = 0;
        if (I2C_transfer(i2c, &i2cTransaction)) {
            System_printf("Temp/Humd!\n");
        } else {
            System_printf("Temp/Humd Fail!\n");
        }

        Task_sleep(1000000 / Clock_tickPeriod);
        txBuffer[0] = 0x00;
        i2cTransaction.slaveAddress = 0x40;//HDC1080 ADDR;
        i2cTransaction.writeBuf = txBuffer;
        i2cTransaction.writeCount = 0;
        i2cTransaction.readBuf = rxBuffer;
        i2cTransaction.readCount = 4;
        if (I2C_transfer(i2c, &i2cTransaction)) {
            /* Extract degrees C from the received data */
            temperature = rxBuffer[0];
            temperature = (temperature<<8 br="">            temperature |= rxBuffer[1];
            temperature = ((double)(int16_t)temperature / 65536)*165 - 40;

            System_printf("Temperature %u: %d (C)\n", i, temperature);

            /* Extract humidity RH from the received data */
            humidity = rxBuffer[2];
            humidity = (humidity<<8 br="">            humidity |= rxBuffer[3];
            //-- calculate relative humidity [%RH]
            humidity = ((double)humidity / 65536)*100;

            System_printf("Humidity %u: %d (RH)\n", i, humidity);
        }
        else {
            System_printf("I2C Bus fault\n");
        }

        System_flush();
        Task_sleep(1000000 / Clock_tickPeriod);
    }

    /* Deinitialized I2C */
    I2C_close(i2c);
    System_printf("I2C closed!\n");

    System_flush();
}

/*
 *  ======== main ========
 */
int main(void)
{
    PIN_Handle ledPinHandle;
    Task_Params taskParams;

    /* Call board init functions */
    Board_initGeneral();
    Board_initI2C();

    /* Construct tmp007 Task thread */
    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task0Stack;
    Task_construct(&task0Struct, (Task_FuncPtr)taskFxn, &taskParams, NULL);

    /* Open LED pins */
    ledPinHandle = PIN_open(&ledPinState, ledPinTable);
    if(!ledPinHandle) {
        System_abort("Error initializing board LED pins\n");
    }

    PIN_setOutputValue(ledPinHandle, Board_LED1, 1);

    System_printf("Starting the I2C example\nSystem provider is set to SysMin."
                  " Halt the target to view any SysMin contents in ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
