#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

#define VEHICLE_SPEED_CAN_ID       0x101u
#define VEHICLE_SPEED_DLC          3u
#define TX_CYCLE_MS                100u
#define RX_TIMEOUT_MS              500u
#define SIMULATION_END_MS          1000u
#define VEHICLE_SPEED_MAX_KPH      250u

/* WdgM configuration: deadline for alive-counter supervision */
#define WDGM_DEADLINE_MS           RX_TIMEOUT_MS

/* NvM configuration */
#define NVM_PERSIST_VEHICLE_SPEED  1u

#endif
