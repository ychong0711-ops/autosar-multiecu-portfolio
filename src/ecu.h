#ifndef ECU_H
#define ECU_H

#include "autosar_types.h"
#include "virtual_can.h"

typedef struct {
    VirtualCanBus *bus;
    uint8_t sequence;
    unsigned tx_confirmations;
} Ecu1;

typedef struct {
    uint16_t vehicle_speed_kph;
    uint8_t last_sequence;
    uint32_t last_rx_ms;
    bool ever_received;
    bool timeout_active;
    unsigned accepted;
    unsigned rejected_id;
    unsigned rejected_dlc;
    unsigned rejected_range;
    unsigned rejected_seq;
    unsigned timeout_events;
} Ecu2;

void Ecu1_Init(Ecu1 *ecu, VirtualCanBus *bus);
void Ecu2_Init(Ecu2 *ecu);
void Ecu1_100msTask(Ecu1 *ecu, uint32_t now_ms, uint16_t speed_kph);
void Ecu2_100msTask(Ecu2 *ecu, uint32_t now_ms);
void Ecu2_CanIf_RxIndication(const CanFrame *frame, void *context);

#endif
