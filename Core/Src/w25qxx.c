#include <stdio.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "w25qxx.h"
#include "menu.h"
////Ext Mem Size define////
#define PACKET_SIZE 			64

#define IDX_START_ADDR		0x0
#define	IDX_END_ADDR		0xFFF		//4Kb

#define START_ADDR			0x1000
#define END_ADDR				0xFFFFF //1024-(4)KB

///////////////////////////


static SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart5;

#define W25QXX_DUMMY_BYTE 0xA5

#define	W25qxx_Delay(delay)		for(volatile uint32_t a=0; a<delay*10; a++) //HAL_Delay(delay)

#define FLASH_CS_GPIO_Port GPIOA
#define FLASH_CS_Pin GPIO_PIN_4

#define		_W25QXX_SPI										hspi1
#define		_W25QXX_CS_GPIO								FLASH_CS_GPIO_Port
#define		_W25QXX_CS_PIN								FLASH_CS_Pin
#define		_W25QXX_USE_FREERTOS					0
#define		_W25QXX_DEBUG									1

w25qxx_t	w25qxx;

//###################################################################################################################
int fputc(int ch, FILE *f)
{
		HAL_UART_Transmit(&huart5, (uint8_t *)&ch, 1, 2);
		return ch;
}

extern void hSPI1_Init(void)
{
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_HARD_OUTPUT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
	
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
		
	}
}

