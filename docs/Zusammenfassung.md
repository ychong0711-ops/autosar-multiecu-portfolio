# Projektzusammenfassung — Virtuelle Multi-ECU AUTOSAR-Kommunikation

## Ausgangslage

Bewerber im Bereich Automobilsoftware zeigen häufig Code, ohne Anforderungen,
Architektur, Verifikation oder Grenzen des eigenen Schaffens zu belegen.
Dieses Projekt beantwortet eine schmale, aber praxisrelevante Engineering-Frage
ganzheitlich:

> Können zwei virtuelle ECUs ein zyklisches Fahrzeuggeschwindigkeitssignal
> über einen AUTOSAR Classic-artigen Kommunikationsstack austauschen,
> fehlerhafte Frames verwerfen und Kommunikationsverlust deterministisch
> erkennen?

## Meine Beiträge

- Definition eines 100-ms-CAN-Schnittstellenvertrags für Fahrzeuggeschwindigkeit
  und Sequenzcounter.
- Implementierung eines portablen C11-Referenzmodells mit getrennten COM-,
  PduR-, CanIf- und CAN-Verantwortlichkeiten.
- Implementierung zweier virtueller ECUs und deterministischer Busfehlerinjektion.
- Hinzufügen von Timeout-, Identifier-, DLC- und Plausibilitätsschutz.
- Acht automatisierte Integrationsszenarien (normal, timeout, invalid-id,
  invalid-dlc, invalid-range, invalid-seq, wrap, uds) und CI mit
  Warnungen als Fehler.
- Erstellung von SWE.1–SWE.5-Arbeitsprodukten und bidirektionaler
  Anforderungs-zu-Test-Rückverfolgbarkeit.
- Fixierte, reproduzierbare TOPPERS ATK2/A-COMSTACK/A-RTEGEN/Athrill-Integration.
- Erweiterung des Upstream-Samples mit einem eigenen 60..250-kph-Fahrzeuggeschwindigkeits-COM-Signal
  (Patch + Re-Run-Evidenz).

## Architektur

```text
ECU1 (Provider)                                      ECU2 (Consumer)
ATK2 100-ms-Task                                     ATK2 100-ms-Task
      │                                                    │
Com_SendSignal                                      Timeout-Monitor
      │                                                    ▲
PduR_ComTransmit                                      Com_RxIndication
      │                                                    ▲
CanIf_Transmit                                     PduR_CanIfRxIndication
      │                                                    ▲
Can_Write ───── CAN ID 0x101 / DLC 3 ────────> CanIf_RxIndication
```

Payload: `[Geschwindigkeit LSB, Geschwindigkeit MSB, Sequenz]`
Zykluszeit: `100 ms` · Timeout: `500 ms` · gültiger Bereich: `0..250 km/h`

## Verifikationsergebnis

Alle 14 automatisierten Integrationstests bestanden:

| Szenario | Anforderung | Ergebnis |
|---|---|---|
| `normal` | REQ-OS-001, REQ-COM-001..003 | 11/11 Frames akzeptiert; vollständiger Layer-Trace |
| `timeout` | REQ-ERR-001 | Frames ab 300 ms verworfen; Timeout bei genau 700 ms |
| `invalid-id` | REQ-ERR-002 | Ein unerwarteter Bezeichner abgelehnt |
| `invalid-dlc` | REQ-ERR-003 | Ein fehlerhafter DLC abgelehnt |
| `invalid-range` | REQ-RNG-001 | 65535 und 251 km/h abgelehnt; exakte Grenze 250 km/h akzeptiert |
| `invalid-seq` | REQ-ERR-004 | Ein duplizierter Counter bei 500 ms markiert; Stream re-synchronisiert |
| `wrap` | REQ-ERR-005 | Counter 255→0 akzeptiert; stale Replay abgelehnt (E2E-artig) |
| `uds` | REQ-UDS-001 | UDS-Diagnosetraffic koexistiert mit COM ohne Störung |

**Abdeckung:** 100 % Zeilenabdeckung über alle Quelldateien.
**Statische Analyse:** clang-tidy (clang-analyzer + bugprone), Warnungen als Fehler.

## Nachweisbare Fähigkeiten

| Domäne | Nachweis |
|---|---|
| AUTOSAR Classic | COM/PduR/CanIf-Schichtentrennung, CAN-Vertrag |
| E2E-Schutz | Mod-256-Sequenzcounter, Half-Range-Regel, Wrap-safe |
| UDS | ReadDataByIdentifier (0x22), Diagnose-COEXISTENCE |
| Deterministische Tests | Simulationszeit statt Echtzeit, fehlerfreie Reproduzierbarkeit |
| Fehlerinjektion | 8 Szenarien mit gezielter Störung |
| Prozess | ASPICE-inspirierte Dokumentation SWE.1–SWE.5 |
| Rückverfolgbarkeit | Bidirektionale Matrix: Anforderung → Architektur → Code → Test → Evidenz |
| CI/CD | GitHub Actions, Build + Test + Statische Analyse + Abdeckung |

## Technische Entscheidungen

1. **Deterministische Simulationszeit** — vermeidet instabile Echtzeittests.
2. **Host-Modell plus Target-Pfad** — Reviewer erhalten sofortige Evidenz,
   während der RH850/Athrill-Pfad reproduzierbar bleibt.
3. **Explizite Schichtentrennung** — ermöglicht Fehlerlokalisierung und
   kartografiert die Reasoning auf AUTOSAR Classic.
4. **Dokumentierte Nicht-Claims** — kein Safety-, Konformitäts- oder
   Produktionsreife-Anspruch.

## Grenzen und nächste Schritte

- Das gefixte RH850/Athrill-Integrationsergebnis wurde einmalig in Docker
  ausgeführt (`evidence/atk2-athrill.log`); es ist noch kein CI-geregelter
  Regressionstest.
- Das Host-Transportmodell modelliert keinen CAN-Arbitrierungsverzug,
  Bus-Off, Taktabweichung oder elektrische Störungen.
- Sequenzcounter-Wrap-Over wird durch ein dediziertes Fehlerinjektionsszenario
  verifiziert, das den Counter auf 255 klettert, auf 0 wrappt und Akzeptanz
  sowie Ablehnung eines alten Replays bestätigt.
- Kein AUTOSAR-E2E-Profil- oder DCM/DEM-Implementierung vorhanden.

Nächster Inkrement: CI-Gate für Athrill-Lauf, E2E-Alive-Counter-Richtlinie
und UDS-Diagnosedienst für Timeout-DTC-Auslesung.

## Reproduktion

```bash
make test && make evidence
```

```bash
make upstream-setup && make upstream-run
```

---

*Dieses Projekt ist ein Bildungsportfolio, keine AUTOSAR-Konformitäts- oder
ISO-26262-Sicherheitsbehauptung.*
