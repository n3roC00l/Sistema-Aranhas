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

## Histórico de sessões

| Data | Fase | Resultado |
|---|---|---|
| 2026-06-30 | Fase 0 | Setup completo, commit inicial realizado |
