# Decisões de Arquitetura — Sistema Aranhas

Documento de referência para todas as decisões técnicas já tomadas e ratificadas. Qualquer alteração nestas decisões exige justificativa técnica explícita e aprovação do mantenedor.

---

## 1. Princípio orientador

**O bem-estar biológico dos animais tem prioridade máxima** sobre qualquer decisão de arquitetura, estética ou conveniência técnica. Toda decisão deve ser avaliada primeiro sob esta ótica.

---

## 2. Hardware (não renegociar sem justificativa técnica)

| Componente | Escolha | Justificativa |
|---|---|---|
| MCU | ESP32 WROOM-32 (chip D0WDQ6) | Capacidade Wi-Fi, GPIO suficiente, suporte PlatformIO maduro |
| Sensores | 3x DHT22 | Temperatura + umidade em um sensor; precisão adequada para husbandry |
| Atuação | Módulo relé 4 canais 5V | 1 relé por terrário + 1 de reserva; isolação lógica/potência |
| Aquecedores | 3x tapetes 5W | Um por face lateral menor do terrário; cria gradiente linear ao longo dos 30cm |

### Pinagem dos relés

| Relé | GPIO | Terrário |
|---|---|---|
| Relé 1 | GPIO 25 | Terrário 1 — Lasiodora klugi |
| Relé 2 | GPIO 26 | Terrário 2 — Monocentropus balfouri |
| Relé 3 | GPIO 27 | Terrário 3 — Heteroscodra maculata |

**Pinos evitados:** GPIO 0, 2, 12, 15 (strapping de boot) e GPIO 6–11 (flash SPI interna). GPIOs 25/26/27 não possuem função de strapping.

---

## 3. Regras de segurança inegociáveis

### 3.1 Estado seguro = relé desenergizado

O aquecedor deve estar **desligado** em todos os cenários de falha:
- Boot do ESP32 (antes de qualquer leitura)
- Leitura de sensor retorna valor nulo ou inconsistente
- Perda de comunicação com backend/dashboard
- Watchdog timeout

**Implementação obrigatória:**
- Pull-down físico de 10 kΩ nos pinos de controle dos relés
- Primeira instrução do `setup()`: escrita explícita de `LOW` em todos os pinos de relé
- Módulo relé com lógica ativa em HIGH (relé energiza com sinal HIGH)

### 3.2 Fail-safe e histerese são locais

A lógica de controle térmico (histerese) e o fail-safe **rodam sempre no firmware do ESP32**, sem dependência de rede, dashboard ou backend. Se a conexão cair, o sistema continua funcionando corretamente.

### 3.3 Comandos remotos são sugestões

Ajustes de setpoint ou qualquer comando remoto (via WebSocket, MQTT, etc.) são tratados como **sugestão** sujeita às mesmas validações de segurança do loop local. Nunca são um override direto de hardware.

---

## 4. Setpoints por espécie

Valores de referência (fontes públicas de husbandry). **Validar com o mantenedor antes de travar como limite rígido no firmware.**

| Terrário | Espécie | Temp. alvo | Umidade alvo | Alerta de umidade |
|---|---|---|---|---|
| 1 | *Lasiodora klugi* | 24–29°C | 60–70% | Déficit |
| 2 | *Monocentropus balfouri* | 24–28°C | 50–60% | **Excesso e déficit** |
| 3 | *Heteroscodra maculata* | 24–28°C | 60–70% | Déficit |

O terrário 2 é a exceção: origem árida (Socotra), sensível à superumidificação. A lógica de alerta deve ser **individual por terrário**, nunca global.

---

## 5. Estrutura de fases

| Fase | Conteúdo | Status |
|---|---|---|
| 0 | Setup: estrutura de repositório, documentação inicial | ✅ Concluída |
| 1 | Firmware ESP32: DHT22, histerese, fail-safe, watchdog | Pendente |
| 2 | Persistência e comunicação (decisão em aberto) | Pendente |
| 3 | Dashboard em tempo real | Pendente |
| 4 | Página de histórico | Pendente |
| 5 | Testes de campo, revisão de segurança, documentação final | Pendente |

**Regra:** nenhuma fase começa sem aprovação explícita do mantenedor.

---

## 6. Decisões em aberto

### Fase 2 — Persistência de histórico

Ainda não definida. Opções a avaliar quando o hardware auxiliar disponível for informado:

- **(a) Backend local** em Raspberry Pi ou mini-PC já ligado 24/7, recebendo dados via WebSocket/MQTT e persistindo em SQLite ou banco timeseries.
- **(b) Módulo SD card** conectado diretamente ao ESP32 (limitação: requer acesso físico para consulta).

A decisão depende do hardware auxiliar disponível na residência do mantenedor.
