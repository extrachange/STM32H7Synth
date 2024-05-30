/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : 主程序，使用STM32CUBEIDE编写
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BUFFER_SIZE 64
#define SAMPLE_RATE 48000
#define VOICE_COUNT 6
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc3;

DAC_HandleTypeDef hdac1;
DMA_HandleTypeDef hdma_dac1_ch1;
DMA_HandleTypeDef hdma_dac1_ch2;

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;

UART_HandleTypeDef huart4;

/* USER CODE BEGIN PV */
uint16_t master_buffer_l[BUFFER_SIZE] = {0};	//主输出Buffer，通过DMA输出
uint16_t master_buffer_r[BUFFER_SIZE] = {0};

uint8_t UART4_rxBuffer[3] = {0};	//UART接收MIDI的Buffer
float noteToHz[128];				//MIDI音符与频率对照表，程序运行后自动生成

int envSpeed = 0;		//包络的档位
volatile uint8_t trackPlaying = 0; //表示当前播放的内置曲目, 0表示停止播放
volatile uint8_t chorusOn = 0;
uint64_t mixer = 0;			//振荡器输出混合的暂存变量
uint16_t ADC_Value;
float LFO1_Rate = 5;

volatile uint32_t trackTime = 0; //自动播放功能的时间轴
volatile uint32_t trackIndex = 0;

SineOsc9x Sine9x_0;		//复音实例化，这里犯了个错误，应该采用数组的形式存储，方便在别的文件里处理
SineOsc9x Sine9x_1;
SineOsc9x Sine9x_2;
SineOsc9x Sine9x_3;
SineOsc9x Sine9x_4;
SineOsc9x Sine9x_5;
SineOsc LFO1;
stDelay delay1;
Envelope env[VOICE_COUNT]; //包络实例化
volatile voice voicePool[VOICE_COUNT]; //复音池实例化

volatile uint8_t B3_OscVol[9]={8,0,0,0,0,0,0,0,0};
uint8_t currentPreset = 0;
volatile uint8_t oscPresets[9][9] = { //预设
		{8,0,0,0,0,0,0,0,0},{7,7,7,0,0,0,0,0,0},{0,0,7,0,3,0,2,0,0},
		{7,3,4,2,1,1,1,0,0},{3,0,5,0,3,0,2,0,5},{7,7,0,7,0,0,5,0,0},
		{0,7,6,0,6,3,0,0,3},{4,3,3,3,3,0,1,2,2},{7,8,0,7,0,0,3,0,0}};
//下方两个为内置曲目和波表，仅仅是文本替换，不是库
#include "tracksData.h"
#include "sineTables.h"

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM6_Init(void);
static void MX_UART4_Init(void);
static void MX_SPI2_Init(void);
static void MX_DAC1_Init(void);
static void MX_TIM7_Init(void);
static void MX_ADC3_Init(void);
/* USER CODE BEGIN PFP */
void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac1);
void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac1);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

