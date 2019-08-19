/**
  ******************************************************************************
  * @file    STM32F4xx_IAP/src/menu.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    10-October-2011
  * @brief   This file provides the software which contains the main menu routine.
  *          The main menu gives the options of:
  *             - downloading a new binary file, 
  *             - uploading internal flash memory,
  *             - executing the binary file already loaded 
  *             - disabling the write protection of the Flash sectors where the 
  *               user loads his binary file.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/** @addtogroup STM32F4xx_IAP
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "flash_if.h"
#include "menu.h"
#include "ymodem.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
void SerialDownload_app(void);
void SerialDownload_data(void);

/* Private variables ---------------------------------------------------------*/
pFunction Jump_To_Application;
uint32_t JumpAddress;
__IO uint32_t FlashProtection = 0;

uint8_t tab_1024[1024] = {0,};
uint8_t FileName[FILE_NAME_LENGTH];

//user function

void jump_to_app()
{
	printf("===== BootLoader END =====\r\n\r\n");
  JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
  /* Jump to user application */
  Jump_To_Application = (pFunction) JumpAddress;
  /* Initialize user application's Stack Pointer */
  __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
  Jump_To_Application();
}

void Firmware_Loader()
{
	
	//Update File Check
	
//	uint8_t cmp_ret = 1;
//	for(uint16_t cmp_cnt = 0; cmp_cnt <=512; cmp_cnt++)//2kb compare
//		if(*(__IO uint32_t*)(APPLICATION_ADDRESS + (cmp_cnt*4)) != *(__IO uint32_t*)(TEMP_APPLICATION_ADDRESS + (cmp_cnt*4)))
//			cmp_ret = 0;
	
	if(*(__IO uint32_t*)(TEMP_APPLICATION_ADDRESS) != 0xFFFFFFFF)
	{
		FLASH_If_Erase(2,4); //OLD firmware area erase

		HAL_FLASH_Unlock();
		
		//loading
		printf("loading start...\r\n");
    for(uint16_t cnt = 0; cnt <= 32768; cnt++) //64bit(8byte) * 16384 == 128Kb
		{ 
		 HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD
											,APPLICATION_ADDRESS+(cnt*4)
											, *(__IO uint32_t*)(TEMP_APPLICATION_ADDRESS+(cnt*4)) );
		}
		/////
		
		//verify
		for(uint16_t cnt = 0; cnt <= 32768; cnt++)
			if(*(__IO uint32_t*)(APPLICATION_ADDRESS+(cnt*4)) != *(__IO uint32_t*)(TEMP_APPLICATION_ADDRESS+(cnt*4)))
			{
				printf("\r\n verifying fail.. restart.. \r\n");
				Firmware_Loader();
			}
			
		printf("\r\n verifying success! \r\n");
		/////
		
		HAL_FLASH_Lock();
		
		FLASH_If_Erase(6,1); //Update file delete
		jump_to_app();
	}
	else
	{
		jump_to_app();
	}
}
    

/**
  * @brief  Download a file via serial port
  * @param  None
  * @retval None
  */
void SerialDownload_data()
{
  int32_t Size = 0;
	
	printf("2. data area write (0x%08x)\n\r",DATA_ADDRESS);
  printf("Waiting for the file to be sent... (press 'a' to abort)\n\r");
  Size = Ymodem_Receive_data(&tab_1024[0]);
  if (Size > 0)
  {
    printf("\r\n Programming Completed Successfully!\r\n--\r\n Name: %s",FileName);
    printf("\r\n Size: %d Bytes\r\n",Size);
    printf("-------------------\r\n");
  }
  else if (Size == -1)
  {
    printf("\r\n\r\nThe image size is higher than the allowed space memory!\r\n");
  }
  else if (Size == -2)
  {
    printf("\r\n\r\nVerification failed!\r\n");
  }
  else if (Size == -3)
  {
    printf("\r\n\r\n\r\nAborted by user.\r\n");
  }
  else if (Size == -4)
  {
    printf("   BOOTLOADER_TIMEOUT....\r\n");
  }
  else
  {
    printf("\r\n\r\nFailed to receive the file!\r\n");
  }
}

void SerialDownload_app()
{
  int32_t Size = 0;

	//printf("1. application area write (0x%08x)\n\r",APPLICATION_ADDRESS);
	printf("1. application area write (0x%08x)\n\r",TEMP_APPLICATION_ADDRESS);
  printf("Waiting for the file to be sent... (press 'a' to abort)\n\r");
	
  Size = Ymodem_Receive_app(&tab_1024[0]);
  if (Size > 0)
  {
    printf("\r\n Programming Completed Successfully!\r\n--\r\n Name: %s",FileName);
    printf("\r\n Size: %d Bytes\r\n",Size);
    printf("-------------------\r\n");
  }
  else if (Size == -1)
  {
    printf("\r\n\r\nThe image size is higher than the allowed space memory!\r\n");
  }
  else if (Size == -2)
  {
    printf("\r\n\r\nVerification failed!\r\n");
  }
  else if (Size == -3)
  {
    printf("\r\n\r\n\r\nAborted by user.\r\n");
  }
  else if (Size == -4)
  {
    printf("   BOOTLOADER_TIMEOUT....\r\n");
  }
  else
  {
    printf("\r\n\r\nFailed to receive the file!\r\n");
  }
}

/**
  * @brief  Display the Main Menu on HyperTerminal
  * @param  None
  * @retval None
  */
void Main_Menu(void)
{
	while(1)
	{
		volatile uint8_t key = 0;
		
		printf("\r\n=============================");
		printf("\r\n    RM-TECH BOOTLOADER V14   ");
		printf("\r\n=============================");
		printf("\r\n       0. Fast execute       ");
		printf("\r\n       1. App write          ");
		printf("\r\n       2. Firm Update        ");
		printf("\r\n       3. All Clear          ");
		printf("\r\n=============================\r\n");

		key = GetKey();

		if (key == 0x30) // '0'
			jump_to_app();

		else if (key == 0x31) // '1'
			SerialDownload_app();
		
//		else if (key == 0x32) // '2'
//			SerialDownload_data();
		
		else if (key == 0x32) // '2'
			Firmware_Loader();

		else if (key == 0x33) // '3'
		{
			printf("Sector 2 ~ 11 erasing..\r\n");
			FLASH_If_Erase(2,10);
			printf("done! \r\n");
		}
	}
}


/**
  * @}
  */

/*******************(C)COPYRIGHT 2011 STMicroelectronics *****END OF FILE******/
