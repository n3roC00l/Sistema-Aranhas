# Especificações de Hardware — Sistema Aranhas

Referência rápida para montagem e manutenção. Ver `decisoes-arquitetura.md` para o contexto e justificativas.

---

## Componentes

| Qtd | Componente | Observação |
|---|---|---|
| 1 | ESP32 WROOM-32 (D0WDQ6) | Lógica 3.3V |
| 3 | Sensor DHT22 | Um por terrário |
| 1 | Módulo relé 4 canais 5V | Lógica ativa em HIGH |
| 3 | Tapete aquecedor 5W | Um por terrário |
| 3 | Resistor pull-down 10 kΩ | Nos pinos de controle dos relés |
| 1 | Fonte 5V adequada | Para alimentar ESP32 + módulo relé |

---

## Pinagem ESP32

### Relés (controle dos aquecedores)

| GPIO | Função | Estado seguro | Terrário |
|---|---|---|---|
| 25 | Relé 1 — Aquecedor | LOW (desligado) | T1 — *L. klugi* |
| 26 | Relé 2 — Aquecedor | LOW (desligado) | T2 — *M. balfouri* |
| 27 | Relé 3 — Aquecedor | LOW (desligado) | T3 — *H. maculata* |

### Sensores DHT22

> Pinos de dados ainda a definir na Fase 1. Evitar GPIO 0, 2, 12, 15 (strapping) e GPIO 6–11 (flash SPI). Candidatos: GPIO 32, 33, 34 (entrada analógica, mas usável como input digital com pull-up externo) ou GPIO 4, 5, 13.

| GPIO | Função | Terrário |
|---|---|---|
| A definir | DHT22 dados | T1 — *L. klugi* |
| A definir | DHT22 dados | T2 — *M. balfouri* |
| A definir | DHT22 dados | T3 — *H. maculata* |

---

## Diagrama de ligação do módulo relé

```
ESP32 GPIO 25 ──[10kΩ pull-down]──┬── IN1 (módulo relé)
                                   └── GND

ESP32 GPIO 26 ──[10kΩ pull-down]──┬── IN2 (módulo relé)
                                   └── GND

ESP32 GPIO 27 ──[10kΩ pull-down]──┬── IN3 (módulo relé)
                                   └── GND

VCC módulo relé ── 5V
GND módulo relé ── GND comum
```

**Importante:** O pull-down de 10 kΩ garante que o pino fique em LOW antes do ESP32 inicializar, evitando pulso espúrio no relé durante o boot.

---

## Terrários

- Dimensões: 30 × 15 cm cada
- Posicionamento do aquecedor: face lateral menor (15 cm)
- Efeito: gradiente térmico linear ao longo dos 30 cm (zona quente ↔ zona fria)
- Posicionamento do DHT22: lado oposto ao aquecedor, para leitura representativa da zona mais fria

---

## Pinos reservados / proibidos

| GPIO | Motivo para evitar |
|---|---|
| 0 | Strapping de boot |
| 2 | Strapping de boot (também LED onboard em algumas placas) |
| 12 | Strapping de boot (nível de tensão flash) |
| 15 | Strapping de boot |
| 6–11 | Flash SPI interna — uso causa instabilidade imediata |
