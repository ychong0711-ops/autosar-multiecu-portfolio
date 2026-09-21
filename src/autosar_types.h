#ifndef AUTOSAR_TYPES_H
#define AUTOSAR_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t Std_ReturnType;
#define E_OK     ((Std_ReturnType)0u)
#define E_NOT_OK ((Std_ReturnType)1u)

typedef struct {
    uint32_t id;
    uint8_t dlc;
    uint8_t data[8];
    uint32_t timestamp_ms;
} CanFrame;

typedef enum {
    SCENARIO_NORMAL,
    SCENARIO_TIMEOUT,
    SCENARIO_INVALID_ID,
    SCENARIO_INVALID_DLC,
    SCENARIO_INVALID_RANGE,
    SCENARIO_INVALID_SEQ,
    SCENARIO_WRAP,
    SCENARIO_UDS,
    SCENARIO_UDS_DTC,
    SCENARIO_WDGM_RECOVERY
} Scenario;

#endif
