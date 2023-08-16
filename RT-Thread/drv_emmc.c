#include "drv_emmc.h"

uint8_t BSP_MMC_ReadBlocks_DMA(uint8_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks)
{
   HAL_StatusTypeDef Status =  HAL_OK;
 
   if(HAL_MMC_ReadBlocks_DMA(&hmmc1, pData, ReadAddr, NumOfBlocks) != HAL_OK)
   {
      Status = HAL_ERROR;
   } 
   return Status; 
}
