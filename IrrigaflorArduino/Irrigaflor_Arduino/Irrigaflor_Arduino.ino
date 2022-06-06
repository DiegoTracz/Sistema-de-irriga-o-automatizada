/*
   TimeNTP_ESP8266WiFi.ino
   Example showing time sync to NTP time source

   This sketch uses the ESP8266WiFi library
*/

#include <TimeLib.h> //https://github.com/PaulStoffregen/Time
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>

const char* mqtt_server = "11.111.11.111";// COLOQUE AQUI O IP DO SEU SERVIDOR MQTT
const char* mqttUser = "seuUser"; // SEU USUARIO MQTT
const char* mqttPassword = "seuPass"; // SUA SENHA DO USER MQTT

const String MQTT_HOUR_ON_TOPIC = "horaLiga";
const String MQTT_MINUTE_ON_TOPIC = "minutoLiga";
const String MQTT_MINUTE_OFF_TOPIC = "minutoDesl";

int  horaLiga;
int  minutoLiga;
int  horaDesl;
int  minutoDesl;

int hourToMin;
int sumHourMin;
int hourOnToMin;
int sumHourMinOn;
int hourOffToMin;
int sumHourMinOff;

const char ssid[] = "JWIFI";  // SSID da sua rede wi-fi 
const char pass[] = "casa1234";       // Senha da sua rede wi-fi

int Valve = 5;                // Pino Utilizado
boolean stateValve;           // Estado do pino Relay
uint8_t status_auto = true;


/*
-------------------------------------------------
NodeMCU / ESP8266  |  NodeMCU / ESP8266  |
D0 = 16            |  D6 = 12            |
D1 = 5             |  D7 = 13            |
D2 = 4             |  D8 = 15            |
D3 = 0             |  D9 = 3             |
D4 = 2             |  D10 = 1            |
D5 = 14            |                     |
-------------------------------------------------
*/
// NTP Servers:
static const char ntpServerName[] = "us.pool.ntp.org";

const int timeZone = -3;     // Central European Time

WiFiClient espClient;
PubSubClient client(espClient);

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);

void callback(char* topic, byte* payload, unsigned int length)
{
  String payloadStr = "";
  for (int i = 0; i < length; i++)
  {
    payloadStr += (char)payload[i];
  }
  String topicStr = String(topic);

  if (topicStr.equals(MQTT_HOUR_ON_TOPIC))
  {
    horaLiga = payloadStr.toInt();
    Serial.print("Hora Liga: ");
    Serial.println(horaLiga);
  }
  if (topicStr.equals(MQTT_MINUTE_ON_TOPIC))
  {
    minutoLiga = payloadStr.toInt();
    Serial.print("Minuto Liga: ");
    Serial.println(minutoLiga);
  }
  if (topicStr.equals(MQTT_MINUTE_OFF_TOPIC))
  {
    minutoDesl = payloadStr.toInt();
    Serial.print("Minuto desliga: ");
    Serial.println(minutoDesl);
  }
}
void GPIO_handler()
{
  if (digitalRead(Valve) != stateValve)
  {
    digitalWrite(Valve, stateValve);
    Serial.println("*WifiRTC: Estado da GPIO mudou ");
  }
}
void setup()
{
  Serial.begin(9600);
  pinMode(Valve, OUTPUT);
  digitalWrite(Valve, stateValve);
  delay(250);
  Serial.println("TimeNTP Example");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("IP number assigned by DHCP is ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.println("Conectando ao servidor MQTT...");
    if (client.connect("Projetovalv", mqttUser, mqttPassword ))
    {
      Serial.println("Conectado ao servidor MQTT!");
    } else
    {

      Serial.print("Falha ao conectar ");
      Serial.print(client.state());
      delay(2000);

    }
  }
  client.subscribe("horaLiga");
  client.subscribe("minutoLiga");
  client.subscribe("minutoDesl");
}


char hora[30];
char horaLigar[10];
char horaDesligar[10];
void turnOn()
{
  stateValve = true;
}

void turnOff()
{
  stateValve = false;
}
time_t prevDisplay = 0; // when the digital clock was displayed
void TimedAction()
{ //executa
  hourToMin = hour() * 60;
  sumHourMin = hourToMin + minute();
  hourOnToMin = horaLiga * 60;
  sumHourMinOn =  hourOnToMin + minutoLiga;
  hourOffToMin = hourToMin;
  sumHourMinOff = sumHourMinOn + minutoDesl;

  if (status_auto)
  {
    if (sumHourMinOn == sumHourMin)
    {
      turnOn();
    } else if (sumHourMinOff == sumHourMin)
    {
      turnOff();
    }
    GPIO_handler();
  }
  Serial.print("Hora de ligar: ");
  Serial.println(sumHourMinOn);
  Serial.print("Hora de desligar: ");
  Serial.println(sumHourMinOff);
  delay(1000);
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  RTCSoft();
  TimedAction();

}

void RTCSoft()
{
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) {
      prevDisplay = now();
      TimedAction();
    }
  }
}

void printDigits(int digits)
{
  // utility for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

void sendNTPpacket(IPAddress &address)
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
