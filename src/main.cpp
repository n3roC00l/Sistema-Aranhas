#include <Arduino.h>
#include <DHT.h>
#include <esp_task_wdt.h>
#include "config.h"

// =============================================================================
// SISTEMA ARANHAS — Firmware ESP32
// Controle de termorregulação para 3 terrários de tarântulas.
//
// Regra de segurança inegociável: a lógica de histerese e o fail-safe
// rodam SEMPRE localmente neste firmware, sem dependência de rede ou dashboard.
// =============================================================================

// ─── Configuração imutável por terrário ──────────────────────────────────────
static const ConfigTerrario CFG[NUM_TERRARIOS] = {
    { PINO_RELE_T1, T1_TEMP_MIN, T1_TEMP_MAX, T1_HISTERESE, T1_UMID_MIN, T1_UMID_MAX },
    { PINO_RELE_T2, T2_TEMP_MIN, T2_TEMP_MAX, T2_HISTERESE, T2_UMID_MIN, T2_UMID_MAX },
    { PINO_RELE_T3, T3_TEMP_MIN, T3_TEMP_MAX, T3_HISTERESE, T3_UMID_MIN, T3_UMID_MAX },
};

static const char* NOMES[NUM_TERRARIOS] = {
    "T1-Lasiodora",
    "T2-Balfouri ",
    "T3-Maculata ",
};

// ─── Objetos dos sensores ─────────────────────────────────────────────────────
static DHT dht1(PINO_DHT_T1, DHT22);
static DHT dht2(PINO_DHT_T2, DHT22);
static DHT dht3(PINO_DHT_T3, DHT22);
static DHT* const SENSORES[NUM_TERRARIOS] = { &dht1, &dht2, &dht3 };

// ─── Estado em tempo de execução por terrário ─────────────────────────────────
struct EstadoTerrario {
    float   temp;
    float   umid;
    bool    aquecedorLigado;
    bool    sensorFalhou;
    uint8_t falhasConsecutivas;
};

static EstadoTerrario estado[NUM_TERRARIOS];

// ─── Controle de temporização ─────────────────────────────────────────────────
static unsigned long ultimaLeitura = 0;

// =============================================================================
// Funções de controle dos relés
// =============================================================================

static inline void setRele(uint8_t pino, bool ligar) {
    digitalWrite(pino, ligar ? RELAY_ACTIVE_LEVEL : RELAY_SAFE_LEVEL);
}

// Coloca todos os relés em estado seguro (aquecedores desligados).
// Chamada no boot e em qualquer condição de falha crítica.
static void relesSeguros() {
    setRele(PINO_RELE_T1, false);
    setRele(PINO_RELE_T2, false);
    setRele(PINO_RELE_T3, false);
}

// =============================================================================
// Validação de leitura do DHT22
// =============================================================================

static bool leituraValida(float temp, float umid) {
    if (isnan(temp) || isnan(umid))                        return false;
    if (temp < TEMP_MIN_VALIDA || temp > TEMP_MAX_VALIDA)  return false;
    if (umid < UMID_MIN_VALIDA || umid > UMID_MAX_VALIDA)  return false;
    return true;
}

// =============================================================================
// Lógica de histerese
//
// Regime:
//   temp < (tempMin - histerese)  → liga  (abaixo do limite inferior)
//   temp > (tempMax + histerese)  → desliga (acima do limite superior)
//   entre esses valores           → mantém estado atual (banda morta)
//
// A banda morta evita chaveamento rápido do relé quando a temperatura
// oscila próximo ao setpoint, protegendo o relé e o aquecedor.
// =============================================================================

static bool calcularAquecedor(float temp, bool estadoAtual,
                               const ConfigTerrario& cfg) {
    if (temp < cfg.tempMin - cfg.histerese) return true;
    if (temp > cfg.tempMax + cfg.histerese) return false;
    return estadoAtual;
}

// =============================================================================
// Verificação de alertas de umidade (lógica individual por espécie)
// =============================================================================

static void verificarAlertas(uint8_t idx) {
    const EstadoTerrario& est = estado[idx];
    const ConfigTerrario& cfg = CFG[idx];

    if (est.umid < cfg.umidMin) {
        Serial.printf("[%s] ALERTA: umidade baixa (%.0f%% < %.0f%%)\n",
                      NOMES[idx], est.umid, cfg.umidMin);
    }

    // T2 (idx=1): Monocentropus balfouri — origem árida, sensível ao excesso
    if (idx == 1 && est.umid > cfg.umidMax) {
        Serial.printf("[%s] ALERTA: umidade ALTA (%.0f%% > %.0f%%) — risco para balfouri\n",
                      NOMES[idx], est.umid, cfg.umidMax);
    }
}

