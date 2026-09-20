#ifndef WDGM_H
#define WDGM_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    WDG_OK,
    WDG_FAILED,
    WDG_EXPIRED
} WdgM_LocalStatusType;

typedef struct {
    uint32_t deadline_ms;
    uint32_t last_checkpoint_ms;
    uint32_t alive_counter;
    uint32_t expected_alive_counter;
    unsigned  deadline_missed_count;
    unsigned  expired_count;
    WdgM_LocalStatusType status;
} WdgM_SupervisedEntity;

void WdgM_Init(WdgM_SupervisedEntity *entity, uint32_t deadline_ms);
void WdgM_CheckpointReached(WdgM_SupervisedEntity *entity, uint32_t now_ms);
void WdgM_MainFunction(WdgM_SupervisedEntity *entity, uint32_t now_ms);
WdgM_LocalStatusType WdgM_GetLocalStatus(const WdgM_SupervisedEntity *entity);

#endif
