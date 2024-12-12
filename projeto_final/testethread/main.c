#include <zephyr.h>
#include <net/openthread.h>
#include <openthread/thread.h>
#include <openthread/coap.h>
#include <string.h>

#define COAP_PORT 5683
#define REMOTE_IP "ff03::1"  // IP multicast do Thread

static otInstance *ot_instance;
static otUdpSocket udp_socket;

void udp_send(const char *msg)
{
    otMessageInfo message_info;
    otMessage *message;

    memset(&message_info, 0, sizeof(message_info));
    otIp6AddressFromString(REMOTE_IP, &message_info.mPeerAddr);
    message_info.mPeerPort = COAP_PORT;

    message = otUdpNewMessage(ot_instance, NULL);
    otMessageAppend(message, msg, strlen(msg));

    otUdpSend(&udp_socket, message, &message_info);
}

void main(void)
{
    const char *message = "Hello from SAM R21!";
    ot_instance = openthread_get_default_instance();

    // Inicia a pilha Thread
    otThreadSetEnabled(ot_instance, true);

    // Configura socket UDP
    memset(&udp_socket, 0, sizeof(udp_socket));
    otUdpOpen(ot_instance, &udp_socket, NULL, NULL);

    // Envia a mensagem
    udp_send(message);

    printk("Message sent: %s\n", message);
}
