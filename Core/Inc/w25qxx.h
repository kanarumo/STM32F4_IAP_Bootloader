#ifndef _W25QXX_H
#define _W25QXX_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
 extern "C" {
#endif
	 
typedef enum
{
	W25Q10=1,
	W25Q20,
	W25Q40,
	W25Q80,
	W25Q16,
	W25Q32,
	W25Q64,
	W25Q128,
	W25Q256,
	W25Q512,
	
}W25QXX_ID_t;

typedef struct
{
	W25QXX_ID_t	ID;
	uint8_t			UniqID[8];

	uint16_t    PageSize;
	uint32_t		PageCount;
	uint32_t		SectorSize;
	uint32_t		SectorCount;
	uint32_t		BlockSize;
	uint32_t		BlockCount;

	uint32_t		CapacityInKiloByte;
	
	uint8_t	StatusRegister1;
	uint8_t	StatusRegister2;
	uint8_t	StatusRegister3;
	
	uint8_t	Lock;
	
}w25qxx_t;

extern w25qxx_t	w25qxx;
//############################################################################
// in Page,Sector and block read/write functions, can put 0 to read maximum bytes 
//############################################################################
void hSPI1_Init(void);
bool	W25qxx_Init(void);

void	W25qxx_EraseChip(void);
void 	W25qxx_EraseSector(uint32_t SectorAddr);
void 	W25qxx_EraseBlock(uint32_t BlockAddr);

uint32_t	W25qxx_PageToSector(uint32_t	PageAddress);
uint32_t	W25qxx_PageToBlock(uint32_t	PageAddress);
uint32_t	W25qxx_SectorToBlock(uint32_t	SectorAddress);
uint32_t	W25qxx_SectorToPage(uint32_t	SectorAddress);
uint32_t	W25qxx_BlockToPage(uint32_t	BlockAddress);

void 	W25qxx_WriteByte(uint8_t pBuffer,uint32_t Bytes_Address);

void 	W25qxx_ReadByte(uint8_t *pBuffer,uint32_t Bytes_Address);
void 	W25qxx_ReadBytes(uint8_t *pBuffer,uint32_t ReadAddr,uint32_t NumByteToRead);
//############################################################################
void W25qxx_WriteBytes(uint8_t *pBuffer, uint32_t Addr, uint32_t len);
void W25qxx_WriteBytes_64 (uint8_t* data);
uint16_t counter (void);
void W25qxx_ReadBytes_all (void);
void password (void);

void err (void);
	
#ifdef __cplusplus
}
#endif

#endif

