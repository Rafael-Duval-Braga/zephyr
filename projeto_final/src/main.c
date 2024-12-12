/*
 * Copyright (c) 2024 UFSM
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/net/openthread.h>
#include <openthread/thread.h>
#include <openthread/udp.h>
#include <stdio.h>
#include <string.h>

#define AT30TSE75X_ADDR 0x4F
#define AT30TSE75X_REG_REA 0x00
#define I2C_DEV_NAME DT_NODELABEL(sercom1)

#define SLEEP_TIME K_MSEC(1000)

#define COAP_PORT 5683
#define REMOTE_IP "ff03::1" // Multicast address

static otInstance *ot_instance;
static otUdpSocket udp_socket;

// Função para enviar dados pela rede Thread
void send_temperature(int8_t temperature) {
    char message[32];
    otMessageInfo message_info;
    otMessage *udp_message;

    // Formata a string para envio
    snprintf(message, sizeof(message), "Temperatura: %d C", temperature);

    // Prepara as informações do destinatário
    memset(&message_info, 0, sizeof(message_info));
    otIp6AddressFromString(REMOTE_IP, &message_info.mPeerAddr);
    message_info.mPeerPort = COAP_PORT;

    // Cria uma nova mensagem UDP
    udp_message = otUdpNewMessage(ot_instance, NULL);
    if (udp_message == NULL) {
        printf("Falha ao criar mensagem UDP.\n");
        return;
    }

    // Adiciona o payload
    otMessageAppend(udp_message, message, strlen(message));

    // Envia a mensagem UDP
    otError error = otUdpSend(&udp_socket, udp_message, &message_info);
    if (error != OT_ERROR_NONE) {
        printf("Falha ao enviar mensagem UDP: %d\n", error);
        otMessageFree(udp_message);
    } else {
        printf("Mensagem enviada: %s\n", message);
    }
}

int main(void) {
    const struct device *i2c_dev = DEVICE_DT_GET(I2C_DEV_NAME);

    if (i2c_dev == NULL || !device_is_ready(i2c_dev)) {
        printf("Não foi possível iniciar o dispositivo I2C!\n");
        return 0;
    }

    printf("Sensor pronto! Início da leitura:\n");

    // Inicializa a instância do OpenThread
    ot_instance = openthread_get_default_instance();
    if (ot_instance == NULL) {
        printf("Falha ao obter instância do OpenThread.\n");
        return 0;
    }

    // Configura o socket UDP
    memset(&udp_socket, 0, sizeof(udp_socket));
    otUdpOpen(ot_instance, &udp_socket, NULL, NULL);

    // Ativa o Thread
    otThreadSetEnabled(ot_instance, true);

    for (;;) {
        uint8_t temperature;
        int ret = i2c_reg_read_byte(i2c_dev, AT30TSE75X_ADDR, AT30TSE75X_REG_REA, &temperature);

        if (ret != 0) {
            printf("Erro leitura I2C: %d\n", ret);
            k_sleep(SLEEP_TIME);
            continue;
        }

        // Corrige a leitura de temperatura para valores válidos
        if (temperature > 127 || temperature < -128) {
            printf("Leitura inválida: %d\n", temperature);
        } else {
            printf("Temperatura: %d C\n", temperature);

            // Envia a temperatura via Thread
            send_temperature(temperature);
        }

        k_sleep(SLEEP_TIME);
    }

    return 0;
}
