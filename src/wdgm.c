#include "wdgm.h"
#include <stdio.h>

void WdgM_Init(WdgM_SupervisedEntity *entity, uint32_t deadline_ms) {
    *entity = (WdgM_SupervisedEntity){
        .deadline_ms = deadline_ms,
        .status = WDG_OK
    };
}

void WdgM_CheckpointReached(WdgM_SupervisedEntity *entity, uint32_t now_ms) {
    entity->last_checkpoint_ms = now_ms;
    entity->alive_counter++;
}

void WdgM_MainFunction(WdgM_SupervisedEntity *entity, uint32_t now_ms) {
    if (entity->alive_counter != entity->expected_alive_counter) {
        entity->expected_alive_counter = entity->alive_counter;
    }
    uint32_t age = now_ms - entity->last_checkpoint_ms;
    if (age >= entity->deadline_ms) {
        entity->deadline_missed_count++;
        entity->status = WDG_FAILED;
        printf("[%04ums][WDGM] deadline missed age=%ums threshold=%ums "
               "missed_count=%u\n",
               now_ms, age, entity->deadline_ms, entity->deadline_missed_count);
        if (entity->deadline_missed_count >= 3u) {
            entity->status = WDG_EXPIRED;
            entity->expired_count++;
            printf("[%04ums][WDGM] EXPIRED missed_count=%u expired_total=%u\n",
                   now_ms, entity->deadline_missed_count, entity->expired_count);
        }
    } else {
        if (entity->status == WDG_FAILED && entity->deadline_missed_count > 0u) {
            entity->deadline_missed_count--;
        }
        entity->status = WDG_OK;
    }
}

WdgM_LocalStatusType WdgM_GetLocalStatus(const WdgM_SupervisedEntity *entity) {
    return entity->status;
}
