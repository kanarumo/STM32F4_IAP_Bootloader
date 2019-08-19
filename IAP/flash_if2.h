#ifndef __FLASH_IF2_H__
#define __FLASH_IF2_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_flash.h"

#include "flash_if.h"

/* Private define ------------------------------------------------------------*/
/** @addtogroup FLASH_Private_Constants
  * @{
  */
#ifndef FLASH_TIMEOUT_VALUE
  #define FLASH_TIMEOUT_VALUE       ((uint32_t)50000)/* 50 s */
#endif
#ifndef SECTOR_MASK
  #define SECTOR_MASK               ((uint32_t)0xFFFFFF07)
#endif

#ifdef USE_HAL_DRIVER
/* Exported types ------------------------------------------------------------*/
/** 
  * @brief FLASH Status  
  */ 
// FLASH_ERROR_PGS 와 같이 사용하면 hal과 중복이 된다.
// 그래서 ERROR 뒤에 _EX 를 추가한다.
typedef enum
{ 
  FLASH_EX_BUSY = 1,
  FLASH_ERROR_EX_PGS,
  FLASH_ERROR_EX_PGP,
  FLASH_ERROR_EX_PGA,
  FLASH_ERROR_EX_WRP,
  FLASH_ERROR_EX_PROGRAM,
  FLASH_ERROR_EX_OPERATION,
  FLASH_EX_COMPLETE
} FLASH_Status;

void FLASH_OB_WRPConfig(uint32_t OB_WRP, FunctionalState NewState);
uint16_t FLASH_OB_GetWRP(void);

#endif

#endif
