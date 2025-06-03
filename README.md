# Automação Residencial com MQTT

Projeto desenvolvido com o objetivo de aplicar o protocolo MQTT em um sistema embarcado funcional de automação residencial.

## 🎯 Objetivo
Controlar LEDs RGB simulando dispositivos de uma casa inteligente, com acionamento automático (via sensores) ou manual (via comandos MQTT).

## 📷 Demonstração

[🔗 Link para o vídeo](https://youtu.be/j1ebMUvKqs8)

## 🔧 Funcionalidades
- Detecção de presença com sensor TCRT5000
- Condicionamento por luminosidade (LDR)
- Controle de LEDs RGB com dois modos:
  - Modo automático: sensores ativam LEDs
  - Modo manual: comandos via app
- Comunicação MQTT:
  - Publicação e assinatura de tópicos
  - Conectado a broker local (Mosquitto)

## 🧩 Periféricos utilizados
- ✅ TCRT5000 (presença)
- ✅ LDR (luminosidade)
- ✅ LEDs RGB

## 🛠️ Plataforma
- Placa: BitDogLab (RP2040)
- Linguagem: C com SDK do Raspberry Pi Pico
- Broker: Mosquitto no PC (Windows)
- Interface: IoT MQTT Panel (emulador Android)

## 📡 Tópicos MQTT

| Função                  | Tópico                    | Direção     |
|-------------------------|---------------------------|-------------|
| Presença detectada      | `/casa/sensor/presenca`   | Publish     |
| Status do LED           | `/casa/led/status`        | Publish     |
| Controle de LEDs        | `/casa/controle/leds`     | Subscribe   |
| Ativar/desativar sensor | `/casa/controle/sensor`   | Subscribe   |

## 📲 Interface MQTT
- Criada com o IoT MQTT Panel (emulador)
- Switch para ativar sensor
- Botões para controlar cor dos LEDs
- Indicador visual de presença

## 📝 Licença
Este programa foi desenvolvido como um exemplo educacional e pode ser usado livremente para fins de estudo e aprendizado.

## 📌 Autor
LORENZO GIUSEPPE OLIVEIRA BARONI
