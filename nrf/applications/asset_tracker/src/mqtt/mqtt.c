#include <stdio.h>
#include <zephyr.h>
#include <net/socket.h>
#include <power/reboot.h>
#include "mqtt.h"
#include "config.h"
#include "hal/hal_gpio.h"
#include "modem/modem_helper.h"

#if !defined(CONFIG_CLOUD_CLIENT_ID)
#define CLIENT_ID_LEN (MODEM_IMEI_LEN + 4)
#else
#define CLIENT_ID_LEN (sizeof(CONFIG_CLOUD_CLIENT_ID) - 1)
#endif

#define CLOUD_CONNACK_WAIT_DURATION	CONFIG_CLOUD_WAIT_DURATION


#define  MQTT_TOPIC_SIZE    (CLIENT_ID_LEN + 31)

static bool connected = 0;

/* When MQTT_EVT_CONNACK callback enable data sending */
atomic_val_t send_data_enable;

/* When connect mqtt server failed, auto reboot */
static struct k_delayed_work cloud_reboot_work;

/* Buffers for MQTT client. */
static u8_t rx_buffer[CONFIG_MQTT_MESSAGE_BUFFER_SIZE];
static u8_t tx_buffer[CONFIG_MQTT_MESSAGE_BUFFER_SIZE];
static u8_t payload_buf[CONFIG_MQTT_PAYLOAD_BUFFER_SIZE];

extern void mqttGetResponse(void);

static  void iotex_mqtt_get_topic(u8_t *buf, int len) 
{
    snprintf(buf, len, "device/%s/data",iotex_mqtt_get_client_id());     
}
static  void iotex_mqtt_get_config_topic(u8_t *buf, int len) 
{
    snprintf(buf, len, "topic/config/%s",iotex_mqtt_get_client_id());
}
static void iotex_get_heart_beat_topic(u8_t *buf, int len)
{
    snprintf(buf, len, "device/%s/connect",iotex_mqtt_get_client_id());
}


/*
 * @brief Resolves the configured hostname and
 * initializes the MQTT broker structure
 */
static void broker_init(const char *hostname, struct sockaddr_storage *storage) {
    int err;
    struct addrinfo *addr;
    struct addrinfo *result;
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM
    };

    err = getaddrinfo(hostname, NULL, &hints, &result);

    if (err) {
        printk("ERROR: getaddrinfo failed %d\n", err);
        return;
    }

    addr = result;
    err = -ENOENT;

    while (addr != NULL) {
        /* IPv4 Address. */
        if (addr->ai_addrlen == sizeof(struct sockaddr_in)) {
            struct sockaddr_in *broker = ((struct sockaddr_in *)storage);

            broker->sin_family = AF_INET;
            broker->sin_port = htons(CONFIG_MQTT_BROKER_PORT);
            broker->sin_addr.s_addr = ((struct sockaddr_in *)addr->ai_addr)->sin_addr.s_addr;

            printk("IPv4 Address 0x%08x\n", broker->sin_addr.s_addr);
            break;
        }
        else if (addr->ai_addrlen == sizeof(struct sockaddr_in6)) {
            /* IPv6 Address. */
            struct sockaddr_in6 *broker = ((struct sockaddr_in6 *)storage);

            memcpy(broker->sin6_addr.s6_addr,
                   ((struct sockaddr_in6 *)addr->ai_addr)
                   ->sin6_addr.s6_addr,
                   sizeof(struct in6_addr));
            broker->sin6_family = AF_INET6;
            broker->sin6_port = htons(CONFIG_MQTT_BROKER_PORT);

            printk("IPv6 Address");
            break;
        } else {
            printk("error: ai_addrlen = %u should be %u or %u\n",
                   (unsigned int)addr->ai_addrlen,
                   (unsigned int)sizeof(struct sockaddr_in),
                   (unsigned int)sizeof(struct sockaddr_in6));
        }

        addr = addr->ai_next;
        break;
    }

    /* Free the address. */
    freeaddrinfo(result);
}

static int subscribe_config_topic(struct mqtt_client *client) {

    uint8_t topic[MQTT_TOPIC_SIZE];
    iotex_mqtt_get_config_topic(topic, sizeof(topic));
    struct mqtt_topic subscribe_topic = {
        .topic = {
            .utf8 = (uint8_t *)topic,
            .size = strlen(topic)
        },
        .qos = MQTT_QOS_1_AT_LEAST_ONCE
    };

    const struct mqtt_subscription_list subscription_list = {
        .list = &subscribe_topic,
        .list_count = 1,
        .message_id = 1234
    };

    printk("Subscribing to: %s len %u, qos %u\n",
           subscribe_topic.topic.utf8,
           subscribe_topic.topic.size,
           subscribe_topic.qos);

    return mqtt_subscribe(client, &subscription_list);
}

