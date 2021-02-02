//Include Libraries
#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <LiquidCrystal_I2C.h>

#define relay_pin D7  
#define led D3
#define rst D0

//SSID & Password
#define WLAN_SSID       "..Your WiFi Name.."
#define WLAN_PASS       "..Your WiFi Pass.."

//Server Authentication
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "..Your Adafruit Username.."
#define AIO_KEY         "..Your Adafruit Key.."

LiquidCrystal_I2C lcd(0x27, 16, 4);

//Create Client Objects
WiFiClient client;

//MQTT Object
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

//Get a subscribe for Relay
Adafruit_MQTT_Subscribe Relay = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME"/feeds/Relay");

//Check Connection
void MQTT_connect()
{
  int8_t ret;

  if (mqtt.connected())
  {
    return;
  }

  lcd.clear();
  Serial.print("Connecting to MQTT... ");
  lcd.print("MQTT Connecting");

  uint8_t retries = 3;

  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
    retries--;
    if (retries == 0) {
      ESP.reset();
    }
  }
  lcd.clear();
  Serial.println("MQTT Connected!");
  lcd.print("MQTT Connected!");

}

void setup() {
  Serial.begin(115200);

  pinMode(relay_pin, OUTPUT);
  pinMode(led,OUTPUT);
  pinMode(rst,INPUT);

  digitalWrite(led,HIGH);
  digitalWrite(relay_pin, HIGH);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);

  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  lcd.print("WiFi Connecting");

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  lcd.clear();
  Serial.println("WiFi connected");
  lcd.print("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //Subscribe to Light
  mqtt.subscribe(&Relay);
}

void loop() 
{
  //Check Connection
  MQTT_connect();

  if(digitalRead(rst) == 1)
  {
    digitalWrite(led,LOW);
    delay(500);
    ESP.reset();
  }

  lcd.setCursor(0,1);
  Adafruit_MQTT_Subscribe *subscription;

  //Read all the subscriptions in loop
  while ((subscription = mqtt.readSubscription(1))) 
  {
    if (subscription == &Relay) 
    {
      //Print Recieved Value
      Serial.printf("Got: ");
      lcd.print("Power : ");
      Serial.println((char *)Relay.lastread);

      //Read Character
      String State = (char *)Relay.lastread;
      Serial.print("State : ");Serial.println(State);

      //Relay ON
      if(State == "1"){
        lcd.print("   ");
        lcd.setCursor(8,1);
        lcd.print("On");
        digitalWrite(relay_pin, LOW);
      }
      else{
        lcd.print("   ");
        lcd.setCursor(8,1);
        lcd.print("OFF");
        digitalWrite(relay_pin, HIGH);
      }
    }
  }
}
