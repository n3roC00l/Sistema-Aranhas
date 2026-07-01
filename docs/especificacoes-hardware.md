# Especificações de Hardware — Sistema Aranhas

Referência rápida para montagem e manutenção. Ver `decisoes-arquitetura.md` para o contexto e justificativas.

---

## Componentes

| Qtd | Componente | Observação |
|---|---|---|
| 1 | ESP32 WROOM-32 (D0WDQ6) | Lógica 3.3V |
| 3 | Sensor DHT22 | Um por terrário |
| 1 | Módulo relé 4 canais 5V (SRD-05VDC-SL-C, optoacoplado, com JD-VCC) | Ver seção de ligação abaixo |
| 3 | Tapete aquecedor 5W | Um por terrário |
| 3 | Resistor 10 kΩ | Função depende da polaridade do relé (ver abaixo) |
| 1 | Fonte 5V adequada | Para alimentar ESP32 + bobinas dos relés |

---

## Pinagem ESP32

### Relés (controle dos aquecedores)

| GPIO | Função | Terrário |
|---|---|---|
| 25 | Relé 1 — Aquecedor | T1 — *L. klugi* |
| 26 | Relé 2 — Aquecedor | T2 — *M. balfouri* |
| 27 | Relé 3 — Aquecedor | T3 — *H. maculata* |

**Estado seguro:** aquecedor desligado. O sinal elétrico que representa este estado depende da polaridade do módulo — ver seção "Polaridade pendente de teste".

### Sensores DHT22

| GPIO | Função | Terrário |
|---|---|---|
| 4 | DHT22 dados | T1 — *L. klugi* |
| 5 | DHT22 dados | T2 — *M. balfouri* |
| 13 | DHT22 dados | T3 — *H. maculata* |

GPIOs 4, 5 e 13 são livres de strapping e de SPI interna. Requerem resistor pull-up externo de 10 kΩ para 3.3V na linha de dados do DHT22 (protocolo 1-Wire).

---

## Ligação do módulo relé — solução JD-VCC

O módulo possui pino **JD-VCC** separado do **VCC**. Esta separação é obrigatória neste projeto para resolver a incompatibilidade entre os 3.3V do ESP32 e os 5V esperados pelo optoacoplador.

### Diagrama de alimentação

```
Fonte 5V ──── JD-VCC  (alimenta as bobinas dos relés)
              GND     ──── GND comum (ESP32 + fonte)

ESP32 3.3V ── VCC     (alimenta o lado lógico / optoacopladores)
```

> **Atenção:** remover o jumper que une JD-VCC e VCC antes de ligar. Com o jumper instalado, o lado lógico fica em 5V e o ESP32 não consegue comandar os relés de forma confiável.

### Diagrama de sinal (GPIOs → INx)

```
ESP32 GPIO 25 ──[10kΩ]──── IN1
ESP32 GPIO 26 ──[10kΩ]──── IN2
ESP32 GPIO 27 ──[10kΩ]──── IN3
```

A função do resistor de 10 kΩ (pull-up ou pull-down) e o sentido lógico do sinal dependem da polaridade — ver seção abaixo.

---

## Polaridade pendente de teste

**Teste a realizar quando os componentes chegarem** (antes de gravar o firmware):

1. Alimente o módulo conforme o diagrama JD-VCC acima.
2. Com ESP32 desconectado, toque o pino IN1 no GND. O relé clicou?
   - **Sim → módulo ativo em LOW** (caso mais comum com JD-VCC separado)
   - **Não → toque IN1 no VCC (3.3V) e observe**

### Cenário A — Módulo ativo em LOW (provável)

| Sinal no GPIO | Estado do relé | Estado do aquecedor |
|---|---|---|
| LOW (0V) | Energizado | **Ligado** |
| HIGH (3.3V) | Desenergizado | **Desligado** ← estado seguro |

- Resistores: **pull-up de 10 kΩ para 3.3V** nos GPIOs 25, 26, 27
- Primeira instrução do `setup()`: `digitalWrite(25/26/27, HIGH)`
- No firmware: `RELAY_OFF = HIGH`, `RELAY_ON = LOW`

### Cenário B — Módulo ativo em HIGH

| Sinal no GPIO | Estado do relé | Estado do aquecedor |
|---|---|---|
| HIGH (3.3V) | Energizado | **Ligado** |
| LOW (0V) | Desenergizado | **Desligado** ← estado seguro |

- Resistores: **pull-down de 10 kΩ para GND** nos GPIOs 25, 26, 27
- Primeira instrução do `setup()`: `digitalWrite(25/26/27, LOW)`
- No firmware: `RELAY_OFF = LOW`, `RELAY_ON = HIGH`

O firmware da Fase 1 terá uma constante `RELAY_ACTIVE_LEVEL` que troca entre os dois cenários com uma única linha — sem reescrever a lógica de segurança.

---

## Terrários

- Dimensões: 30 × 15 cm cada
- Posicionamento do aquecedor: face lateral menor (15 cm)
- Efeito: gradiente térmico linear ao longo dos 30 cm (zona quente ↔ zona fria)
- Posicionamento do DHT22: lado oposto ao aquecedor, leitura da zona mais fria

---

## Pinos reservados / proibidos

| GPIO | Motivo para evitar |
|---|---|
| 0 | Strapping de boot |
| 2 | Strapping de boot (LED onboard em algumas placas) |
| 12 | Strapping de boot (define tensão da flash) |
| 15 | Strapping de boot |
| 6–11 | Flash SPI interna — uso causa travamento imediato |
