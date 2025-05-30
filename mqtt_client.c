#include "pico/stdlib.h"            // Biblioteca da Raspberry Pi Pico para funções padrão (GPIO, temporização, etc.)
#include "pico/cyw43_arch.h"        // Biblioteca para arquitetura Wi-Fi da Pico com CYW43
#include "pico/unique_id.h"         // Biblioteca com recursos para trabalhar com os pinos GPIO do Raspberry Pi Pico

#include "hardware/gpio.h"          // Biblioteca de hardware de GPIO
#include "hardware/irq.h"           // Biblioteca de hardware de interrupções
#include "hardware/adc.h"           // Biblioteca de hardware para conversão ADC

#include "lwip/apps/mqtt.h"         // Biblioteca LWIP MQTT -  fornece funções e recursos para conexão MQTT
#include "lwip/apps/mqtt_priv.h"    // Biblioteca que fornece funções e recursos para Geração de Conexões
#include "lwip/dns.h"               // Biblioteca que fornece funções e recursos suporte DNS:
#include "lwip/altcp_tls.h"         // Biblioteca que fornece funções e recursos para conexões seguras usando TLS:

#define WIFI_SSID "Baroni VIVO"            // Substitua pelo nome da sua rede Wi-Fi
#define WIFI_PASSWORD "baroni123456789"    // Substitua pela senha da sua rede Wi-Fi
#define MQTT_SERVER "192.168.15.3"         // Substitua pelo endereço do host - broket MQTT: Ex: 192.168.1.107
#define MQTT_USERNAME "lorenzo"            // Substitua pelo nome da host MQTT - Username
#define MQTT_PASSWORD "537301"             // Substitua pelo Password da host MQTT - credencial de acesso - caso exista

// GPIOs usados
#define TCRT_PIN 16        // Exemplo de GPIO do TCRT5000
#define LDR_PIN 28         // GPIO ADC para o LDR
#define LED_R 13
#define LED_G 11
#define LED_B 12

// Tópicos MQTT
#define TOPICO_PRESENCA "/casa/sensor/presenca"
#define TOPICO_LED_STATUS "/casa/led/status"
#define TOPICO_CONTROLE_LEDS "/casa/controle/leds"
#define TOPICO_CONTROLE_SENSOR "/casa/controle/sensor"

// Variáveis globais
mqtt_client_t *mqtt_client;
bool sensor_ativo = true;
bool modo_automatico = true;

// Publica uma mensagem MQTT em um tópico específico
void publicar(const char *topico, const char *mensagem) {
    mqtt_publish(mqtt_client, topico, mensagem, strlen(mensagem), 1, false, NULL, NULL);
}

// Controla os LEDs com base na string recebida
void controle_led(const char *msg) {
    if (strcmp(msg, "vermelho") == 0) {
        gpio_put(LED_R, 1); gpio_put(LED_G, 0); gpio_put(LED_B, 0);
    } else if (strcmp(msg, "verde") == 0) {
        gpio_put(LED_R, 0); gpio_put(LED_G, 1); gpio_put(LED_B, 0);
    } else if (strcmp(msg, "azul") == 0) {
        gpio_put(LED_R, 0); gpio_put(LED_G, 0); gpio_put(LED_B, 1);
    } else {
        gpio_put(LED_R, 0); gpio_put(LED_G, 0); gpio_put(LED_B, 0);
    }
    publicar(TOPICO_LED_STATUS, msg);
}

// Trata os dados recebidos via MQTT
void dados_recebidos_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    char payload[64];
    strncpy(payload, (const char *)data, len);
    payload[len] = '\0';

    // Se for comando de controle dos LEDs (manual)
    if (strcmp((char *)arg, TOPICO_CONTROLE_LEDS) == 0) {
        // Só executa se o modo automático estiver desativado
        if (!modo_automatico) {
            controle_led(payload);
        }

    // Se for comando para ativar/desativar o modo automático
    } else if (strcmp((char *)arg, TOPICO_CONTROLE_SENSOR) == 0) {
        if (strcmp(payload, "on") == 0) {
            modo_automatico = true;
        } else {
            modo_automatico = false;
            controle_led("off");  // desliga o LED ao sair do modo automático
        }
    }
}

// Callback de recebimento de tópico
void topico_recebido_cb(void *arg, const char *topic, u32_t tot_len) {
    (void)arg;
    (void)tot_len;
    mqtt_set_inpub_callback(mqtt_client, topico_recebido_cb, dados_recebidos_cb, (void *)topic);
}

// Quando a conexão MQTT é aceita, assina os tópicos de controle
void conexao_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        mqtt_subscribe(client, TOPICO_CONTROLE_LEDS, 1, NULL, NULL);
        mqtt_subscribe(client, TOPICO_CONTROLE_SENSOR, 1, NULL, NULL);
        mqtt_set_inpub_callback(client, topico_recebido_cb, dados_recebidos_cb, NULL);
    }
}

// Conecta ao Wi-Fi e ao broker MQTT
void conectar_wifi_e_mqtt() {
    cyw43_arch_enable_sta_mode();
    cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000);

    ip_addr_t addr;
    dns_gethostbyname(MQTT_SERVER, &addr, NULL, NULL);

    mqtt_client = mqtt_client_new();
    struct mqtt_connect_client_info_t ci = {
        .client_id = "pico_mqtt",
        .client_user = MQTT_USERNAME,
        .client_pass = MQTT_PASSWORD,
        .keep_alive = 60
    };

    mqtt_client_connect(mqtt_client, &addr, 1883, conexao_cb, NULL, &ci);
}

int main() {
    stdio_init_all();
    cyw43_arch_init();

    gpio_init(LED_R); gpio_set_dir(LED_R, GPIO_OUT);
    gpio_init(LED_G); gpio_set_dir(LED_G, GPIO_OUT);
    gpio_init(LED_B); gpio_set_dir(LED_B, GPIO_OUT);
    gpio_put(LED_R, 0);
    gpio_put(LED_G, 0);
    gpio_put(LED_B, 0);
    gpio_init(TCRT_PIN); gpio_set_dir(TCRT_PIN, GPIO_IN);
    gpio_pull_up(TCRT_PIN);

    adc_init();
    adc_gpio_init(LDR_PIN);

    conectar_wifi_e_mqtt();

    while (1) {
        if (modo_automatico) {
            adc_select_input(2);
            uint16_t ldr = adc_read();

            bool ambiente_escuro = ldr < 3000;
            bool objeto_detectado = gpio_get(TCRT_PIN) == 0;

            if (ambiente_escuro && objeto_detectado) {
                controle_led("vermelho");
                publicar(TOPICO_PRESENCA, "1");
            } else {
                controle_led("off");
                publicar(TOPICO_PRESENCA, "0");
            }
        }
        sleep_ms(1000);
    }
    return 0;
}
