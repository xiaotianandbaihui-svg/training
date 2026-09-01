#include "stm32f4xx.h"                  // Device header
#include "My_Flash.h"
#include "stm32f4xx_flash.h"
#include <stdio.h>
uint16_t STMFLASH_GetFlashSector(u32 addr)
{
	if(addr<ADDR_FLASH_SECTOR_1)return FLASH_Sector_0;
	else if(addr<ADDR_FLASH_SECTOR_2)return FLASH_Sector_1;
	else if(addr<ADDR_FLASH_SECTOR_3)return FLASH_Sector_2;
	else if(addr<ADDR_FLASH_SECTOR_4)return FLASH_Sector_3;
	else if(addr<ADDR_FLASH_SECTOR_5)return FLASH_Sector_4;
	else if(addr<ADDR_FLASH_SECTOR_6)return FLASH_Sector_5;
	else if(addr<ADDR_FLASH_SECTOR_7)return FLASH_Sector_6;
	else if(addr<ADDR_FLASH_SECTOR_8)return FLASH_Sector_7;
	else if(addr<ADDR_FLASH_SECTOR_9)return FLASH_Sector_8;
	else if(addr<ADDR_FLASH_SECTOR_10)return FLASH_Sector_9;
	else if(addr<ADDR_FLASH_SECTOR_11)return FLASH_Sector_10; 
	return FLASH_Sector_11;	
}


void My_Flash_Era_Unlock()
{
	FLASH_Unlock();
	FLASH_DataCacheCmd(DISABLE);
	FLASH_EraseSector(FLASH_Sector_4,VoltageRange_2);
	FLASH_EraseSector(FLASH_Sector_5,VoltageRange_2);
	FLASH_EraseSector(FLASH_Sector_6,VoltageRange_2);
	FLASH_EraseSector(FLASH_Sector_7,VoltageRange_2);
	FLASH_EraseSector(FLASH_Sector_8,VoltageRange_2);
	FLASH_EraseSector(FLASH_Sector_9,VoltageRange_2);
	FLASH_EraseSector(FLASH_Sector_10,VoltageRange_2);
	FLASH_EraseSector(FLASH_Sector_11,VoltageRange_2);
		FLASH_DataCacheCmd(ENABLE);
	FLASH_Lock();

}
uint32_t error_value=0;
uint8_t My_Flash_Prog(uint32_t count,__IO uint16_t**Start_Pointer,uint16_t* My_Array)
{
	FLASH_Unlock();
	
	for(int i=0;i<count;i++)
	{
		if(FLASH_ProgramHalfWord((uint32_t)(*Start_Pointer),My_Array[i])!=FLASH_COMPLETE)
		{			
			FLASH_Lock();
			error_value++;
			return 0;//存在写入失败
		}
		*Start_Pointer+=1;
	}
	
	FLASH_Lock();
	return 1;//全部写入成功
}


uint16_t My_Flash_Read(uint32_t Start_Pointer)
{
	uint16_t temp=0;
	temp=*(__IO uint16_t*)Start_Pointer;
	return temp;
}
