#include "virtual_can.h"
#include <stdio.h>

void VirtualCan_Init(VirtualCanBus *bus, Scenario scenario,
                     CanRxCallback receiver, void *context) {
    *bus = (VirtualCanBus){
        .receiver = receiver,
        .receiver_context = context,
        .scenario = scenario
    };
}

Std_ReturnType VirtualCan_Transmit(VirtualCanBus *bus, const CanFrame *source) {
    CanFrame frame = *source;
    bus->transmitted++;

    if (bus->scenario == SCENARIO_TIMEOUT && frame.timestamp_ms >= 300u) {
        bus->dropped++;
        printf("[%04ums][BUS] DROP reason=fault-injection id=0x%03X\n",
               frame.timestamp_ms, frame.id);
        return E_OK;
    }
    if (bus->scenario == SCENARIO_INVALID_ID && frame.timestamp_ms == 300u) {
        frame.id = 0x777u;
    }
    if (bus->scenario == SCENARIO_INVALID_DLC && frame.timestamp_ms == 300u) {
        frame.dlc = 2u;
    }
    if (bus->scenario == SCENARIO_INVALID_RANGE) {
        if (frame.timestamp_ms == 0u) {
            frame.data[0] = 0xFFu;
            frame.data[1] = 0xFFu;           /* 0xFFFF == 65535 kph > 250 */
        } else if (frame.timestamp_ms == 500u) {
            frame.data[0] = 250u;
            frame.data[1] = 0u;              /* exact boundary: must be accepted */
        } else if (frame.timestamp_ms == 800u) {
            frame.data[0] = 251u;
            frame.data[1] = 0u;              /* 251 kph > 250 */
        }
    }
    if (bus->scenario == SCENARIO_INVALID_SEQ && frame.timestamp_ms == 500u) {
        frame.data[2] = 3u;                  /* duplicate of an old counter */
    }

    printf("[%04ums][BUS] ROUTE id=0x%03X dlc=%u ECU1->ECU2\n",
           frame.timestamp_ms, frame.id, frame.dlc);
    bus->delivered++;
    bus->receiver(&frame, bus->receiver_context);
    return E_OK;
}
