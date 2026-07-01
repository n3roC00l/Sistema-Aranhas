# PROGRESS.md — Sistema Aranhas

Arquivo de continuidade entre sessões de trabalho. Atualizar ao final de cada sessão, mesmo que a fase não esteja concluída.

---

## Sessão 2026-06-30 — Fase 0: Setup

### O que foi feito

- Recebido e analisado o prompt de inicialização completo do projeto.
- Verificado estado do repositório GitHub (`n3roC00l/Sistema-Aranhas`): estava vazio.
- Verificado scaffold PlatformIO local: já presente com `board = upesy_wroom`.
- Consultado repositório de skills (`anthropics/skills`): identificadas `frontend-design` (Fase 3) e `webapp-testing` (Fase 5) como relevantes.
- Criada estrutura completa de diretórios e arquivos da Fase 0:
  - `README.md`
  - `PROGRESS.md` (este arquivo)
  - `docs/decisoes-arquitetura.md`
  - `docs/especificacoes-hardware.md`
  - `docs/prototipo-dashboard.html` (protótipo validado pelo usuário)
  - `backend/.gitkeep` (placeholder Fase 2)
  - `frontend/.gitkeep` (placeholder Fase 3/4)
  - `.gitignore` atualizado com padrões de frontend e backend
- Primeiro commit e push ao repositório GitHub realizados.

### Decisões técnicas tomadas

Todas registradas em `docs/decisoes-arquitetura.md`. Resumo:
- Firmware permanece na raiz do projeto PlatformIO (não movido para subdiretório `firmware/`), por compatibilidade com a extensão PlatformIO no VS Code.
- Estado seguro = relé desenergizado, sempre — no boot e em qualquer falha.
- Fail-safe e histerese são 100% locais no ESP32, sem dependência de rede.
- Setpoints são individuais por terrário (não globais).

### Pendente / próximo passo

**Fase 1 — Firmware ESP32** (aguardando aprovação do usuário para iniciar):
- Definir pinos de dados dos DHT22 (candidatos: GPIO 4, 5, 13 ou 32/33).
- Implementar leitura dos 3x DHT22 com validação de leitura.
- Implementar lógica de histerese individual por terrário.
- Implementar fail-safe: relé abre imediatamente em leitura nula/inconsistente.
- Implementar watchdog de hardware.
- Configurar `platformio.ini` com biblioteca DHT e parâmetros de build.

### Suposições que precisam ser validadas

1. **Setpoints como limites rígidos:** os valores de temperatura e umidade são pontos de partida — confirmado pelo mantenedor que serão validados antes de travar no firmware. ✅
2. **Pinos DHT22:** definidos — GPIO 4 (T1), GPIO 5 (T2), GPIO 13 (T3). Confirmado pelo mantenedor. ✅
3. **Polaridade do módulo relé (crítico — bloqueia gravação do firmware):** módulo identificado como SRD-05VDC-SL-C com optoacoplador e pino JD-VCC. Solução de compatibilidade 3.3V/5V: separar JD-VCC (→ 5V) do VCC (→ 3.3V ESP32). Polaridade (ativo em HIGH ou LOW) ainda não confirmada — componentes ainda não chegaram. Teste descrito em `docs/especificacoes-hardware.md`. O firmware será desenvolvido com constante `RELAY_ACTIVE_LEVEL` configurável para não bloquear o desenvolvimento.

---

---

## Sessão 2026-06-30 — Fase 1: Firmware ESP32

### O que foi feito

- Atualizado `platformio.ini` com dependências DHT + Adafruit Unified Sensor e monitor_speed.
- Criado `include/config.h` com todas as constantes configuráveis:
  - Pinos de relé (GPIO 25/26/27) e DHT22 (GPIO 4/5/13)
  - Constante `RELAY_ACTIVE_LEVEL` / `RELAY_SAFE_LEVEL` para configurar polaridade sem reescrever a lógica
  - Setpoints individuais por terrário (tempMin/Max, histerese, umidMin/Max)
  - Limites de validação de leitura e temporização
- Criado `src/main.cpp` com firmware completo:
  - Primeiro passo do `setup()`: pinos como OUTPUT + `relesSeguros()` — elimina pulso no boot
  - Watchdog de hardware (10s) com reinício + volta ao estado seguro automaticamente
  - Leitura não-bloqueante com `millis()` — loop livre para futuras extensões (Fase 2)
  - Validação de leitura DHT22: NaN, fora de range físico → relé abre imediatamente
  - Histerese individual por terrário (banda morta: liga abaixo de tempMin-H, desliga acima de tempMax+H)
  - Alertas de umidade individuais: T1/T3 alertam por déficit; T2 (balfouri) alerta por déficit E excesso
  - Saída Serial estruturada pronta para integração com Fase 2

### Decisões técnicas tomadas

- **Polaridade do relé:** implementada com `RELAY_ACTIVE_LEVEL = LOW` (provável) e `RELAY_SAFE_LEVEL = HIGH`. Para inverter, basta trocar os dois valores em `config.h` — nenhuma outra linha muda.
- **Fail-safe:** qualquer leitura inválida (NaN ou fora de range) abre o relé imediatamente, sem aguardar múltiplas falhas. Segurança dos animais tem prioridade sobre disponibilidade do aquecedor.
- **Loop não-bloqueante:** uso de `millis()` em vez de `delay()` prepara o loop para receber código de rede (WebSocket/MQTT) na Fase 2 sem refatoração.
- **Watchdog:** ao ser disparado, o ESP32 reinicia e `setup()` volta a executar — os relés são colocados em estado seguro automaticamente, sem intervenção humana.
- **Nota de compatibilidade watchdog:** se a versão ESP32 Arduino 3.x (ESP-IDF 5.x) gerar erro de compilação no `esp_task_wdt_init()`, substituir pelo bloco comentado no próprio código.

### Pendente / próximo passo

**Validação antes de gravar o firmware no hardware real:**
1. Testar polaridade do módulo relé (procedimento em `docs/especificacoes-hardware.md`) e atualizar `RELAY_ACTIVE_LEVEL` em `config.h` se necessário.
2. Verificar resistores pull-up de 10 kΩ na linha de dados dos DHT22 (GPIO 4/5/13).
3. Compilar e gravar; monitorar Serial a 115200 baud para confirmar leituras válidas.

**Fase 2 — Persistência e comunicação** (aguardando aprovação e definição de hardware auxiliar disponível):
- Definir se o histórico será armazenado em backend local (Raspberry Pi / mini-PC) ou módulo SD card.
- O usuário informará qual hardware tem disponível para decidir a arquitetura.

### Suposições que precisam ser validadas

1. **Polaridade do relé:** `RELAY_ACTIVE_LEVEL = LOW`. Confirmar com teste físico.
2. **API do watchdog:** código usa `esp_task_wdt_init(segundos, panic)` — compatível com ESP-IDF 4.x. Se usar ESP32 Arduino 3.x, usar API comentada no código.

---

## Histórico de sessões

| Data | Fase | Resultado |
|---|---|---|
| 2026-06-30 | Fase 0 | Setup completo, commit inicial realizado |
| 2026-06-30 | Fase 1 | Firmware completo: config.h, main.cpp, platformio.ini |
