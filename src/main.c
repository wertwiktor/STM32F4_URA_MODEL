#include "stm32f4_discovery.h"
#include "stm32f4xx.h"
#include <stdio.h>
#include "dac.h"

double a = 0.0f;
double b = 0;
double c = 0;
double d = 0;

double a1, a2, a3, b0, b1, b2, b3;
double x, y;
double w[4] = { 0, 0, 0, 0 };

void RCC_Configuration(void) {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
}

void SetupLEDS(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14
			| GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
}

void SetupADC1_CH6(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	/* DAC channel 1 & 2 (DAC_OUT1 = PA.4)(DAC_OUT2 = PA.5) configuration */
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void ADC_Configuration(void) {

	ADC_InitTypeDef ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;

	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge =
	ADC_ExternalTrigConvEdge_Rising;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_15Cycles);

	/* Enable DMA request after last transfer (Single-ADC mode) */
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

	/* Enable ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);

	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);
}

#define BLOCKSIZE 1
uint32_t ADC_ConvertedValue[2][BLOCKSIZE];
uint32_t OutputBuffer[2][BLOCKSIZE];
#define ADC1_DR_ADDRESS     ((uint32_t)0x4001204C);

static void DMA_Configuration(void) {
	DMA_InitTypeDef DMA_InitStructure;
	DMA_DeInit(DMA2_Stream0);
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;
	DMA_InitStructure.DMA_Memory0BaseAddr =
			(uint32_t) &ADC_ConvertedValue[0][0];
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &ADC1->DR;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = BLOCKSIZE; // Count of 16-bit words
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
	DMA_DoubleBufferModeConfig(DMA2_Stream0,
			(uint32_t) &ADC_ConvertedValue[1][0], DMA_Memory_1);
	DMA_DoubleBufferModeCmd(DMA2_Stream0, ENABLE);
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);

	DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);

	DMA_Cmd(DMA2_Stream0, ENABLE);
}

void NVIC_Configuration(void) {
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void TIM2_Configuration(void) {
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	//TIM_TimeBaseStructure.TIM_Period = 1749;  // 1 MHz down to 10 KHz (0.1 ms)
	//TIM_TimeBaseStructure.TIM_Prescaler = 24 - 1; // 24 MHz Clock down to 1 MHz (adjust per your clock)
	//TIM_TimeBaseStructure.TIM_ClockDivision = 1;
	TIM_TimeBaseStructure.TIM_Period = (84000000 / 200) - 1; // 200 KHz, from 84 MHz TIM2CLK (ie APB1 = HCLK/4, TIM2CLK = HCLK/2)
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update); // ADC_ExternalTrigConv_T2_TRGO
	TIM_Cmd(TIM2, ENABLE);
}

#define DAC_DHR12R1_ADDR 0x40007408
void DAC1_Config1111(void) {
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
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) &OutputBuffer[0][0];
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

void DMA2_Stream0_IRQHandler(void) // Called at 1 KHz for 200 KHz sample rate, LED Toggles at 500 Hz
{

	if (DMA_GetITStatus(DMA2_Stream0, DMA_IT_HTIF0)) {
		DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_HTIF0);
		GPIO_ToggleBits(GPIOD, GPIO_Pin_13);
	}

	y = (b0 * x + w[0]) / (b + c + d + 1);
	w[0] = b1 * x + w[1] - a1 * y;
	w[1] = b2 * x + w[2] - a2 * y;
	w[2] = b3 * x - a3 * y;

	OutputBuffer[0][0] = y;
	//OutputBuffer[1][0] = 4095;
	if (DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0)) {
		DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);
		GPIO_ToggleBits(GPIOD, GPIO_Pin_13);
	}
	GPIO_ToggleBits(GPIOD, GPIO_Pin_15);
}

int main(void) {

	RCC_Configuration();

	SetupLEDS();

	SetupADC1_CH6();

	NVIC_Configuration();

	TIM2_Configuration();

	DAC1_Config(1,&OutputBuffer[0][0]);

	DMA_Configuration();

	ADC_Configuration();


	SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2));

	double Tp = 0.01f;
	double T1 = 1;
	double T2 = 3;
	double zeta = 0.3;
	double k = 1;

	a = 2 / Tp;
	b = a * a * a * T1 * T2 * T2;
	c = a * a * (T2 * T2 + 2 * zeta * T1 * T2);
	d = a * (T1 + 2 * zeta * T2);

	a1 = (3 + d - c - 3 * b);
	a2 = (3 - d - c + 3 * b);
	a3 = (1 - d + c - b);

	b0 = k;
	b1 = 3 * k;
	b2 = 3 * k;
	b3 = k;
	x = 2000;

	while (1) {
		//OutputBuffer[0][0] = 4095;
		//OutputBuffer[1][0] = 0;
	}
}
