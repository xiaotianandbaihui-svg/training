#ifndef MY_FLASH_H
#define MY_FLASH_H

#define ADDR_FLASH_SECTOR_0     ((u32)0x08000000) 	
#define ADDR_FLASH_SECTOR_1     ((u32)0x08004000) 	 
#define ADDR_FLASH_SECTOR_2     ((u32)0x08008000) 	 
#define ADDR_FLASH_SECTOR_3     ((u32)0x0800C000) 	  
#define ADDR_FLASH_SECTOR_4     ((u32)0x08010000) 	  
#define ADDR_FLASH_SECTOR_5     ((u32)0x08020000) 	 
#define ADDR_FLASH_SECTOR_6     ((u32)0x08040000) 	
#define ADDR_FLASH_SECTOR_7     ((u32)0x08060000) 	
#define ADDR_FLASH_SECTOR_8     ((u32)0x08080000) 	 
#define ADDR_FLASH_SECTOR_9     ((u32)0x080A0000) 	
#define ADDR_FLASH_SECTOR_10    ((u32)0x080C0000) 	
#define ADDR_FLASH_SECTOR_11    ((u32)0x080E0000) 	 

uint16_t STMFLASH_GetFlashSector(u32 addr);
void My_Flash_Era_Unlock(void);
uint8_t My_Flash_Prog(uint32_t count,__IO uint16_t**Start_Pointer,uint16_t* My_Array);
uint16_t My_Flash_Read(uint32_t Start_Pointer);
#endif