/**@brief Function to print strings without null-termination. */
static void data_print(u8_t *prefix, u8_t *data, size_t len) {
    char buf[len + 1];

    memcpy(buf, data, len);
    buf[len] = 0;
    printk("%s%s\n", prefix, buf);
}

/*
 * @brief Function to read the published payload.
 */
static int publish_get_payload(struct mqtt_client *c, u8_t *write_buf, size_t length) {
    u8_t *buf = write_buf;
    u8_t *end = buf + length;

    if (length > sizeof(payload_buf)) {
        return -EMSGSIZE;
    }

    while (buf < end) {
        int ret = mqtt_read_publish_payload_blocking(c, buf, end - buf);

        if (ret < 0) {
            return ret;
        }
        else if (ret == 0) {
            return -EIO;
        }

        buf += ret;
    }

    return 0;
}


/**@brief Reboot the device if CONNACK has not arrived. */
static void cloud_reboot_work_fn(struct k_work *work) {
    static uint32_t cnt = 0;

    if (++cnt >= CLOUD_CONNACK_WAIT_DURATION) {
        printk("[%s] MQTT Connect timeout reboot!\n", __func__);
        sys_reboot(0);
    }

    printk(".");
    k_delayed_work_submit(&cloud_reboot_work, K_SECONDS(1));
}

/**@brief MQTT client event handler */
static void mqtt_evt_handler(struct mqtt_client *const c, const struct mqtt_evt *evt) {

    int err;

    switch (evt->type) {
        case MQTT_EVT_CONNACK:
            if (evt->result != 0) {
                printk("MQTT connect failed %d\n", evt->result);
                break;
            }

            /* Cancel reboot worker */
            k_delayed_work_cancel(&cloud_reboot_work);
            connected = 1;
            atomic_set(&send_data_enable, 1);

            iotex_hal_gpio_set(LED_BLUE, LED_ON);
            iotex_hal_gpio_set(LED_GREEN, LED_OFF);

            printk("[%s:%d] MQTT client connected!\n", __func__, __LINE__);
            subscribe_config_topic(c);
            break;

        case MQTT_EVT_DISCONNECT:
            connected = 0;
            atomic_set(&send_data_enable, 0);
            iotex_hal_gpio_set(LED_BLUE, LED_OFF);
            iotex_hal_gpio_set(LED_GREEN, LED_ON);
            printk("[%s:%d] MQTT client disconnected %d\n", __func__, __LINE__, evt->result);
            break;

        case MQTT_EVT_PUBLISH: {
            const struct mqtt_publish_param *p = &evt->param.publish;
            err = publish_get_payload(c, payload_buf, p->message.payload.len);

            if (err >= 0) {
                data_print("Received: ", payload_buf, p->message.payload.len);
                iotex_mqtt_update_config(payload_buf, p->message.payload.len);
            }
            else {

                if ((err = mqtt_disconnect(c))) {
                    printk("Could not disconnect: %d\n", err);
                }

                break;
            }

            /* Send acknowledgment. */
            if (p->message.topic.qos == MQTT_QOS_1_AT_LEAST_ONCE) {
                const struct mqtt_puback_param ack = {
                    .message_id = p->message_id
                };

                err = mqtt_publish_qos1_ack(c, &ack);

                if (err) {
                    printk("unable to ack\n");
                }
            }

            break;
        }

        case MQTT_EVT_PUBACK:
            if (evt->result != 0) {
                printk("MQTT PUBACK error %d\n", evt->result);
                break;
            }

            printk("[%s:%d] PUBACK packet id: %u\n", __func__, __LINE__,
                   evt->param.puback.message_id);
            break;

        case MQTT_EVT_SUBACK:
            if (evt->result != 0) {
                printk("MQTT SUBACK error %d\n", evt->result);
                break;
            }

            printk("[%s:%d] SUBACK packet id: %u\n", __func__, __LINE__,
                   evt->param.suback.message_id);
            break;

        default:
            printk("[%s:%d] default: %d\n", __func__, __LINE__, evt->type);
            mqttGetResponse();
            break;
    }
}

bool iotex_mqtt_is_connected(void) {
    return connected;
}