//###################################################################################################################
uint8_t	W25qxx_Spi(uint8_t	Data)
{
	uint8_t	ret;
	HAL_SPI_TransmitReceive(&_W25QXX_SPI,&Data,&ret,1,10);
	return ret;	
}
//###################################################################################################################
uint32_t W25qxx_ReadID(void)
{
  uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;
  HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
  W25qxx_Spi(0x9F);
  Temp0 = W25qxx_Spi(W25QXX_DUMMY_BYTE);
  Temp1 = W25qxx_Spi(W25QXX_DUMMY_BYTE);
  Temp2 = W25qxx_Spi(W25QXX_DUMMY_BYTE);
  HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
  Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;
  return Temp;
}
//###################################################################################################################
void W25qxx_ReadUniqID(void)
{
  HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
  W25qxx_Spi(0x4B);
	for(uint8_t	i=0;i<4;i++)
		W25qxx_Spi(W25QXX_DUMMY_BYTE);
	for(uint8_t	i=0;i<8;i++)
		w25qxx.UniqID[i] = W25qxx_Spi(W25QXX_DUMMY_BYTE);
  HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
}
//###################################################################################################################
void W25qxx_WriteEnable(void)
{
  HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
  W25qxx_Spi(0x06);
  HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
	W25qxx_Delay(1);
}
//###################################################################################################################
void W25qxx_WriteDisable(void)
{
  HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
  W25qxx_Spi(0x04);
  HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
	W25qxx_Delay(1);
}
//###################################################################################################################
uint8_t W25qxx_ReadStatusRegister(uint8_t	SelectStatusRegister_1_2_3)
{
	uint8_t	status=0;
  HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
	if(SelectStatusRegister_1_2_3==1)
	{
		W25qxx_Spi(0x05);
		status=W25qxx_Spi(W25QXX_DUMMY_BYTE);	
		w25qxx.StatusRegister1 = status;
	}
	else if(SelectStatusRegister_1_2_3==2)
	{
		W25qxx_Spi(0x35);
		status=W25qxx_Spi(W25QXX_DUMMY_BYTE);	
		w25qxx.StatusRegister2 = status;
	}
	else
	{
		W25qxx_Spi(0x15);
		status=W25qxx_Spi(W25QXX_DUMMY_BYTE);	
		w25qxx.StatusRegister3 = status;
	}	
  HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
	return status;
}
//###################################################################################################################
void W25qxx_WriteStatusRegister(uint8_t	SelectStatusRegister_1_2_3,uint8_t Data)
{
  HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
	if(SelectStatusRegister_1_2_3==1)
	{
		W25qxx_Spi(0x01);
		w25qxx.StatusRegister1 = Data;
	}
	else if(SelectStatusRegister_1_2_3==2)
	{
		W25qxx_Spi(0x31);
		w25qxx.StatusRegister2 = Data;
	}
	else
	{
		W25qxx_Spi(0x11);
		w25qxx.StatusRegister3 = Data;
	}
	W25qxx_Spi(Data);
  HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
}
//###################################################################################################################
void W25qxx_WaitForWriteEnd(void)
{
	W25qxx_Delay(1);
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
	W25qxx_Spi(0x05);
  do
  {
    w25qxx.StatusRegister1 = W25qxx_Spi(W25QXX_DUMMY_BYTE);
		W25qxx_Delay(1);
  }
  while ((w25qxx.StatusRegister1 & 0x01) == 0x01);
 HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
}
//###################################################################################################################
bool	W25qxx_Init(void)
{
	w25qxx.Lock=1;	
	while(HAL_GetTick()<100)
		W25qxx_Delay(1);
	uint32_t	id = 0;
	#if (_W25QXX_DEBUG==1)
	//printf("w25qxx Init Begin...\r\n");
	#endif
	id=W25qxx_ReadID();
	
	#if (_W25QXX_DEBUG==1)
	//printf("w25qxx ID:0x%X\r\n",id);
	#endif
	switch(id & 0x0000FFFF)
	{
		case 0x4018:	// 	w25q128
			w25qxx.ID=W25Q128;
			w25qxx.BlockCount=256;
			#if (_W25QXX_DEBUG==1)
			//printf("w25qxx Chip: w25q128\r\n");
			#endif
		break;
		case 0x4014:	//	w25q80
			w25qxx.ID=W25Q80;
			w25qxx.BlockCount=16;
			#if (_W25QXX_DEBUG==1)
			//printf("w25qxx Chip: w25q80\r\n");
			#endif
		break;
		default:
				#if (_W25QXX_DEBUG==1)
				printf("w25qxx Unknown ID\r\n");
				#endif
			w25qxx.Lock=0;	
			return false;
				
	}		
	w25qxx.PageSize=256;
	w25qxx.SectorSize=0x1000;
	w25qxx.SectorCount=w25qxx.BlockCount*16;
	w25qxx.PageCount=(w25qxx.SectorCount*w25qxx.SectorSize)/w25qxx.PageSize;
	w25qxx.BlockSize=w25qxx.SectorSize*16;
	w25qxx.CapacityInKiloByte=(w25qxx.SectorCount*w25qxx.SectorSize)/1024;
	W25qxx_ReadUniqID();
	W25qxx_ReadStatusRegister(1);
	W25qxx_ReadStatusRegister(2);
	W25qxx_ReadStatusRegister(3);
	#if (_W25QXX_DEBUG==1)
//	printf("w25qxx Page Size: %d Bytes\r\n",w25qxx.PageSize);
//	printf("w25qxx Page Count: %d\r\n",w25qxx.PageCount);
//	printf("w25qxx Sector Size: %d Bytes\r\n",w25qxx.SectorSize);
//	printf("w25qxx Sector Count: %d\r\n",w25qxx.SectorCount);
//	printf("w25qxx Block Size: %d Bytes\r\n",w25qxx.BlockSize);
//	printf("w25qxx Block Count: %d\r\n",w25qxx.BlockCount);
//	printf("w25qxx Capacity: %d KB\r\n",w25qxx.CapacityInKiloByte);
//	printf("w25qxx Init Done\r\n");
	#endif
	w25qxx.Lock=0;	
	return true;
}	
//###################################################################################################################
void	W25qxx_EraseChip(void)
{
	while(w25qxx.Lock==1)
		W25qxx_Delay(1);
	w25qxx.Lock=1;	
	#if (_W25QXX_DEBUG==1)
	uint32_t	StartTime=HAL_GetTick();	
	printf("w25qxx EraseChip Begin...\r\n");
	#endif
	W25qxx_WriteEnable();
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
  W25qxx_Spi(0xC7);
  HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
	W25qxx_WaitForWriteEnd();
	#if (_W25QXX_DEBUG==1)
	printf("w25qxx EraseChip done after %d ms!\r\n",HAL_GetTick()-StartTime);
	#endif
	W25qxx_Delay(10);
	w25qxx.Lock=0;	
}
//###################################################################################################################
void W25qxx_EraseSector(uint32_t SectorAddr)
{
	while(w25qxx.Lock==1)
		W25qxx_Delay(1);
	w25qxx.Lock=1;	
	#if (_W25QXX_DEBUG==1)
	//uint32_t	StartTime=HAL_GetTick();	
	//printf("[ERASE] SECTOR %d\r\n",SectorAddr);
	#endif
	W25qxx_WaitForWriteEnd();
	SectorAddr = SectorAddr * w25qxx.SectorSize;
  W25qxx_WriteEnable();
  HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
  W25qxx_Spi(0x20);
	if(w25qxx.ID>=W25Q256)
		W25qxx_Spi((SectorAddr & 0xFF000000) >> 24);
  W25qxx_Spi((SectorAddr & 0xFF0000) >> 16);
  W25qxx_Spi((SectorAddr & 0xFF00) >> 8);
  W25qxx_Spi(SectorAddr & 0xFF);
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
  W25qxx_WaitForWriteEnd();
//	#if (_W25QXX_DEBUG==1)
//	printf("[ERASE] SECTOR : %d	COST : %d\r\n", SectorAddr, HAL_GetTick()-StartTime);
//	#endif
	W25qxx_Delay(1);
	w25qxx.Lock=0;
}
//###################################################################################################################
void W25qxx_EraseBlock(uint32_t BlockAddr)
{
	while(w25qxx.Lock==1)
		W25qxx_Delay(1);
	w25qxx.Lock=1;	
	#if (_W25QXX_DEBUG==1)
	printf("w25qxx EraseBlock %d Begin...\r\n",BlockAddr);
	W25qxx_Delay(100);
	uint32_t	StartTime=HAL_GetTick();	
	#endif
	W25qxx_WaitForWriteEnd();
	BlockAddr = BlockAddr * w25qxx.SectorSize*16;
  W25qxx_WriteEnable();
  HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
  W25qxx_Spi(0xD8);
	if(w25qxx.ID>=W25Q256)
		W25qxx_Spi((BlockAddr & 0xFF000000) >> 24);
  W25qxx_Spi((BlockAddr & 0xFF0000) >> 16);
  W25qxx_Spi((BlockAddr & 0xFF00) >> 8);
  W25qxx_Spi(BlockAddr & 0xFF);
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
  W25qxx_WaitForWriteEnd();
	#if (_W25QXX_DEBUG==1)
	printf("w25qxx EraseBlock done after %d ms\r\n",HAL_GetTick()-StartTime);
	W25qxx_Delay(100);
	#endif
	W25qxx_Delay(1);
	w25qxx.Lock=0;
}

