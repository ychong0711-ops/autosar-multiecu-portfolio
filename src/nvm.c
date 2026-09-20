#include "nvm.h"
#include <stdio.h>
#include <string.h>

void NvM_Init(NvM_Descriptor *nvm) {
    *nvm = (NvM_Descriptor){0};
}

bool NvM_ReadBlock(NvM_Descriptor *nvm, uint8_t block_id,
                   void *dest, size_t len) {
    if (block_id >= NVM_NUM_BLOCKS || len > NVM_BLOCK_SIZE) {
        return false;
    }
    if (!nvm->blocks[block_id].valid) {
        return false;
    }
    nvm->read_count++;
    memcpy(dest, nvm->blocks[block_id].data, len);
    return true;
}

bool NvM_WriteBlock(NvM_Descriptor *nvm, uint8_t block_id,
                    const void *src, size_t len) {
    if (block_id >= NVM_NUM_BLOCKS || len > NVM_BLOCK_SIZE) {
        return false;
    }
    nvm->write_count++;
    memcpy(nvm->blocks[block_id].data, src, len);
    nvm->blocks[block_id].valid = true;
    return true;
}

unsigned NvM_GetReadCount(const NvM_Descriptor *nvm) {
    return nvm->read_count;
}

unsigned NvM_GetWriteCount(const NvM_Descriptor *nvm) {
    return nvm->write_count;
}
