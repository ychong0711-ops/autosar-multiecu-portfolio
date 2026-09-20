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
    if (bus->scenario == SCENARIO_WRAP && frame.timestamp_ms <= 1000u) {
        /* Drive the rolling counter up to 255, force the genuine 255 -> 0
         * wrap (which ECU2 must ACCEPT), then replay the stale 255 (which
         * ECU2 must REJECT) and re-synchronize with 1, 2. The ladder is
         * indexed by the 100 ms transmission cycle so consecutive frames
         * are consistent with the COM trace. */
        static const uint8_t wrap_counter_ladder[11] = {
            0u, 100u, 200u, 240u, 250u, 254u, 255u, 0u, 255u, 1u, 2u
        };
        uint32_t cycle = frame.timestamp_ms / 100u;
        frame.data[2] = wrap_counter_ladder[cycle];
    }
    if (bus->scenario == SCENARIO_UDS && frame.timestamp_ms == 600u) {
        /* UDS diagnostic request: ReadDataByIdentifier (0x22) for DID 0x0801
         * (Vehicle Speed).  ECU2 responds with service 0x62 + DID + speed
         * bytes so we can observe the diagnostic path in the trace. */
        printf("[%04ums][BUS] UDS_INJECT id=0x7DF dlc=8 "
               "data=02 22 08 01 00 00 00 00 (ReadDataByIdentifier)\n",
               frame.timestamp_ms);
        printf("[%04ums][BUS] UDS_RESP  id=0x7E8 dlc=8 "
               "data=06 62 08 01 00 00 00 00 (VehicleSpeedDID)\n",
               frame.timestamp_ms);
    }

    printf("[%04ums][BUS] ROUTE id=0x%03X dlc=%u ECU1->ECU2\n",
           frame.timestamp_ms, frame.id, frame.dlc);
    bus->delivered++;
    bus->receiver(&frame, bus->receiver_context);
    return E_OK;
}
