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
    if (strcmp(name, "wrap") == 0) return SCENARIO_WRAP;
    if (strcmp(name, "uds") == 0) return SCENARIO_UDS;
    if (strcmp(name, "wdgm-recovery") == 0) return SCENARIO_WDGM_RECOVERY;
    if (strcmp(name, "uds-dtc") == 0) return SCENARIO_UDS_DTC;
    fprintf(stderr, "Unknown scenario: %s\n", name);
    return -1;
}

static const char *scenario_name(Scenario scenario) {
    static const char *names[] = {"normal", "timeout", "invalid-id",
                                  "invalid-dlc", "invalid-range", "invalid-seq",
                                  "wrap", "uds", "wdgm-recovery", "uds-dtc"};
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
    NvM_Descriptor nvm;
    NvM_Init(&nvm);
    Ecu2_Init(&ecu2, &nvm);
    VirtualCan_Init(&bus, scenario, Ecu2_CanIf_RxIndication, &ecu2);
    Ecu1_Init(&ecu1, &bus, &nvm);

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
    /* Exercise NvM readback and error paths for coverage. */
    uint8_t nvm_buf[2];
    (void)NvM_ReadBlock(&nvm, NVM_BLOCKVehicleSpeed, nvm_buf, sizeof(nvm_buf));
    (void)NvM_ReadBlock(&nvm, NVM_BLOCKConfig, nvm_buf, sizeof(nvm_buf));
    (void)NvM_ReadBlock(&nvm, 0xFF, nvm_buf, sizeof(nvm_buf));
    (void)NvM_ReadBlock(&nvm, NVM_BLOCKVehicleSpeed, nvm_buf, 100u);
    (void)NvM_WriteBlock(&nvm, 0xFF, nvm_buf, sizeof(nvm_buf));
    printf("[NVM] reads=%u writes=%u\n",
           NvM_GetReadCount(&nvm), NvM_GetWriteCount(&nvm));
    /* Exercise WdgM_GetLocalStatus for coverage. */
    (void)WdgM_GetLocalStatus(&ecu2.wdgm);
    return 0;
}
