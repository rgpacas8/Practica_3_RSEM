/**
 * @file    RSE_CAN.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"
#include "task.h"
#include "FreeRTOS.h"
/* TODO: insert other include files here. */
#include "fsl_adc16.h"
#include "fsl_flexcan.h"
#include "ADC.h"

/* TODO: insert other definitions and declarations here. */
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*
 * ADC0   -> PTB_2
 * TX CAN -> PTB_18
 * RX CAN -> PTB_19
 * https://community.st.com/s/question/0D50X00009XkWRUSA3/stm32f032-can-bus
 * https://forum.arduino.cc/index.php?topic=148791.0
 */
#define DEMO_ADC16_BASE          ADC0
#define DEMO_ADC16_CHANNEL_GROUP 0U
#define DEMO_ADC16_USER_CHANNEL  12U

#define EXAMPLE_CAN            CAN0
#define EXAMPLE_CAN_CLK_SOURCE (kFLEXCAN_ClkSrc1)
#define EXAMPLE_CAN_CLK_FREQ   CLOCK_GetFreq(kCLOCK_BusClk)
#define RX_MESSAGE_BUFFER_NUM  (9)
#define TX_MESSAGE_BUFFER_NUM  (8)
#define DLC                    (8)

#define MSG1TxBATTERY          (0x25)

/* Fix MISRA_C-2012 Rule 17.7. */
#define LOG_INFO (void)PRINTF
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

volatile bool txComplete = pdFALSE;
volatile bool rxComplete = pdFALSE;
flexcan_handle_t flexcanHandle;
flexcan_mb_transfer_t txBATTERYfer;
flexcan_frame_t txBATTERY_FRAME;
uint8_t RxMBID;


static void flexcan_callback(CAN_Type *base, flexcan_handle_t *handle, status_t status, uint32_t result, void *userData)
{
	switch (status)
    {
        /* Process FlexCAN Rx event. */
        case kStatus_FLEXCAN_RxIdle:
            if (RX_MESSAGE_BUFFER_NUM == result)
            {
            	RxMBID = RX_MESSAGE_BUFFER_NUM;
            	rxComplete = pdTRUE;
            }
            break;

        /* Process FlexCAN Tx event. */
        case kStatus_FLEXCAN_TxIdle:
            if (TX_MESSAGE_BUFFER_NUM == result)
            {
                txComplete = pdTRUE;
            }
            break;

        default:
            break;
    }
}

void CAN_Init(void){

	flexcan_config_t flexcanConfig;
    flexcan_rx_mb_config_t mbConfig;

	/* Init FlexCAN module. */
    /*
     * flexcanConfig.clkSrc                 = kFLEXCAN_ClkSrc0;
     * flexcanConfig.baudRate               = 1000000U;
     * flexcanConfig.baudRateFD             = 2000000U;
     * flexcanConfig.maxMbNum               = 16;
     * flexcanConfig.enableLoopBack         = false;
     * flexcanConfig.enableSelfWakeup       = false;
     * flexcanConfig.enableIndividMask      = false;
     * flexcanConfig.disableSelfReception   = false;
     * flexcanConfig.enableListenOnlyMode   = false;
     * flexcanConfig.enableDoze             = false;
     */
    FLEXCAN_GetDefaultConfig(&flexcanConfig);

#if defined(EXAMPLE_CAN_CLK_SOURCE)
    flexcanConfig.clkSrc = EXAMPLE_CAN_CLK_SOURCE;
#endif
    /*Change CAN BR to 125kbps*/
    flexcanConfig.baudRate = 125000U;
    FLEXCAN_Init(EXAMPLE_CAN, &flexcanConfig, EXAMPLE_CAN_CLK_FREQ);

    /* Setup Tx Message Buffer. */
    FLEXCAN_SetTxMbConfig(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, true);
    /* Prepare Tx Frame for sending. */
    txBATTERY_FRAME.format = (uint8_t)kFLEXCAN_FrameFormatStandard;
    txBATTERY_FRAME.type   = (uint8_t)kFLEXCAN_FrameTypeData;
    txBATTERY_FRAME.id     = FLEXCAN_ID_STD(MSG1TxBATTERY);
    txBATTERY_FRAME.length = (uint8_t)DLC;


    /* Create FlexCAN handle structure and set call back function. */
    FLEXCAN_TransferCreateHandle(EXAMPLE_CAN, &flexcanHandle, flexcan_callback, NULL);
}

void vTaskTx10ms(void * pvParameters)
{
	TickType_t xLastWakeTime;
	const TickType_t xPeriod = pdMS_TO_TICKS(10);
	xLastWakeTime = xTaskGetTickCount();
	static uint8_t TxByte0 = 0;
	static uint16_t TicksCounter = 0;
	static uint16_t ADC_lvl;

	 /* Enter the loop that defines the task behavior. */
	 for(;;){
		 uint8_t prueba[3];
		 vTaskDelayUntil(&xLastWakeTime, xPeriod);
		 ADC_lvl=ADC_value_demo();
		 ADC_lvl=((ADC_lvl-190)*100)/3700;
		 prueba[0] = ADC_lvl%10;
		 prueba[1] = (ADC_lvl/10)%10;
		 prueba[2] = (ADC_lvl/100)%10;
		/*Increment a random value for demo*/
		 TicksCounter++;
		 if(TicksCounter > 300){
			 if(TxByte0 < 100){
				 TxByte0 += 5;
			 }else{
				 TxByte0 = 0;
			 }
			 TicksCounter = 0;
		 }else{}

		 /* Perform the periodic actions here. */
		 txBATTERY_FRAME.dataByte0 = prueba[0]+0x30;
		 txBATTERY_FRAME.dataByte1 = prueba[1]+0x30;
		 txBATTERY_FRAME.dataByte2 = prueba[2]+0x30;
		 txBATTERY_FRAME.dataByte3 = 0x03;
		 txBATTERY_FRAME.dataByte4 = 0x04;
		 txBATTERY_FRAME.dataByte5 = 0x05;
		 txBATTERY_FRAME.dataByte6 = 0x06;
		 txBATTERY_FRAME.dataByte7 = 0x07;

		/* Send data through Tx Message Buffer. */
		txBATTERYfer.mbIdx = (uint8_t)TX_MESSAGE_BUFFER_NUM;
		txBATTERYfer.frame = &txBATTERY_FRAME;
		(void)FLEXCAN_TransferSendNonBlocking(EXAMPLE_CAN, &flexcanHandle, &txBATTERYfer);
	 }
}

int main(void) {

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif

    ADC0_init_demo();
    CAN_Init();

    if(xTaskCreate(vTaskTx10ms,"BatteryLevel",(configMINIMAL_STACK_SIZE+100),NULL,(configMAX_PRIORITIES-1),NULL) != pdPASS){
      	PRINTF("FAIL to create vTaskTx10ms");
      }

    /* Start the scheduler. */
       vTaskStartScheduler();

    /* Enter an infinite loop, just incrementing a counter. */
    while(pdTRUE) {
        //printf("adc_value =%d\n",ADC_value_demo());
    }
    return 0 ;
}
