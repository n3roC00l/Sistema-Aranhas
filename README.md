# Sistema Aranhas — Termorregulação de Terrários

Sistema embarcado de termorregulação e monitoramento automatizado para 3 terrários de tarântulas, localizado em Guarapuava, Paraná, Brasil.

---

## Terrários

| # | Espécie | Nome popular | Temp. alvo | Umidade alvo |
|---|---|---|---|---|
| 1 | *Lasiodora klugi* | Bahia Scarlet | 24–29°C | 60–70% |
| 2 | *Monocentropus balfouri* | Blue Baboon (Socotra) | 24–28°C | 50–60% |
| 3 | *Heteroscodra maculata* | Togo Starburst Baboon | 24–28°C | 60–70% |

---

## Arquitetura

```
firmware/          ESP32 WROOM-32 — leitura DHT22, histerese, fail-safe
    src/           Código-fonte Arduino (PlatformIO)
    include/       Headers e constantes de configuração

backend/           Fase 2 — persistência de histórico (decisão pendente)
frontend/          Fase 3/4 — dashboard web em tempo real + histórico
docs/              Documentação, decisões de arquitetura, protótipo visual
```

O firmware roda a lógica de controle **sempre localmente**, sem dependência de rede. O dashboard é somente leitura — nunca toma decisões de segurança.

---

## Hardware

- **MCU:** ESP32 WROOM-32
- **Sensores:** 3x DHT22 (temperatura + umidade)
- **Atuação:** 3x tapete aquecedor 5W controlados por módulo relé 4 canais
- **Relés:** GPIO 25 (T1), GPIO 26 (T2), GPIO 27 (T3)
- **Estado seguro:** relé desenergizado (aquecedor desligado) em boot e qualquer falha

Ver [`docs/especificacoes-hardware.md`](docs/especificacoes-hardware.md) para pinagem completa.  
Ver [`docs/decisoes-arquitetura.md`](docs/decisoes-arquitetura.md) para todas as decisões técnicas.

---

## Fases de desenvolvimento

| Fase | Descrição | Status |
|---|---|---|
| 0 | Setup: repositório, documentação, decisões de arquitetura | ✅ Concluída |
| 1 | Firmware ESP32: DHT22, histerese individual, fail-safe, watchdog | Pendente |
| 2 | Persistência e comunicação (backend TBD) | Pendente |
| 3 | Dashboard em tempo real | Pendente |
| 4 | Página de histórico por terrário | Pendente |
| 5 | Testes de campo, revisão de segurança, documentação final | Pendente |

---

## Protótipo visual

Um protótipo funcional do dashboard (dados simulados, sem firmware) está disponível em [`docs/prototipo-dashboard.html`](docs/prototipo-dashboard.html). Abra diretamente no navegador para visualizar.
