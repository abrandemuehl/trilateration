


```C
struct WSConnection {
  uint8_t status;
  struct espconn* connection;
  WSOnMessage onMessage;
};

```

```
struct ip_addr {
    uint32 addr;
};

typedef struct {
    station_info *next;
    uint8_t bssid[6]; // MAC address
    struct ip_addr ip;
} station_info;
station_info *wifi_softap_get_station_info();
```

```C
typedef struct _esp_tcp {
    int remote_port;      /**< remote port of TCP connection            */
    int local_port;       /**< ESP8266's local port of TCP connection   */
    uint8 local_ip[4];    /**< local IP of ESP8266                      */
    uint8 remote_ip[4];   /**< remote IP of TCP connection              */
    espconn_connect_callback connect_callback;      /**< connected callback */
    espconn_reconnect_callback reconnect_callback;  /**< as error handler, the TCP connection broke unexpectedly */
    espconn_connect_callback disconnect_callback;   /**< disconnected callback */
    espconn_connect_callback write_finish_fn;       /**< data send by espconn_send has wrote into buffer waiting for sending, or has sent successfully */
} esp_tcp;
```
```C
typedef struct _esp_udp {
    int remote_port;    /**< remote port of UDP transmission            */
    int local_port;     /**< ESP8266's local port for UDP transmission  */
    uint8 local_ip[4];  /**< local IP of ESP8266                        */
    uint8 remote_ip[4]; /**< remote IP of UDP transmission              */
} esp_udp;
```

```C
struct espconn {
    enum espconn_type type;     /**< type of the espconn (TCP or UDP)   */
    enum espconn_state state;   /**< current state of the espconn       */
    union {
        esp_tcp *tcp;
        esp_udp *udp;
    } proto;
    espconn_recv_callback recv_callback;    /**< data received callback */
    espconn_sent_callback sent_callback;    /**< data sent callback     */
    uint8 link_cnt;         /**< link count             */
    void *reserve;          /**< reserved for user data */
};
```


