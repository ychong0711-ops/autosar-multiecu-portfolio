#include "ecu.h"
#include "../config/project_config.h"
#include <stdio.h>

static Std_ReturnType Can_Write(Ecu1 *ecu, const CanFrame *frame) {
    printf("[%04ums][ECU1][CAN] Can_Write id=0x%03X dlc=%u\n",
           frame->timestamp_ms, frame->id, frame->dlc);
    return VirtualCan_Transmit(ecu->bus, frame);
}

static Std_ReturnType CanIf_Transmit(Ecu1 *ecu, const CanFrame *frame) {
    printf("[%04ums][ECU1][CANIF] CanIf_Transmit pdu=VehicleSpeedPdu\n",
           frame->timestamp_ms);
    Std_ReturnType result = Can_Write(ecu, frame);
    if (result == E_OK) {
        ecu->tx_confirmations++;
        printf("[%04ums][ECU1][CANIF] CanIf_TxConfirmation pdu=VehicleSpeedPdu\n",
               frame->timestamp_ms);
    }
    return result;
}

static Std_ReturnType PduR_ComTransmit(Ecu1 *ecu, const CanFrame *frame) {
    printf("[%04ums][ECU1][PDUR] PduR_ComTransmit\n", frame->timestamp_ms);
    return CanIf_Transmit(ecu, frame);
}

static Std_ReturnType Com_SendSignal(Ecu1 *ecu, uint32_t now_ms,
                                     uint16_t speed_kph) {
    CanFrame frame = {
        .id = VEHICLE_SPEED_CAN_ID,
        .dlc = VEHICLE_SPEED_DLC,
        .data = {
            (uint8_t)(speed_kph & 0xFFu),
            (uint8_t)((speed_kph >> 8u) & 0xFFu),
            ecu->sequence++
        },
        .timestamp_ms = now_ms
    };
    printf("[%04ums][ECU1][COM] Com_SendSignal speed=%ukph seq=%u\n",
           now_ms, speed_kph, frame.data[2]);
    return PduR_ComTransmit(ecu, &frame);
}

void Ecu1_Init(Ecu1 *ecu, VirtualCanBus *bus) {
    *ecu = (Ecu1){.bus = bus};
    puts("[INIT][ECU1] ATK2 task + COM/PduR/CanIf/Can initialized");
}

void Ecu2_Init(Ecu2 *ecu) {
    *ecu = (Ecu2){0};
    puts("[INIT][ECU2] ATK2 task + CAN/CanIf/PduR/COM initialized");
}

void Ecu1_100msTask(Ecu1 *ecu, uint32_t now_ms, uint16_t speed_kph) {
    printf("[%04ums][ECU1][OS] Task_100ms activated\n", now_ms);
    (void)Com_SendSignal(ecu, now_ms, speed_kph);
}

static void Com_RxIndication(Ecu2 *ecu, const CanFrame *frame) {
    uint16_t speed = (uint16_t)frame->data[0] |
                     ((uint16_t)frame->data[1] << 8u);
    /*
     * E2E Profile-1 style continuity check on the 8-bit rolling counter in
     * frame->data[2]. diff is computed with unsigned mod-256 arithmetic, so
     * a genuine forward wrap 255 -> 0 yields diff == 1 (not -255) and is
     * treated as NEW. Half-range maximum-delta rule (AUTOSAR E2E
     * convention): the frame is NEW iff 0 < diff <= 128. This accepts every
     * legitimate single-step advance including the wrap, and rejects
     * duplicates (diff == 0), backward replays and oversized forward gaps
     * (diff > 128) that indicate loss of message order.
     */
    uint8_t diff = (uint8_t)((uint8_t)frame->data[2] -
                             (uint8_t)ecu->last_sequence);
    if (ecu->ever_received && !(diff > 0u && diff <= 128u)) {
        ecu->rejected_seq++;
        printf("[%04ums][ECU2][COM] REJECT reason=seq-not-new counter=%u last=%u\n",
               frame->timestamp_ms, frame->data[2], ecu->last_sequence);
        return;
    }
    if (speed > VEHICLE_SPEED_MAX_KPH) {
        ecu->rejected_range++;
        printf("[%04ums][ECU2][COM] REJECT reason=range speed=%u\n",
               frame->timestamp_ms, speed);
        return;
    }
    ecu->vehicle_speed_kph = speed;
    ecu->last_sequence = frame->data[2];
    ecu->last_rx_ms = frame->timestamp_ms;
    ecu->ever_received = true;
    ecu->timeout_active = false;
    ecu->accepted++;
    printf("[%04ums][ECU2][COM] Com_RxIndication speed=%ukph seq=%u result=ACCEPT\n",
           frame->timestamp_ms, speed, ecu->last_sequence);
}

void Ecu2_CanIf_RxIndication(const CanFrame *frame, void *context) {
    Ecu2 *ecu = context;
    printf("[%04ums][ECU2][CANIF] CanIf_RxIndication id=0x%03X dlc=%u\n",
           frame->timestamp_ms, frame->id, frame->dlc);
    if (frame->id != VEHICLE_SPEED_CAN_ID) {
        ecu->rejected_id++;
        printf("[%04ums][ECU2][CANIF] REJECT reason=unexpected-can-id\n",
               frame->timestamp_ms);
        return;
    }
    if (frame->dlc != VEHICLE_SPEED_DLC) {
        ecu->rejected_dlc++;
        printf("[%04ums][ECU2][CANIF] REJECT reason=invalid-dlc\n",
               frame->timestamp_ms);
        return;
    }
    printf("[%04ums][ECU2][PDUR] PduR_CanIfRxIndication\n", frame->timestamp_ms);
    Com_RxIndication(ecu, frame);
}

void Ecu2_100msTask(Ecu2 *ecu, uint32_t now_ms) {
    printf("[%04ums][ECU2][OS] Task_100ms activated\n", now_ms);
    if (ecu->ever_received && !ecu->timeout_active &&
        (now_ms - ecu->last_rx_ms) >= RX_TIMEOUT_MS) {
        ecu->timeout_active = true;
        ecu->timeout_events++;
        printf("[%04ums][ECU2][COM] TIMEOUT age=%ums threshold=%ums\n",
               now_ms, now_ms - ecu->last_rx_ms, RX_TIMEOUT_MS);
    }
}