const uint8_t *iotex_mqtt_get_client_id() {
    static u8_t client_id_buf[CLIENT_ID_LEN + 1];
#if !defined(CONFIG_CLOUD_CLIENT_ID)
    snprintf(client_id_buf, sizeof(client_id_buf), "nrf-%s", iotex_modem_get_imei());
#else
    memcpy(client_id_buf, CONFIG_CLOUD_CLIENT_ID, CLIENT_ID_LEN + 1);
#endif /* !defined(NRF_CLOUD_CLIENT_ID) */
    return client_id_buf;
}

int iotex_mqtt_publish_data(struct mqtt_client *client, enum mqtt_qos qos, char *data) {
    struct mqtt_publish_param param;
    u8_t pub_topic[MQTT_TOPIC_SIZE];
    iotex_mqtt_get_topic(pub_topic, sizeof(pub_topic));
    param.message.topic.qos = qos;
    param.message.topic.topic.utf8 = pub_topic;
    param.message.topic.topic.size = strlen(param.message.topic.topic.utf8);

    param.message.payload.data = data;
    param.message.payload.len = strlen(data);
    param.message_id = iotex_random(); 
    param.dup_flag = 0U;
    param.retain_flag = 0U;

    return mqtt_publish(client, &param);
}

int iotex_mqtt_heart_beat(struct mqtt_client *client, enum mqtt_qos qos)
{
     struct mqtt_publish_param param;
    u8_t pub_topic[MQTT_TOPIC_SIZE];
    iotex_get_heart_beat_topic(pub_topic, sizeof(pub_topic));
    param.message.topic.qos = qos;
    param.message.topic.topic.utf8 = pub_topic;
    param.message.topic.topic.size = strlen(param.message.topic.topic.utf8);

    param.message.payload.data = "a";
    param.message.payload.len = 1;
    param.message_id = iotex_random();
    param.dup_flag = 0U;
    param.retain_flag = 0U;

    return mqtt_publish(client, &param);   
}

/**@brief Initialize the MQTT client structure */
int iotex_mqtt_client_init(struct mqtt_client *client, struct pollfd *fds) {

    int err;
    struct sockaddr_storage broker_storage;
    const uint8_t *client_id = iotex_mqtt_get_client_id();
    connected = 0;
    mqtt_client_init(client);

    /* Load mqtt data channel configure */
    iotex_mqtt_load_config();
    broker_init(CONFIG_MQTT_BROKER_HOSTNAME, &broker_storage);

    printk("client_id: %s\n", client_id);

    /* MQTT client configuration */
    client->broker = &broker_storage;
    client->evt_cb = mqtt_evt_handler;
    client->client_id.utf8 = client_id;
    client->client_id.size = strlen(client_id);
    client->password = NULL;
    client->user_name = NULL;
    client->protocol_version = MQTT_VERSION_3_1_1;
    client->unacked_ping = 100;

    /* MQTT buffers configuration */
    client->rx_buf = rx_buffer;
    client->rx_buf_size = sizeof(rx_buffer);
    client->tx_buf = tx_buffer;
    client->tx_buf_size = sizeof(tx_buffer);

    /* MQTT transport configuration */
    if(CONFIG_MQTT_BROKER_PORT == 1884)
        client->transport.type = MQTT_TRANSPORT_NON_SECURE;
    else
        client->transport.type = MQTT_TRANSPORT_SECURE;

    static sec_tag_t sec_tag_list[] = {CONFIG_CLOUD_CERT_SEC_TAG};
    struct mqtt_sec_config *tls_config = &(client->transport).tls.config;

    tls_config->peer_verify = 2;
    tls_config->cipher_list = NULL;
    tls_config->cipher_count = 0;
    tls_config->sec_tag_count = ARRAY_SIZE(sec_tag_list);
    tls_config->sec_tag_list = sec_tag_list;
    tls_config->hostname = CONFIG_MQTT_BROKER_HOSTNAME;

    /* Reboot the device if CONNACK has not arrived in 30s */
    k_delayed_work_init(&cloud_reboot_work, cloud_reboot_work_fn);
    k_delayed_work_submit(&cloud_reboot_work, K_SECONDS(1));
    printk("Before mqtt_connect, connect timeout will reboot in %d seconds\n", CLOUD_CONNACK_WAIT_DURATION);

    if ((err = mqtt_connect(client)) != 0) {
        return err;
    }

    /* Initialize the file descriptor structure used by poll */
    fds->fd = client->transport.tls.sock;
    fds->events = POLLIN;

    return 0;
}
