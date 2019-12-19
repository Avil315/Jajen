//Code for Receiver [Heltech ESPLoRa]

    #include <WiFi.h>
    #include <WiFiClientSecure.h>
    #include <SPI.h>
    #include <LoRa.h>
    #include "SSD1306.h"
    SSD1306  display(0x3c, 4, 15);
    
    #define SS      18
    #define RST     14
    #define DI0     26
    #define BAND    433E6
    
    const char *ssid = "*******"; // wifi ssid
    const char *password = "********";  // wifi password
    const char* host = "script.google.com";  // server name
    const char* fingerprint = "89 ff f4 0f 4d 98 09 ed e3 ef 95 f2 8a af af 22 89 08 ac 03";
    
    String url;
    float h;
    float t;
    String data;
    
    void setup() 
    {
    Serial.begin(115200);
    delay(100);
    
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, password); 
    while (WiFi.status() != WL_CONNECTED) 
    {
    delay(500);
    Serial.print(".");
    }
    Serial.println("");
    Serial.println("*********Credentials accepted!********\n ********WiFi connected********");  
    
    pinMode(16,OUTPUT);
    digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
    delay(50); 
    digitalWrite(16, HIGH);
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    
    while (!Serial); //if just the the basic function, must connect to a computer
    delay(1000);
    Serial.println("LoRa Receiver"); 
    display.drawString(5,5,"LoRa Receiver"); 
    display.display();
    SPI.begin(5,19,27,18);
    LoRa.setPins(SS,RST,DI0);
    
    if (!LoRa.begin(BAND)) {
    display.drawString(5,25,"Starting LoRa failed!");
    while (1);
    }
    Serial.println("LoRa Initial OK!");
    display.drawString(5,25,"LoRa Initializing OK!");
    display.display();
    }
    int counter = 1;
    void loop() 
    { 
    int packetSize = LoRa.parsePacket();
    if(packetSize) {
    // received a packets
    Serial.print("Received packet. ");
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.drawString(3, 0, "Received packet ");
    display.display();
    Serial.println(counter);
    display.drawString(3,10, String(counter));
    counter++;
    while (LoRa.available()) 
    {
    data = LoRa.readString();
    Serial.print(data);
    
    display.drawString(3,20, data);  // show on the oled display
    display.drawString(3,40, "SENDING TO SHEET.........");
    
    display.display();
    delay(1000);
    
    String values=data;
    String t=values.substring(4,8);
    String h=values.substring(14,18);
    //  float h= Humidity.toInt();
    //  float t= Temperature.toInt();
    Serial.print("\nhumidity="+ String(h)+"\n");
    Serial.print("Temperature="+ String(t)+ "\n");
    
    Serial.print("connecting to ");
    Serial.println(host);
    WiFiClientSecure client;
    const int httpPort = 443;
    if (!client.connect(host, httpPort)) 
    {
    Serial.println("connection failed");
    return;
    }
    //url for upload endpoint of google sheets
    url ="https://script.google.com/macros/s/AKfycbz1Umvqrdy0oi9WCjNsSdRFoG2O_sp6HlvgFTJyKBDvdg6BeYY/exec?func=addData&temperature="+ String(t) + "&humidity="+ String(h);
    Serial.print("Requesting URL: ");
    Serial.println(url);
    
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(500);
   Serial.println("Sending result to sheets....");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }
  while(client.available())   // check if connection is established
  { 
    String line = client.readStringUntil('\r');
   // Serial.print(line);
  }
  Serial.println();
  Serial.println("closing connection");
    delay(1000);   
    }
    }
    }