//###################################################################################################################
void W25qxx_WriteByte(uint8_t pBuffer, uint32_t WriteAddr_inBytes)
{
	while(w25qxx.Lock==1)
		W25qxx_Delay(1);
	w25qxx.Lock=1;
	
	#if (_W25QXX_DEBUG==1)
	uint32_t	StartTime=HAL_GetTick();
	printf("w25qxx WriteByte 0x%02X at address %x begin...",pBuffer,WriteAddr_inBytes);
	#endif
	
	W25qxx_WaitForWriteEnd();
  W25qxx_WriteEnable();
  HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
	
  W25qxx_Spi(0x02);
	if(w25qxx.ID>=W25Q256)
		W25qxx_Spi((WriteAddr_inBytes & 0xFF000000) >> 24);
	
  W25qxx_Spi((WriteAddr_inBytes & 0xFF0000) >> 16);
  W25qxx_Spi((WriteAddr_inBytes & 0xFF00) >> 8);
  W25qxx_Spi(WriteAddr_inBytes & 0xFF);
  W25qxx_Spi(pBuffer);
	
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
  W25qxx_WaitForWriteEnd();
	
	#if (_W25QXX_DEBUG==1)
	//printf("[WRITE] ADDR : %d	COST : %d\r\n", WriteAddr_inBytes, HAL_GetTick()-StartTime);
	printf("w25qxx WriteByte done after %d ms\r\n",HAL_GetTick()-StartTime);
	#endif
	
	w25qxx.Lock=0;
}
//###################################################################################################################
void W25qxx_WriteBytes(uint8_t *pBuffer, uint32_t Addr, uint32_t len)
{
	while(w25qxx.Lock==1)
		W25qxx_Delay(1);
	w25qxx.Lock=1;
	
	#if (_W25QXX_DEBUG==0)
	uint32_t	StartTime=HAL_GetTick();
	#endif
	
	W25qxx_WaitForWriteEnd();
	W25qxx_WriteEnable();
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
	
	W25qxx_Spi(0x02);
	if(w25qxx.ID>=W25Q256)
		W25qxx_Spi((Addr & 0xFF000000) >> 24);
	
	W25qxx_Spi((Addr & 0xFF0000) >> 16);
	W25qxx_Spi((Addr & 0xFF00) >> 8);
	W25qxx_Spi(Addr & 0xFF);
	
	for(int num=0; num < len; num++)
		W25qxx_Spi(pBuffer[num]);
	
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
	W25qxx_WaitForWriteEnd();
	
	#if (_W25QXX_DEBUG==0)
	printf("[WRITE] ADDR : %d	COST : %d\r\n",Addr, HAL_GetTick()-StartTime);
	#endif
	
	w25qxx.Lock=0;
}
//###################################################################################################################
void 	W25qxx_ReadByte(uint8_t *pBuffer,uint32_t Bytes_Address)
{
	while(w25qxx.Lock==1)
		W25qxx_Delay(1);
	w25qxx.Lock=1;
	#if (_W25QXX_DEBUG==1)
	uint32_t	StartTime=HAL_GetTick();
	#endif
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
  W25qxx_Spi(0x0B);
	if(w25qxx.ID>=W25Q256)
		W25qxx_Spi((Bytes_Address & 0xFF000000) >> 24);
  W25qxx_Spi((Bytes_Address & 0xFF0000) >> 16);
  W25qxx_Spi((Bytes_Address& 0xFF00) >> 8);
  W25qxx_Spi(Bytes_Address & 0xFF);
	W25qxx_Spi(0);
	*pBuffer = W25qxx_Spi(W25QXX_DUMMY_BYTE);
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);	
//	#if (_W25QXX_DEBUG==1)
//	printf("[READ] ADDR : %d	DATA : %d\r\n", Bytes_Address, *pBuffer);
//	#endif
	w25qxx.Lock=0;
}
//###################################################################################################################
void W25qxx_ReadBytes(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead)
{
	while(w25qxx.Lock==1)
		W25qxx_Delay(1);
	w25qxx.Lock=1;
	
	#if (_W25QXX_DEBUG==1)
	printf("[READ] ADDR : %d	BYTE : %d\r\n", ReadAddr,NumByteToRead);
	#endif	
	
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
	W25qxx_Spi(0x0B);
	if(w25qxx.ID>=W25Q256)
		W25qxx_Spi((ReadAddr & 0xFF000000) >> 24);
  W25qxx_Spi((ReadAddr & 0xFF0000) >> 16);
  W25qxx_Spi((ReadAddr& 0xFF00) >> 8);
  W25qxx_Spi(ReadAddr & 0xFF);
	W25qxx_Spi(0);
	HAL_SPI_Receive(&_W25QXX_SPI,pBuffer,NumByteToRead,2000);	
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
	
	#if (_W25QXX_DEBUG==1)
		for(uint32_t i=0;i<NumByteToRead ; i++)
	{
		if((i%8==0)&&(i>2))
		{
			printf("\r\n");
		}
		printf("0x%02X ",pBuffer[i]);		
	}
	printf("\r\n");
	
	#endif	
	
	W25qxx_Delay(1);
	w25qxx.Lock=0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////						USER Funtion 					//////////////////////////////////HJ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t counter (void)
{
	uint8_t index_buf	[2];
	uint16_t	data_count		= 0; //data count num
	
	//index sector read //not print log
	W25qxx_ReadByte(index_buf		, 0x0);
	W25qxx_ReadByte(index_buf+1	, 0x1);
	
	//data division
	data_count = index_buf[0] << 8 | index_buf [1];

	return data_count;
}

uint16_t counter_and_control (void)
{
	uint8_t index_buf	[3];
	
	uint16_t	data_count		= 0; //data num
	uint8_t 	cycle_count		=	0; //flash cycle num
	
	//index sector read and erase
	W25qxx_ReadBytes(index_buf,0x0,3);
	W25qxx_EraseSector(0x0);
	
	//data division
	data_count = index_buf[0] << 8 | index_buf [1];
	cycle_count = index_buf[2];
	
	//detector
	if(data_count == 0xFFFF) //init detect
	{
		printf("CNT_CLS...\r\n");
		data_count = 0;
	}
	else if (data_count > 0x3FC0)//count end detect //0x3FC0 == 16320
	{
		printf("CNT_OBF...\r\n");
		data_count = 0;
		cycle_count++;
		
		W25qxx_EraseChip();
	}
	else if (cycle_count == 0xFF)//cycle end detect
	{
		printf("Cycle_0xFF...\r\n");
		cycle_count = 0;
	}
	
	printf("COUNT : %d	Cycle : %d\r\n", data_count, cycle_count);
	
	data_count++;
	
	//merge
	index_buf[0] = data_count >> 8; 
	index_buf[1] = data_count;			
	index_buf[2] = cycle_count;
	
	//write
	W25qxx_WriteBytes(index_buf,0x0,3);

	return data_count;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void W25qxx_WriteBytes_64 (uint8_t* data)
{
	uint16_t count = 0;
	
	printf("--------------\r\n");
	
	count = counter_and_control();
	
	W25qxx_WriteBytes(data,0x1000+(64*(count-1)),64);
	W25qxx_ReadBytes(data,0x1000+(64*(count-1)),64);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void password (void)
{
	uint8_t key[6] = "rmtech";
	uint8_t input[6];
	
	printf("\r\nPASSWORD :");

	HAL_UART_Receive(&huart5,input,6,10000);

	if (strcmp((char*)key, (char*)input) == 0)
		W25qxx_ReadBytes_all();
	else
		printf("\r\nFAIL\r\n");

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void W25qxx_ReadBytes_all (void)
{
	
	uint8_t data [2048]; //2kb
	uint32_t address = 0;
	uint32_t time = 0;

	printf("\r\n=====MEMORY_ALL_READ=====\r\n");
	printf("S/N : %lli \r\n",103190300001);
	printf("=========================\r\n");
	
	time = HAL_GetTick();

	for(uint16_t j=0; j<512; j++) // (2kb * 512) == 1MB
	{
		address = j*2048;

		while(w25qxx.Lock==1)
			W25qxx_Delay(1);
			
		w25qxx.Lock=1;
		
		HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
		W25qxx_Spi(0x0B);

		W25qxx_Spi((address & 0xFF0000) >> 16);
		W25qxx_Spi((address & 0xFF00) >> 8);
		W25qxx_Spi(address & 0xFF);
		W25qxx_Spi(0);
			
		HAL_SPI_Receive(&_W25QXX_SPI,data,2048,20000);// read 2048byte = read 2kb
		HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
		
		for(uint16_t i=0; i < 2048; i++)
		{
			if(i%64==0)
				printf("\r\n");
			
			printf("%02X ",data[i]);
		}
		
		W25qxx_Delay(1);
		w25qxx.Lock=0;
	}
	
	printf("\r\n=====MEMORY_READ_END=====\r\n");
	printf("time : %d ms\r\n",HAL_GetTick() - time);
	Main_Menu();
}



