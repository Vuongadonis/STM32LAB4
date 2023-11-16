/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
typedef struct {
	//Pointer to the task
	void (*pTask)(void);
	//Delay (ticks) until the function will (next) be run
	uint32_t Delay;
	//Interval (ticks) between subsequent runs
	uint32_t Period;
	//Incremented (by scheduler) when task is due to execute
	uint8_t RunMe;
	//This is a hint to solve the question below
	uint32_t TaskID;
}sTask;

//MUST BE ADJUSTED FOR EACH NEW PROJECT
#define SCH_MAX_TASKS 40
#define NO_TASK_ID 0
sTask SCH_tasks_G[SCH_MAX_TASKS];

//Ham giam dan thoi gian delay va set RunMe len de execute
void SCH_Update(void){
	uint8_t Index = 0;
	//Kiem tra tung task
	for(Index = 0; Index < SCH_MAX_TASKS; Index++){
		//kiem tra xem co task khong
		if(SCH_tasks_G[Index].pTask){
			//neu task het thoi gian delay
			if(SCH_tasks_G[Index].Delay == 0){
				//tang bien RunMe len de execute task
				SCH_tasks_G[Index].RunMe += 1;
				//Kiem tra co phai "one shot" task, neu khong phai thi gan lai Delay
				if(SCH_tasks_G[Index].Period > 0){
					SCH_tasks_G[Index].Delay = SCH_tasks_G[Index].Period;
				}
			}else{
				//van con thoi gian delay
				SCH_tasks_G[Index].Delay--;
			}
		}
	}
}

void SCH_Update2(void){
	//kiem tra task dau tien
	if(SCH_tasks_G[0].pTask){
		if(SCH_tasks_G[0].Delay == 0){
			SCH_tasks_G[0].RunMe = 1;
		}else{
			SCH_tasks_G[0].Delay--;
		}
	}
}

//Ham them task vao mang
uint32_t SCH_Add_Task(void (*pFunction)(), uint32_t DELAY, uint32_t PERIOD){
	uint8_t Index = 0;
	//First find a gap in the array (if there is one)
	while((SCH_tasks_G[Index].pTask != 0) && (Index < SCH_MAX_TASKS))
	{
		Index++;
	}
	//Have we reached the end of the list?
	if(Index == SCH_MAX_TASKS){
//		//Task list is full
//		//Set the global error variable
//		Error_code_G = ERROR_SCH_TOO_MANY_TASKS;
//		//also return an error code
//		return SCh_MAX_TASKS;
		return 0;
	}
	//if we're here, there is a space in the task array
	SCH_tasks_G[Index].pTask = pFunction;
	SCH_tasks_G[Index].Delay = DELAY/10;
	SCH_tasks_G[Index].Period = PERIOD/10;
	SCH_tasks_G[Index].RunMe = 0;
	//return position of task (to allow later deletion)
	return Index;
}

uint32_t SCH_Add_Task2(void (*pFunction)(), uint32_t DELAY, uint32_t PERIOD){
	uint8_t Index = 0;
	//bien de dich chuyen phan tu cua mang qua 1 don vi
	uint8_t move = 0;
	uint8_t delayDcycle = DELAY/10;
	//tong delay cua cac task (de so sanh voi delay cua task moi )
	uint16_t total = 0;
	//First find a gap in the array (if there is one)
	while((SCH_tasks_G[Index].pTask != 0) && (Index < SCH_MAX_TASKS))
	{
		if(total + SCH_tasks_G[Index].Delay > delayDcycle){
			break;
		}
		total = total + SCH_tasks_G[Index].Delay;
		Index++;
	}
	//Have we reached the end of the list?
	if(Index == SCH_MAX_TASKS){
		return 0;
	}
	//if we're here, there is a space in the task array
	if(SCH_tasks_G[Index].pTask){
		for(move = SCH_MAX_TASKS - 1; move > Index; move--){
			SCH_tasks_G[move].pTask = SCH_tasks_G[move-1].pTask;
			SCH_tasks_G[move].Delay = SCH_tasks_G[move-1].Delay;
			SCH_tasks_G[move].Period = SCH_tasks_G[move-1].Period;
			SCH_tasks_G[move].RunMe = SCH_tasks_G[move-1].RunMe;
		}
	}
	//add new task into array
	SCH_tasks_G[Index].pTask = pFunction;
	SCH_tasks_G[Index].Delay = delayDcycle - total;
	SCH_tasks_G[Index].Period = PERIOD;
	SCH_tasks_G[Index].RunMe = 0;
	//update delay of task after new task
	if(SCH_tasks_G[Index+1].pTask){
		SCH_tasks_G[Index+1].Delay = SCH_tasks_G[Index+1].Delay - SCH_tasks_G[Index].Delay;
	}
	//return position of task (to allow later deletion)
	return Index;
}

