#ifndef NVM_H
#define NVM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NVM_BLOCK_SIZE      16u
#define NVM_NUM_BLOCKS      8u
#define NVM_BLOCKVehicleSpeed   0u
#define NVM_BLOCKConfig         1u

/* CRC configuration */
#define NVM_CRC_POLY         0x1021u  /* CRC-CCITT */
#define NVM_CRC_INIT         0xFFFFu

typedef struct {
    uint8_t data[NVM_BLOCK_SIZE];
    uint16_t crc;
    bool    valid;
} NvM_Block;

typedef struct {
    NvM_Block blocks[NVM_NUM_BLOCKS];
    NvM_Block redundant_blocks[NVM_NUM_BLOCKS];  /* Redundant copy for integrity */
    unsigned  read_count;
    unsigned  write_count;
    unsigned  crc_error_count;
    unsigned  redundancy_mismatch_count;
} NvM_Descriptor;

void     NvM_Init(NvM_Descriptor *nvm);
bool     NvM_ReadBlock(NvM_Descriptor *nvm, uint8_t block_id,
                       void *dest, size_t len);
bool     NvM_WriteBlock(NvM_Descriptor *nvm, uint8_t block_id,
                        const void *src, size_t len);
unsigned NvM_GetReadCount(const NvM_Descriptor *nvm);
unsigned NvM_GetWriteCount(const NvM_Descriptor *nvm);
unsigned NvM_GetCrcErrorCount(const NvM_Descriptor *nvm);
unsigned NvM_GetRedundancyMismatchCount(const NvM_Descriptor *nvm);
uint16_t NvM_CalculateCrc16(const void *data, size_t len);
bool     NvM_VerifyBlockIntegrity(const NvM_Descriptor *nvm, uint8_t block_id);

#endif