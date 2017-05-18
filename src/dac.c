//
// DAC.C
// Digital to analog converter module, non-DMA, can be called from
// a timer interrupt for consistent sampling rate, etc
// Configures DAC channel 2 on PA.5
//
// Written by Ross Wolin
//

#include "stm32f4xx.h"

#include "hw.h"
#include "dac.h"


#define DAC_DHR12R1_ADDR 0x40007408
void DAC1_Config(int BLOCKSIZE, uint32_t *OutputBuffer) {
	DAC_InitTypeDef DAC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	/* DAC channel1 Configuration */
	//DAC_DeInit();
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO;
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude =
	DAC_TriangleAmplitude_2047;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);

	DMA_DeInit(DMA1_Stream5);
	DMA_InitStructure.DMA_Channel = DMA_Channel_7;
	DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR12R1_ADDR;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) OutputBuffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = BLOCKSIZE;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	//DMA_DoubleBufferModeConfig(DMA1_Stream5, (uint32_t) &OutputBuffer[1][0],
	//DMA_Memory_1);
	//DMA_DoubleBufferModeCmd(DMA1_Stream5, ENABLE);
	DMA_Init(DMA1_Stream5, &DMA_InitStructure);

	//Enable DMA1_Stream5
	DMA_Cmd(DMA1_Stream5, ENABLE);
	DAC_DMACmd(DAC_Channel_1, ENABLE);
	DAC_Cmd(DAC_Channel_1, ENABLE);

}



void DAC2_init(void)
{
   // Enable clocks for port A and DAC
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);  
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
   
   // Set up PA.5 as DAC channel 2 output
   GPIO_InitTypeDef GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_Init(GPIOA, &GPIO_InitStructure);
   
   /* DAC channel 2 Configuration */
   DAC_InitTypeDef DAC_InitStructure;
   DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
   DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
   DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
   DAC_Init(DAC_Channel_2, &DAC_InitStructure);

   /* Enable DAC Channel 2 */
   DAC_Cmd(DAC_Channel_2, ENABLE);
}



void DAC2_set(uint16_t n)
{
   if (DAC_MIN <= n  &&  n <= DAC_MAX) 
      DAC_SetChannel2Data(DAC_Align_12b_R, n);
}

