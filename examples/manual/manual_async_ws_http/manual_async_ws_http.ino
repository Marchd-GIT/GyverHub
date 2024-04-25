#include <Arduino.h>
#include <ESPDMX.h> // для управления по протаколу DMX512
// отключаем встроенную реализацию
#define GH_NO_STREAM
#define GH_NO_HTTP
#define GH_NO_WS
#define GH_NO_MQTT
#define GH_NO_HTTP_TRANSFER
#define GH_INCLUDE_PORTAL
#include <GyverHub.h>
GyverHub hub;

#include <PairsFile.h>
PairsFile fs_data(&LittleFS, "/data.dat", 10000);

#include <ESP8266WiFi.h>

DMXESPSerial dmx;


#include <ESPAsyncTCP.h>
#include <ESPAsyncWebSrv.h>

#define WS_MAX_QUEUED_MESSAGES 8 // просто чтобы понимать склько там их 8 - это умолчания для ESP2866

// ======================= WS =======================
class HubWS : public gh::Bridge {
public:
  HubWS(void* hub) {
    this->config(hub, gh::Connection::WS, GyverHub::parseHook);
    this->server = new AsyncWebServer(81);
    this->aws = new AsyncWebSocket("/");
  }


  void begin() {
    aws->onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
      switch (type) {
        case WS_EVT_CONNECT:
          setFocus();
          break;
        case WS_EVT_DISCONNECT:
          clearFocus();
          break;
        case WS_EVT_DATA:
          clientID = client->id();
          //Serial.println("WS_EVT_DATA");
          parse(sutil::AnyText((char*)data, len));
          break;
        default:
          break;
      }
    });
    server->addHandler(this->aws);
    server->begin();
  }



  void end() {
    aws->closeAll();
  }
  void tick() {
  }
  void send(gh::BridgeData& data) {
    if (data.text.pgm()) {
      char buf[data.text.length()];
      data.text.toStr(buf);
      if (data.broadcast) {
        aws->textAll((uint8_t*)buf, data.text.length());
      } else {
        aws->text(clientID, (uint8_t*)buf, data.text.length());
      }
    } else {
      if (data.broadcast) {
        aws->textAll(data.text.str(), data.text.length());
      } else {
        aws->text(clientID, data.text.str(), data.text.length());
      }
    }
  }

private:
  AsyncWebServer* server;
  AsyncWebSocket* aws;
  uint8_t clientID = 0;
};
HubWS aws(&hub);

// ======================= HTTP =======================
class HubHTTP : public gh::Bridge {
public:
  HubHTTP(void* hub) {
    this->config(hub, gh::Connection::HTTP, GyverHub::parseHook);
    this->server = new AsyncWebServer(GH_HTTP_PORT);
  }

  void begin() {
    server->on("/hub", HTTP_GET, [this](AsyncWebServerRequest* request) {
      String message;
      String req = request->url().substring(5);
      client_p = request;
      parse(req);
      client_p = nullptr;
    });
    server->begin();
  }
  void end() {
    server->end();
  }
  void tick() {
  }
  void send(gh::BridgeData& data) {
    if (client_p){
      AsyncWebServerResponse *response = client_p->beginResponse(200, "text/plain", data.text);
      response->addHeader("Access-Control-Allow-Origin","*");
      response->addHeader("Access-Control-Allow-Private-Network","*");
      response->addHeader("Access-Control-Allow-Methods","*");
      client_p->send(response);
    }


  }
  AsyncWebServer* server;

private:
  // будем хранить указатель на клиента для ответов
  AsyncWebServerRequest* client_p = nullptr;
};
HubHTTP http(&hub);



// ======================= BUILD =======================

// билдер
void build(gh::Builder& b) {
  //b.Title(F("Управление вытяжкой")).size(1,2);
  if (b.beginRow()) {
    if (b.Switch_("button1", &fs_data).size(1, 70).label(F("ВКЛ"))
          .click()) {
      if (fs_data["button1"] == 0) {
        dmx_send(1, 0);
      } else {
        dmx_send(1, fs_data["slider1"]);
      }
    }
    if (b.Slider_("slider1", &fs_data).size(3, 70).label(F("Скорость")).range(100, 200, 5).click()) {
      if (fs_data["button1"] == 1) {
        dmx_send(1, fs_data["slider1"]);
      }
    }
    b.endRow();
  }
}

void setup() {
  Serial.begin(115200);
  dmx.init(512);

  //режим подключения
  char ssid[] = "Alex";
  char pass[] = "alexpass";
  IPAddress device_ip(10, 40, 0, 210);
  IPAddress dns_ip(8, 8, 8, 8);
  IPAddress gateway_ip(10, 40, 0, 1);
  IPAddress subnet_mask(255, 255, 255, 0);
  // подключение к роутеру
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  WiFi.config(device_ip, dns_ip, gateway_ip, subnet_mask);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println(WiFi.localIP());

  //режим точки доступа
  // WiFi.mode(WIFI_AP);
  // WiFi.softAP("MyHub");
  // //Serial.println(WiFi.softAPIP());    // по умолч. 192.168.4.1

  hub.config(F("MyDevices"), F("ESP"), F(""));;
  hub.setVersion("1.0.56");
  hub.onBuild(build);
  hub.addBridge(&http);
  hub.addBridge(&aws);
  hub.begin();
  fs_data.begin();
}

void dmx_send(int ch, int vol) {
  dmx.write(ch, vol);
  dmx.update();
}

void dmx_idle() {
  dmx_send(510, 0);  // отправляем отключение адреса данные чтобы канал не умирал
}

void loop() {
  hub.tick();
  fs_data.tick();
  // =========== ОБНОВЛЕНИЯ ПО ТАЙМЕРУ ===========
  // в библиотеке предусмотрен удобный класс асинхронного таймера
  static gh::Timer tmr(1000);

  // каждую секунду будем обновлять заголовок
  if (tmr) {
    dmx_idle();
  }
}