void updateB3Vol();
void updateB3Freq();
void audio_frame_upper();
void audio_frame_lower();
void DSP_tick();


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void DSP_tick(int i){					//主DSP循环，每采样调用一次，将采样写入Master Buffer
			SineOsc9x_tick(&Sine9x_0);  //计算每复音输出
			SineOsc9x_tick(&Sine9x_1);
			SineOsc9x_tick(&Sine9x_2);
			SineOsc9x_tick(&Sine9x_3);
			SineOsc9x_tick(&Sine9x_4);
			SineOsc9x_tick(&Sine9x_5);

			SineOsc_setFreq(&LFO1, LFO1_Rate);	//更新LFO速率并计算
			SineOsc_tick(&LFO1);

			Envelope_tick(env, voicePool);

			mixer = 0;
			mixer += ((Sine9x_0.output * env[0].value) >> 16); //开始计算每复音包络
			mixer += ((Sine9x_1.output * env[1].value) >> 16);
			mixer += ((Sine9x_2.output * env[2].value) >> 16);
			mixer += ((Sine9x_3.output * env[3].value) >> 16);
			mixer += ((Sine9x_4.output * env[4].value) >> 16);
			mixer += ((Sine9x_5.output * env[5].value) >> 16);

			mixer = (mixer * ADC_Value ) >> 16;	  //总音量控制
			mixer = (mixer * Sine9x_0.vol_scale) >> 16; //根据开启的振荡器数量平衡音量
			if(chorusOn){ //Chorus
				mixer = ((mixer * (uint64_t)(0xC003 + (LFO1.output <<2))) >> 16); //Tremolo
				stDelay_timeMod(&delay1, 1000*((double)LFO1.output/4095.0f - 0.5));	//LFO调制振荡器以实现Chorus
				stDelay_tick(&delay1, (uint16_t)(mixer>> 16), (uint16_t)(mixer>> 16));

				uint32_t chorus_l = (3 * delay1.out_l + (uint16_t)(mixer >> 16))>>2;
				uint32_t chorus_r = (3 * delay1.out_r + (uint16_t)(mixer >> 16))>>2;

				master_buffer_l[i] = chorus_l;
				master_buffer_r[i] = chorus_r;
			}else{
				master_buffer_l[i] = (uint16_t)(mixer >> 16);
				master_buffer_r[i] = (uint16_t)(mixer >> 16);
			}
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){	//处理MIDI输入信息
	uint8_t midiBuffer[3];			//暂存本次MIDI输入，注意这里复制一份Buffer是必要的，
	midiBuffer[0] =UART4_rxBuffer[0];//目的是防止中断完成前传入了新的MIDI数据导致被覆盖
	midiBuffer[1] =UART4_rxBuffer[1];
	midiBuffer[2] =UART4_rxBuffer[2];
	HAL_UART_Receive_IT (&huart4, UART4_rxBuffer, 3);
	if ((midiBuffer[0] & 0xF0) == 0x90){
		if (midiBuffer[2] != 0){  						// 非零速度表示Note On
			voicePool_noteOn(midiBuffer[1], voicePool); //分配新音符并返回编
	    }else{  										// 0力度也可以表示Note Off
	        voicePool_noteOff(midiBuffer[1], voicePool);
	        }
	    }
	    else if ((midiBuffer[0] & 0xF0) == 0x80){	// 检查是否为Note Off消息
	    	voicePool_noteOff(midiBuffer[1], voicePool);
	    }else if((midiBuffer[0] & 0xF0) == 0xB0){ 	// 0xB0是Control Change的状态字节
	        midi_cc(midiBuffer[1], midiBuffer[2]);  // 调用处理函数，第一个字节是CC编号，第二个字节是值
	        updateB3Vol();
	    }
	updateB3Freq();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){	//用定时器播放MIDI文件，应该能用，
	//if(htim == &htim7){										//但是发现放在主循环里效果也不差
		//trackTime++;
		//if(trackTime > 1 ) HAL_GPIO_WritePin(GPIOE,GPIO_PIN_3,GPIO_PIN_SET);
		//midi_playFile_tick(track1);
	//}
}

//下方是DAC缓冲区传输的半传输和全传输中断，分别调用函数计算采样填充到缓冲区
//函数功能是一样的，只不过填充缓冲区的位置不同所以分两个函数处理
void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac1){
	audio_frame_upper();
}
void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac1){
	audio_frame_lower();
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
    ADC_Value = 0xFFFF - HAL_ADC_GetValue(&hadc3);	//我电位器反着放的所以需要反转一下
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) //处理按键和编码器输入
{
  if(GPIO_Pin == ENCODER_BTN_Pin) {
    trackPlaying++;
    if(trackPlaying >=3) trackPlaying = 0;
    voicePool_panic();
    trackTime = 0;
    trackIndex = 0;
  }else if(GPIO_Pin == BTN_RIGHT_Pin) {
	currentPreset++;
	if(currentPreset > 8){
		currentPreset -= 8;
	}
	for(int i = 0; i < 9; i++){
		B3_OscVol[i] = oscPresets[currentPreset][i];
	}
	updateB3Vol();
  }else if(GPIO_Pin == BTN_LEFT_Pin) {
	  envSpeed = !envSpeed;
	  Envelope_setSpeed(env,envSpeed);
  } else if(GPIO_Pin == BTN_MID_Pin) {
	  chorusOn = !chorusOn;
  }
}

void audio_frame_upper(){
	for (int i = 0; i < 0.5 * BUFFER_SIZE; i++)
	  {
		DSP_tick(i);
	  }
}
void audio_frame_lower(){
	for (int i = 0.5 * BUFFER_SIZE; i < BUFFER_SIZE; i++)
	  {
		DSP_tick(i);
	  }
}

void updateB3Vol(){
	SineOsc9x_setVol(&Sine9x_0, B3_OscVol);
	SineOsc9x_setVol(&Sine9x_1, B3_OscVol);
	SineOsc9x_setVol(&Sine9x_2, B3_OscVol);
	SineOsc9x_setVol(&Sine9x_3, B3_OscVol);
	SineOsc9x_setVol(&Sine9x_4, B3_OscVol);
	SineOsc9x_setVol(&Sine9x_5, B3_OscVol);
}

