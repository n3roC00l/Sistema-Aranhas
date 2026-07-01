#pragma once
#include <Arduino.h>

// =============================================================================
// SISTEMA ARANHAS — Configurações centrais
// Edite apenas este arquivo para ajustar pinos, setpoints e comportamento.
// =============================================================================

// ─── Geral ────────────────────────────────────────────────────────────────────
#define NUM_TERRARIOS  3

// ─── Pinos dos relés ──────────────────────────────────────────────────────────
// GPIOs 25/26/27: livres de strapping de boot e de SPI interna.
#define PINO_RELE_T1  25   // T1 — Lasiodora klugi
#define PINO_RELE_T2  26   // T2 — Monocentropus balfouri
#define PINO_RELE_T3  27   // T3 — Heteroscodra maculata

// ─── Pinos dos sensores DHT22 ────────────────────────────────────────────────
// Requerem resistor pull-up externo de 10 kΩ para 3.3V na linha de dados.
#define PINO_DHT_T1    4
#define PINO_DHT_T2    5
#define PINO_DHT_T3   13

// ─── Polaridade do módulo relé ────────────────────────────────────────────────
// VALIDAR com teste físico antes de gravar no hardware definitivo.
// Procedimento: alimente o módulo com JD-VCC separado (ver especificacoes-hardware.md),
// toque IN1 no GND — se o relé clicar, o módulo é ativo em LOW.
//
//   LOW  = relé energiza com sinal 0 V  (mais comum com JD-VCC separado)
//   HIGH = relé energiza com sinal 3.3V
//
// RELAY_SAFE_LEVEL deve ser sempre o oposto de RELAY_ACTIVE_LEVEL.
#define RELAY_ACTIVE_LEVEL  LOW
#define RELAY_SAFE_LEVEL    HIGH

// ─── Setpoints por terrário ───────────────────────────────────────────────────
// Valores de referência (fontes públicas de husbandry).
// Validar com o mantenedor antes de usar em produção.

// T1 — Lasiodora klugi (Bahia Scarlet) — tropical úmido, espécie robusta
#define T1_TEMP_MIN   24.0f
#define T1_TEMP_MAX   29.0f
#define T1_HISTERESE   1.0f   // banda morta: ±1°C em torno dos limites
#define T1_UMID_MIN   60.0f
#define T1_UMID_MAX   70.0f

// T2 — Monocentropus balfouri (Blue Baboon) — origem árida (Socotra)
// ATENÇÃO: sensível ao excesso de umidade — alerta bidirecional no firmware
#define T2_TEMP_MIN   24.0f
#define T2_TEMP_MAX   28.0f
#define T2_HISTERESE   1.0f
#define T2_UMID_MIN   50.0f
#define T2_UMID_MAX   60.0f

// T3 — Heteroscodra maculata (Togo Starburst) — arborícola, umidade constante
#define T3_TEMP_MIN   24.0f
#define T3_TEMP_MAX   28.0f
#define T3_HISTERESE   1.0f
#define T3_UMID_MIN   60.0f
#define T3_UMID_MAX   70.0f

// ─── Limites de validação de leitura do DHT22 ────────────────────────────────
// Leituras fora desta faixa são tratadas como falha de sensor.
// Guarapuava-PR: temperatura ambiente esperada entre 10°C e 40°C.
#define TEMP_MIN_VALIDA   5.0f
#define TEMP_MAX_VALIDA  50.0f
#define UMID_MIN_VALIDA  10.0f
#define UMID_MAX_VALIDA  99.0f

// ─── Temporização ─────────────────────────────────────────────────────────────
// DHT22 exige no mínimo 2000 ms entre leituras consecutivas.
#define INTERVALO_LEITURA_MS  2000UL

// ─── Watchdog ─────────────────────────────────────────────────────────────────
// Se o loop() travar por mais que este tempo, o ESP32 reinicia.
// Ao reiniciar, setup() coloca os relés em estado seguro novamente.
#define WATCHDOG_TIMEOUT_S  10

// ─── Estrutura de configuração imutável por terrário ─────────────────────────
struct ConfigTerrario {
    uint8_t pinoRele;
    float   tempMin;
    float   tempMax;
    float   histerese;
    float   umidMin;
    float   umidMax;
};
