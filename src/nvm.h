#ifndef NVM_H
#define NVM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NVM_BLOCK_SIZE      16u
#define NVM_NUM_BLOCKS      8u
#define NVM_BLOCKVehicleSpeed   0u
#define NVM_BLOCKConfig         1u

typedef struct {
    uint8_t data[NVM_BLOCK_SIZE];
    bool    valid;
} NvM_Block;

typedef struct {
    NvM_Block blocks[NVM_NUM_BLOCKS];
    unsigned  read_count;
    unsigned  write_count;
} NvM_Descriptor;

void     NvM_Init(NvM_Descriptor *nvm);
bool     NvM_ReadBlock(NvM_Descriptor *nvm, uint8_t block_id,
                       void *dest, size_t len);
bool     NvM_WriteBlock(NvM_Descriptor *nvm, uint8_t block_id,
                        const void *src, size_t len);
unsigned NvM_GetReadCount(const NvM_Descriptor *nvm);
unsigned NvM_GetWriteCount(const NvM_Descriptor *nvm);

#endif