void updateB3Freq(){
	SineOsc9x_setFreq(&Sine9x_0, noteToHz[voicePool[0].note]);
	SineOsc9x_setFreq(&Sine9x_1, noteToHz[voicePool[1].note]);
	SineOsc9x_setFreq(&Sine9x_2, noteToHz[voicePool[2].note]);
	SineOsc9x_setFreq(&Sine9x_3, noteToHz[voicePool[3].note]);
	SineOsc9x_setFreq(&Sine9x_4, noteToHz[voicePool[4].note]);
	SineOsc9x_setFreq(&Sine9x_5, noteToHz[voicePool[5].note]);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	SineOsc9x_init(&Sine9x_0, SineLut);
	SineOsc9x_init(&Sine9x_1, SineLut);
	SineOsc9x_init(&Sine9x_2, SineLut);
	SineOsc9x_init(&Sine9x_3, SineLut);
	SineOsc9x_init(&Sine9x_4, SineLut);
	SineOsc9x_init(&Sine9x_5, SineLut);

	SineOsc_init(&LFO1, SineLut);
	Envelope_init(env);
	stDelay_init(&delay1);

	for (int note = 0; note < 128; note++) {	//计算midi和频率对应表
		noteToHz[note] = 440.0f * powf(2.0f, (note - 69) / 12.0f);
	}

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM6_Init();
  MX_UART4_Init();
  MX_SPI2_Init();
  MX_DAC1_Init();
  MX_TIM7_Init();
  MX_ADC3_Init();
  /* USER CODE BEGIN 2 */

  HAL_UART_Receive_IT (&huart4, UART4_rxBuffer, 3);
  HAL_TIM_Base_Start_IT(&htim6);

  HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*) master_buffer_l, BUFFER_SIZE, DAC_ALIGN_12B_R);
  HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_2, (uint32_t*) master_buffer_r, BUFFER_SIZE, DAC_ALIGN_12B_R);

  HAL_ADC_Start_IT(&hadc3);
  //midi_playFile(track1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(trackPlaying == 0){

	  }else if (trackPlaying == 1){
		  midi_playFile_tick(track1);
		  trackTime++;
		  updateB3Freq();
		  HAL_Delay(4);						//这个Delay影响播放速度，由于只能用毫秒所以为了精确，定时器方案会更好一些
	  }else if (trackPlaying == 2){
		  midi_playFile_tick(track2);
		  trackTime++;
		  updateB3Freq();
		  HAL_Delay(3);
	  }
	   HAL_ADC_Start_IT(&hadc3); 					//这个ADC采样率比较低，对于旋钮足够了

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CKPER;
  PeriphClkInitStruct.CkperClockSelection = RCC_CLKPSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC3_Init(void)
{

  /* USER CODE BEGIN ADC3_Init 0 */

  /* USER CODE END ADC3_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC3_Init 1 */

  /* USER CODE END ADC3_Init 1 */

  /** Common config
  */
  hadc3.Instance = ADC3;
  hadc3.Init.Resolution = ADC_RESOLUTION_16B;
  hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc3.Init.LowPowerAutoWait = DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc3.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc3.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc3.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  sConfig.OffsetSignedSaturation = DISABLE;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC3_Init 2 */

  /* USER CODE END ADC3_Init 2 */

}

/**
  * @brief DAC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC1_Init(void)
{

  /* USER CODE BEGIN DAC1_Init 0 */
	void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac1);
	void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac1);
  /* USER CODE END DAC1_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC1_Init 1 */

  /* USER CODE END DAC1_Init 1 */

  /** DAC Initialization
  */
  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_DISABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT2 config
  */
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC1_Init 2 */

  /* USER CODE END DAC1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 0x0;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi2.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi2.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi2.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi2.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi2.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi2.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi2.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi2.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi2.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 100-1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 50-1;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 240 - 1;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 5208;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 31250;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart4, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart4, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 4, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
  /* DMA1_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 4, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, CS_Pin|DC_Pin|GPIO_PIN_2, GPIO_PIN_RESET);

  /*Configure GPIO pin : PE3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : CS_Pin DC_Pin PB2 */
  GPIO_InitStruct.Pin = CS_Pin|DC_Pin|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : BTN_LEFT_Pin ENCODER_BTN_Pin BTN_RIGHT_Pin BTN_MID_Pin */
  GPIO_InitStruct.Pin = BTN_LEFT_Pin|ENCODER_BTN_Pin|BTN_RIGHT_Pin|BTN_MID_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PD12 PD13 PD0 PD1
                           PD2 PD3 PD4 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_0|GPIO_PIN_1
                          |GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
