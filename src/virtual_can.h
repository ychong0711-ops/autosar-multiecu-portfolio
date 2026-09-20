#ifndef VIRTUAL_CAN_H
#define VIRTUAL_CAN_H

#include "autosar_types.h"

typedef void (*CanRxCallback)(const CanFrame *frame, void *context);

typedef struct {
    CanRxCallback receiver;
    void *receiver_context;
    Scenario scenario;
    unsigned transmitted;
    unsigned delivered;
    unsigned dropped;
} VirtualCanBus;

void VirtualCan_Init(VirtualCanBus *bus, Scenario scenario,
                     CanRxCallback receiver, void *context);
Std_ReturnType VirtualCan_Transmit(VirtualCanBus *bus, const CanFrame *frame);

#endif