//Xoa task duoc chi dinh
void SCH_Delete_Task(uint32_t taskID){
	if(SCH_tasks_G[taskID].pTask == 0){
		//No task at this location...
		//
		//Set the global error variable
		//...also return an error code
		//return 0;
		return;
	}
	SCH_tasks_G[taskID].pTask = 0x0000;
	SCH_tasks_G[taskID].Delay = 0;
	SCH_tasks_G[taskID].Period = 0;
	SCH_tasks_G[taskID].RunMe = 0;
	//return 0;
}

void SCH_Delete_Task2(){
	uint8_t Index;
	for(Index = 0; Index < SCH_MAX_TASKS-1; Index++){
		if(SCH_tasks_G[Index+1].pTask){
			SCH_tasks_G[Index].pTask = SCH_tasks_G[Index+1].pTask;
			SCH_tasks_G[Index].Delay = SCH_tasks_G[Index+1].Delay;
			SCH_tasks_G[Index].Period = SCH_tasks_G[Index+1].Period;
			SCH_tasks_G[Index].RunMe = SCH_tasks_G[Index+1].RunMe;
		}
		if(SCH_tasks_G[Index].pTask == 0x0000){
			SCH_Delete_Task(Index);
			break;
		}
	}
}

//Ham check xem co task nao can execute chua
void SCH_Dispatch_Tasks(void){
	uint8_t Index;
	//Dispatches (runs) the next task (if one is ready)
	for(Index = 0; Index < SCH_MAX_TASKS; Index++){
		if(SCH_tasks_G[Index].RunMe > 0){
			(*SCH_tasks_G[Index].pTask)(); //Run the task
			SCH_tasks_G[Index].RunMe -= 1; //Reset/Reduce RunMe flag
			//Periodic tasks will automatically run again
			//-if this is a 'one shot' task, remove it from the array
			if(SCH_tasks_G[Index].Period == 0){
				SCH_Delete_Task(Index);
			}
		}
	}
}

void SCH_Dispatch_Tasks2(void){
	//Dispatches (runs) the next task (if one is ready)
	if(SCH_tasks_G[0].pTask){
		if(SCH_tasks_G[0].RunMe > 0){
			(*SCH_tasks_G[0].pTask)();
			SCH_tasks_G[0].RunMe--;
			if(SCH_tasks_G[0].Period > 0){
				SCH_Add_Task2(SCH_tasks_G[0].pTask, SCH_tasks_G[0].Period, SCH_tasks_G[0].Period);
			}
			SCH_Delete_Task2();
		}
	}
}

//Ham xoa tat ca cac task trong array, khien array nhu vua duoc khoi tao
void SCH_Init(void){
	uint8_t i;
	for(i=0; i<SCH_MAX_TASKS; i++){
		SCH_Delete_Task(i);
	}
	//Reset the global error variable
	//-SCH_Delete_Task() will generate an error code
	//(because the task array is empty)
//	Error_code_G = 0;
//	Timer_init();
//	Watchdog_init();
}

void toggleRED(){
	HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
}

void toggleGREEN(){
	HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
}

void toggleYELLOW(){
	HAL_GPIO_TogglePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin);
}

void turnOnLED(){
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, RESET);
}

void turnOffLED(){
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, SET);
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  SCH_Init();
  SCH_Add_Task2(toggleRED, 1000, 1000);
  SCH_Add_Task2(toggleGREEN, 1250, 1000);
  SCH_Add_Task2(toggleYELLOW, 1500, 1000);
//  SCH_Add_Task2(turnOnLED, 2000, 2000);
//  SCH_Add_Task2(turnOffLED, 2000, 2500);
  SCH_Add_Task2(SCH_tasks_G[0].pTask, SCH_tasks_G[0].Period, SCH_tasks_G[0].Period);
//  SCH_Delete_Task2();
  while (1)
  {
	  SCH_Dispatch_Tasks2();
//	  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
//	  SCH_tasks_G[0].pTask();
//	  SCH_tasks_G[1].pTask();
//	  SCH_tasks_G[2].pTask();
//	  HAL_Delay(500);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

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
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED_Pin|LED_GREEN_Pin|LED_RED_Pin|LED_YELLOW_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED_Pin LED_GREEN_Pin LED_RED_Pin LED_YELLOW_Pin */
  GPIO_InitStruct.Pin = LED_Pin|LED_GREEN_Pin|LED_RED_Pin|LED_YELLOW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	SCH_Update2();
}
/* USER CODE END 4 */

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