// =============================================================================
// Processamento completo de um terrário (leitura + controle + alertas)
// =============================================================================

static void processarTerrario(uint8_t idx) {
    EstadoTerrario&       est = estado[idx];
    const ConfigTerrario& cfg = CFG[idx];

    float temp = SENSORES[idx]->readTemperature();
    float umid = SENSORES[idx]->readHumidity();

    if (!leituraValida(temp, umid)) {
        est.falhasConsecutivas++;
        est.sensorFalhou = true;

        // Fail-safe: relé abre imediatamente a qualquer leitura inválida.
        est.aquecedorLigado = false;
        setRele(cfg.pinoRele, false);

        Serial.printf("[%s] FALHA sensor #%d — rele aberto (seguranca)\n",
                      NOMES[idx], est.falhasConsecutivas);
        return;
    }

    // Leitura válida — zera contador e limpa flag de falha
    if (est.sensorFalhou) {
        Serial.printf("[%s] Sensor recuperado\n", NOMES[idx]);
        est.sensorFalhou       = false;
        est.falhasConsecutivas = 0;
    }

    est.temp = temp;
    est.umid = umid;

    // Aplica histerese e aciona relé somente se o estado mudou
    bool novoEstado = calcularAquecedor(temp, est.aquecedorLigado, cfg);
    if (novoEstado != est.aquecedorLigado) {
        est.aquecedorLigado = novoEstado;
        setRele(cfg.pinoRele, novoEstado);
        Serial.printf("[%s] Aquecedor %s\n",
                      NOMES[idx], novoEstado ? "LIGADO" : "desligado");
    }

    Serial.printf("[%s] %.1f C  %.0f%%  aquec=%s\n",
                  NOMES[idx], temp, umid,
                  est.aquecedorLigado ? "ON" : "off");

    verificarAlertas(idx);
}

// =============================================================================
// setup() — ordem obrigatória de inicialização
// =============================================================================

void setup() {
    // 1. PRIMEIRO: configurar pinos e colocar relés em estado seguro antes
    //    de qualquer outra instrução — elimina pulso espúrio no boot.
    pinMode(PINO_RELE_T1, OUTPUT);
    pinMode(PINO_RELE_T2, OUTPUT);
    pinMode(PINO_RELE_T3, OUTPUT);
    relesSeguros();

    // 2. Serial
    Serial.begin(115200);
    delay(500);
    Serial.println("\n========================================");
    Serial.println(" Sistema Aranhas — iniciando");
    Serial.println("========================================");
    Serial.printf(" Polaridade: ACTIVE=%s  SAFE=%s\n",
                  RELAY_ACTIVE_LEVEL == LOW ? "LOW" : "HIGH",
                  RELAY_SAFE_LEVEL   == LOW ? "LOW" : "HIGH");

    // 3. Watchdog de hardware — reinicia o ESP32 se o loop() travar.
    //    Ao reiniciar, setup() executa novamente e os relés voltam ao estado seguro.
    //
    //    Se houver erro de compilação aqui, sua versão usa a API do ESP-IDF 5.x:
    //      esp_task_wdt_config_t wdt_cfg = {
    //          .timeout_ms = WATCHDOG_TIMEOUT_S * 1000UL,
    //          .trigger_panic = true
    //      };
    //      esp_task_wdt_reconfigure(&wdt_cfg);
    esp_task_wdt_init(WATCHDOG_TIMEOUT_S, true);
    esp_task_wdt_add(NULL);

    // 4. Sensores DHT22
    dht1.begin();
    dht2.begin();
    dht3.begin();

    // 5. Estado inicial zerado
    for (uint8_t i = 0; i < NUM_TERRARIOS; i++) {
        estado[i] = { 0.0f, 0.0f, false, false, 0 };
    }

    Serial.println(" DHT22: T1=GPIO4  T2=GPIO5  T3=GPIO13");
    Serial.println(" Reles: T1=GPIO25 T2=GPIO26 T3=GPIO27");
    Serial.println(" Aguardando primeira leitura (2s)...");
}

// =============================================================================
// loop()
// =============================================================================

void loop() {
    esp_task_wdt_reset();

    // Leitura cadenciada sem delay() — mantém o loop responsivo para
    // futuras extensões (WebSocket, MQTT) na Fase 2.
    if ((unsigned long)(millis() - ultimaLeitura) >= INTERVALO_LEITURA_MS) {
        ultimaLeitura = millis();
        Serial.println("----------------------------------------");
        for (uint8_t i = 0; i < NUM_TERRARIOS; i++) {
            processarTerrario(i);
        }
    }
}
