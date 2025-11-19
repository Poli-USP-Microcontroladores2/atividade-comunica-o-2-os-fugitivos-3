# PSI-Microcontroladores2-Aula10
Atividade: Comunicação UART

# Projeto UART – Atividade em Duplas (Echo Bot + Async API)

## 1. Informações Gerais

* Dupla:

  * Integrante 1: Philip William (NUSP 16897414)
  * Integrante 2: Ricardo Peloso (NUSP ...)

* Objetivo: implementar, testar e documentar aplicações de comunicação UART baseadas nos exemplos oficiais “echo_bot” e “async_api”, utilizando desenvolvimento orientado a testes, diagramas de sequência D2 e registro de evidências.

---

# 2. Estrutura Esperada do Repositório

```
README.md
src/

docs/
  evidence/
  sequence-diagrams/

```

---

# 3. Etapa 1 – Echo Bot (UART Polling/Interrupt)

## 3.1 Descrição do Funcionamento

Descrever aqui de forma textual o comportamento esperado baseado no exemplo oficial.
Link usado como referência:
[https://docs.zephyrproject.org/latest/samples/drivers/uart/echo_bot/README.html](https://docs.zephyrproject.org/latest/samples/drivers/uart/echo_bot/README.html)

## 3.2 Casos de Teste Planejados (TDD)

### CT1 – Eco básico

* Entrada:Ricardo e Philip
* Saída esperada:Ricardo e Philip
* Critério de Aceitação:string com menos de 32 carácteres. contando com new lines e returns 

referência:docs\evidence\echobot\CT1.txt

### CT2 – Linha vazia
* Entrada:
* Saída esperada:

referência:docs\evidence\echobot\CT2.txt

### CT3 – Linha longa

* Entrada:hello we and philip wanted to say hi! :)
* Saída esperada:hello we and philip wanted to say hi! :)
* Saída Real:hello we and philip wanted to

houve o corte da mensagem pois extrapola o limite de 32 chars.

referência:docs\evidence\echobot\CT3.txt

## 3.3 Implementação

* Arquivo(s) modificados: main.c e proj.config
* Justificativa das alterações: implementar/autualizar o código para enviar mensagens no modelo URT por interrupções no zephyr

## 3.4 Evidências de Funcionamento

Salvar evidências em `docs/evidence/echo_bot/`.

Exemplo de referência no README:

```
[Link para o log CT1](docs/evidence/echo_bot/ct1_output.txt)
```

Adicionar aqui pequenos trechos ilustrativos:

```
Hello! I'm your echo bot. Tell me something and press enter:
Echo: Hello World!
```

## 3.5 Diagramas de Sequência D2

Vide material de apoio: https://d2lang.com/tour/sequence-diagrams/

Adicionar arquivos (diagrama completo e o código-base para geração do diagrama) em `docs/sequence-diagrams/`.

---

# 4. Etapa 2 – Async API (Transmissão/Recepção Assíncrona)

## 4.1 Descrição do Funcionamento

Descrever o comportamento esperado de forma textual, especialmente com a alternância TX/RX.
Link usado como referência:
[https://docs.zephyrproject.org/latest/samples/drivers/uart/async_api/README.html](https://docs.zephyrproject.org/latest/samples/drivers/uart/async_api/README.html)

## 4.2 Casos de Teste Planejados (TDD)

### CT1 – Transmissão de pacotes a cada 5s

### CT2 – Recepção

### CT3 – Verificação de timing dos 5s

(Adicionar mais casos se necessário.)

## 4.3 Implementação

* Arquivos modificados: main.c, prj.conf, platformio.ini
* Motivos/Justificativas: No código original, erros aconteciam por conta da falta de suporte DMA do driver MCUX na placa FRDM. Por conta disso, o código até compilava, porém não rodava como o esperado. Desse modo, alterou-se o código para uma espécie de API orientada a interrupção, que contorna a falta de suporte ao Async API na placa FRDM, gerando resultados extremamente semelhantes sem sacrificar a lógica da atividade.

## 4.4 Evidências de Funcionamento

Salvar em `docs/evidence/async_api/`.

Exemplo:

```
Loop 0:
Sending 3 packets (packet size: 5)
Packet: 0
Packet: 1
Packet: 2
```

Ou:

```
RX is now enabled
UART callback: RX_RDY
Data (HEX): 48 65 6C 6C 6F
Data (ASCII): Hello
```

## 4.5 Diagramas de Sequência D2

Vide material de referência: https://d2lang.com/tour/sequence-diagrams/

Adicionar arquivos (diagrama completo e o código-base para geração do diagrama) em `docs/sequence-diagrams/`.

---

# 5. Conclusões da Dupla

* O que deu certo: Conseguimos implementar a lógica de alternância de estado TX/RX do Async API do Zephyr e validar o timing de 5 segundos entre os ciclos, comprovando que o controle assíncrono do fluxo de comunicação está funcionando. O sucesso do projeto foi alcançado ao identificar e contornar as restrições de hardware da placa.
* O que foi mais desafiador: O principal obstáculo foi o diagnóstico de runtime do erro (-134) durante as tentativas de uso do Async API original. Esse erro revelou que a placa FRDM-KL25Z não possuía o suporte de hardware (DMA) necessário para a API Assíncrona. Em vez de utilizar o overlay e forçar o DMA (que gerava erros de compilação), a decisão foi migrar a arquitetura de I/O para a API Orientada a Interrupção como uma forma de contornar a limitação e mesmo assim manter a lógica. Isso resolveu o problema de compatibilidade e permitiu a implementação funcional da lógica assíncrona.
