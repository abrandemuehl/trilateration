#include <Arduino.h>
#include <ArduinoJson.h>
#include <easyMesh.h>
#include <easyWebServer.h>
#include <easyWebSocket.h>

extern "C" {
#include <user_interface.h>
}

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneeky"
#define   MESH_PORT       5555

uint8_t channel = 1;
// Sequence number of a packet from AP to client
uint16_t seq_n = 0;

// Packet buffer
uint8_t packet_buffer[64];

void ICACHE_FLASH_ATTR
sniffer_system_init_done(void);
// globals
easyMesh  mesh;  // mesh global

void setup() {
  Serial.begin( 115200 );

  // setup mesh
  //  mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE | APPLICATION ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP | APPLICATION );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.setReceiveCallback( &receivedCallback );
  mesh.setNewConnectionCallback( &newConnectionCallback );

  // setups webServer
  webServerInit();

  // setup webSocket
  webSocketInit();
  webSocketSetReceiveCallback( &wsReceiveCallback );
  webSocketSetConnectionCallback( &wsConnectionCallback );

  mesh.debugMsg( STARTUP, "\nIn setup() my chipId=%d\n", mesh.getChipId());
    system_init_done_cb(sniffer_system_init_done);


}

void loop() {
  mesh.update();

  static uint16_t previousConnections;
  uint16_t numConnections = mesh.connectionCount();
  if ( countWsConnections() > 0 )
    numConnections++;

  if ( previousConnections != numConnections ) {
    mesh.debugMsg( GENERAL, "loop(): numConnections=%d\n", numConnections);

    sendWsControl();

    previousConnections = numConnections;
  }

}

void newConnectionCallback( bool adopt ) {
  if ( adopt == false ) {
    String control = buildControl();
    mesh.sendBroadcast( control );
  }
}

void receivedCallback( uint32_t from, String &msg ) {
  mesh.debugMsg( APPLICATION, "receivedCallback():\n");

  DynamicJsonBuffer jsonBuffer(50);
  JsonObject& control = jsonBuffer.parseObject( msg );

  broadcastWsMessage(msg.c_str(), msg.length(), OPCODE_TEXT);

  mesh.debugMsg( APPLICATION, "control=%s\n", msg.c_str());
}

void wsConnectionCallback( void ) {
  mesh.debugMsg( APPLICATION, "wsConnectionCallback():\n");
}

void wsReceiveCallback( char *payloadData ) {
  mesh.debugMsg( APPLICATION, "wsReceiveCallback(): payloadData=%s\n", payloadData );

  String msg( payloadData );
  mesh.sendBroadcast( msg );

  if ( strcmp( payloadData, "wsOpened") == 0) {  // hack to give the browser time to get the ws up and running
    mesh.debugMsg( APPLICATION, "wsReceiveCallback(): received wsOpened\n" );
    sendWsControl();
    return;
  }

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& control = jsonBuffer.parseObject(payloadData);

  if (!control.success()) {   // Test if parsing succeeded.
    mesh.debugMsg( APPLICATION, "wsReceiveCallback(): parseObject() failed. payload=%s<--\n", payloadData);
    return;
  }
}

void sendWsControl( void ) {
  mesh.debugMsg( APPLICATION, "sendWsControl():\n");

  String control = buildControl();
  broadcastWsMessage(control.c_str(), control.length(), OPCODE_TEXT);
}

String buildControl ( void ) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& control = jsonBuffer.createObject();

  String ret;
  control.printTo(ret);
  return ret;
}
struct RxControl {
  signed rssi:8;
  unsigned rate:4;
  unsigned is_group:1;
  unsigned:1;
  unsigned sig_mode:2;
  unsigned legacy_length:12;
  unsigned damatch0:1;
  unsigned damatch1:1;
  unsigned bssidmatch0:1;
  unsigned bssidmatch1:1;
  unsigned MCS:7;
  unsigned CWB:1;
  unsigned HT_length:16;
  unsigned Smoothing:1;
  unsigned Not_Sounding:1;
  unsigned:1;
  unsigned Aggregation:1;
  unsigned STBC:2;
  unsigned FEC_CODING:1;
  unsigned SGI:1;
  unsigned rxend_state:8;
  unsigned ampdu_cnt:8;
  unsigned channel:4;
  unsigned:12;
};

struct LenSeq {
  uint16_t length;
  uint16_t seq;
  uint8_t  address3[6];
};

struct sniffer_buf {
  struct RxControl rx_ctrl;
  uint8_t buf[36];
  uint16_t cnt;
  struct LenSeq lenseq[1];
};

struct sniffer_buf2{
  struct RxControl rx_ctrl;
  uint8_t buf[112];
  uint16_t cnt;
  uint16_t len;
};


int rssi = -1000;
/* Listens communication between AP and client */
static void ICACHE_FLASH_ATTR
promisc_cb(uint8_t *buf, uint16_t len)
{
  if (len == 12){
    struct RxControl *sniffer = (struct RxControl*) buf;
    if(sniffer->rssi > rssi) rssi = sniffer->rssi;
  } else if (len == 128) {
    struct sniffer_buf2 *sniffer = (struct sniffer_buf2*) buf;
    if(sniffer->rx_ctrl.rssi > rssi) rssi = sniffer->rx_ctrl.rssi;
  } else {
    struct sniffer_buf *sniffer = (struct sniffer_buf*) buf;
    if(sniffer->rx_ctrl.rssi > rssi) rssi = sniffer->rx_ctrl.rssi;
  }
  os_printf("Max rssi: %d\n", rssi);
}

void ICACHE_FLASH_ATTR
sniffer_system_init_done(void)
{
  // Set up promiscuous callback
  wifi_set_channel(channel);
  wifi_promiscuous_enable(0);
  wifi_set_promiscuous_rx_cb(promisc_cb);
  wifi_promiscuous_enable(1);
}



