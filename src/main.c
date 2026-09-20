#include "ecu.h"
#include "../config/project_config.h"
#include <stdio.h>
#include <string.h>

static int parse_scenario(const char *name) {
    if (strcmp(name, "normal") == 0) return SCENARIO_NORMAL;
    if (strcmp(name, "timeout") == 0) return SCENARIO_TIMEOUT;
    if (strcmp(name, "invalid-id") == 0) return SCENARIO_INVALID_ID;
    if (strcmp(name, "invalid-dlc") == 0) return SCENARIO_INVALID_DLC;
    if (strcmp(name, "invalid-range") == 0) return SCENARIO_INVALID_RANGE;
    if (strcmp(name, "invalid-seq") == 0) return SCENARIO_INVALID_SEQ;
    fprintf(stderr, "Unknown scenario: %s\n", name);
    return -1;
}

static const char *scenario_name(Scenario scenario) {
    static const char *names[] = {"normal", "timeout", "invalid-id",
                                  "invalid-dlc", "invalid-range", "invalid-seq"};
    return names[scenario];
}

int main(int argc, char **argv) {
    const char *requested = argc > 1 ? argv[1] : "normal";
    int parsed = parse_scenario(requested);
    if (parsed < 0) return 2;
    Scenario scenario = (Scenario)parsed;

    Ecu1 ecu1;
    Ecu2 ecu2;
    VirtualCanBus bus;
    Ecu2_Init(&ecu2);
    VirtualCan_Init(&bus, scenario, Ecu2_CanIf_RxIndication, &ecu2);
    Ecu1_Init(&ecu1, &bus);

    printf("[RUN] scenario=%s cycle=%ums timeout=%ums\n",
           scenario_name(scenario), TX_CYCLE_MS, RX_TIMEOUT_MS);
    for (uint32_t now = 0; now <= SIMULATION_END_MS; now += TX_CYCLE_MS) {
        uint16_t speed = (uint16_t)(60u + now / 20u);
        Ecu1_100msTask(&ecu1, now, speed);
        Ecu2_100msTask(&ecu2, now);
    }

    printf("[SUMMARY] scenario=%s tx=%u delivered=%u dropped=%u accepted=%u "
           "reject_id=%u reject_dlc=%u reject_range=%u reject_seq=%u "
           "timeouts=%u tx_confirm=%u\n",
           scenario_name(scenario), bus.transmitted, bus.delivered, bus.dropped,
           ecu2.accepted, ecu2.rejected_id, ecu2.rejected_dlc,
           ecu2.rejected_range, ecu2.rejected_seq,
           ecu2.timeout_events, ecu1.tx_confirmations);
    return 0;
}
