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
#define MSG1TxIDKEEPALIVE      (0x100)
#define MSG1TxBATTERY          (0x25)
#define MSG1RxACTUADOR         (0x55)

/* The CAN clock prescaler = CAN source clock/(baud rate * quantum), and the prescaler must be an integer.
   The quantum default value is set to 10=(3+2+1)+4, because for most platforms the CAN clock frequency is
   a multiple of 10. e.g. 120M CAN source clock/(1M baud rate * 10) is an integer. If the CAN clock frequency
   is not a multiple of 10, users need to set SET_CAN_QUANTUM and define the PSEG1/PSEG2/PROPSEG (classical CAN)
   and FPSEG1/FPSEG2/FPROPSEG (CANFD) vaule. Or can set USE_IMPROVED_TIMING_CONFIG macro to use driver api to
   calculates the improved timing values. */

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
flexcan_mb_transfer_t txKEEPALIVE,txBATTERY, rxACTUADOR;
flexcan_frame_t txKEEPALIVE_FRAME,txBATTERY_FRAME, rxACTUADOR_Frame;
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

    /* Setup Rx Message Buffer. */
    mbConfig.format = kFLEXCAN_FrameFormatStandard;
    mbConfig.type   = kFLEXCAN_FrameTypeData;
    mbConfig.id     = FLEXCAN_ID_STD(MSG1RxACTUADOR);
    FLEXCAN_SetRxMbConfig(EXAMPLE_CAN, RX_MESSAGE_BUFFER_NUM, &mbConfig, true);
    /* Start receive data through Rx Message Buffer. */
    rxACTUADOR.mbIdx = (uint8_t)RX_MESSAGE_BUFFER_NUM;
    rxACTUADOR.frame = &rxACTUADOR_Frame;

    /* Setup Tx Message Buffer. */
    FLEXCAN_SetTxMbConfig(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, true);
    /* Prepare Tx Frame for sending. */

    txKEEPALIVE_FRAME.format = (uint8_t)kFLEXCAN_FrameFormatStandard;
    txKEEPALIVE_FRAME.type   = (uint8_t)kFLEXCAN_FrameTypeData;
    txKEEPALIVE_FRAME.id     = FLEXCAN_ID_STD(MSG1TxIDKEEPALIVE);
    txKEEPALIVE_FRAME.length = (uint8_t)DLC;

    txBATTERY_FRAME.format = (uint8_t)kFLEXCAN_FrameFormatStandard;
    txBATTERY_FRAME.type   = (uint8_t)kFLEXCAN_FrameTypeData;
    txBATTERY_FRAME.id     = FLEXCAN_ID_STD(MSG1TxBATTERY);
    txBATTERY_FRAME.length = (uint8_t)DLC;

    /* Create FlexCAN handle structure and set call back function. */
    FLEXCAN_TransferCreateHandle(EXAMPLE_CAN, &flexcanHandle, flexcan_callback, NULL);
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


    /* Enter an infinite loop, just incrementing a counter. */
    while(1) {
        printf("adc_value =%d\n",ADC_value_demo());
    }
    return 0 ;
}
