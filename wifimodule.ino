/*
    HTTP over TLS (HTTPS) example sketch

    This example demonstrates how to use
    WiFiClientSecure class to access HTTPS API.
    We fetch and display the status of
    esp8266/Arduino project continuous integration
    build.

    Limitations:
      only RSA certificates
      no support of Perfect Forward Secrecy (PFS)
      TLSv1.2 is supported since version 2.4.0-rc1

    Created by Ivan Grokhotkov, 2015.
    This example is in public domain.
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "MATTOS220";
const char* password = "mattost14";

//const char* host = "http://192.168.0.102";
IPAddress host(192,168,0,102);
const int httpsPort = 5000;
const String url="http://192.168.0.2:5000/uploadmyweight/";
String peso;
char data[100];

HTTPClient http;

// Use web browser to view and copy
bool wifiConnected, newData=false;
const byte numChars = 50;
char receivedChars[numChars];


void setup() {
  Serial.begin(115200);
  wifiConnected = false;
  if(wifiConnection())
    wifiConnected = true;
}

void loop() {
  /*if (Serial.available()) {
    Serial.write(Serial.read());
  }*/
  recvWithStartEndMarkers();
  if(newData && WiFi.status() == WL_CONNECTED){
    showNewData();
    httpsPost(url, String(receivedChars));
  }
  
  
//  
//  data[0] = (char)0; //Clean the data array
//  Serial.readBytes(data,100);
//  Serial.print("data:");
//  Serial.println(data);
//  if(data[0]==0 || data==""){
//    if(wifiConnected){
//      String msg=httpsPost(url,data); 
//      if(msg=="Success")
//        Serial.println("y");
//      else
//        Serial.print("Error: ");
//        Serial.println(msg);
//    }
//    else
//      Serial.println("Error: Wifi connection is out!");        
//  }
}

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
 
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void showNewData() {
  Serial.print("Received data from arduino: ");
  Serial.println(receivedChars);
  newData = false;
}

boolean wifiConnection() {
  WiFi.begin(ssid, password);
  int count = 0;
  Serial.print("Waiting for Wi-Fi connection");
  while ( count < 20 ) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      Serial.println("Connected!");
      return (true);
    }
    delay(500);
    Serial.print(".");
    count++;
  }
  Serial.println("Timed out.");
  return false;
}

void httpsPost(String url, String data) {
  char msg[30];
  http.begin(url);
  http.addHeader("Content-type","application/json");
  int httpCode = http.POST(data);
  String payload = http.getString();
  if(httpCode<0)//got an error
    //Serial.println(http.errorToString(httpCode).c_str());
    Serial.println(httpCode);
    sprintf(msg, "<Error:%d>", httpCode);
    if(Serial.available())
        Serial.write(msg); //Send msg to arduino   
  else{
    Serial.println(httpCode);
    Serial.println(payload);
    if(Serial.available())
        Serial.write("<Uploaded>"); //Send msg to arduino
  }
  http.end();

  

  
  
//  char msg[20];
//  if (client.connect(host, httpsPort)) {
//    Serial.print("Server connected, data:");
//    Serial.println(data);
//    client.println("POST " + url + " HTTP/1.1");
//    client.println("Host: " + (String)host);
//    client.println("User-Agent: ESP8266/1.0");
//    client.println("Connection: close");
//    client.println("Content-Type: text/plain;");
//    client.print("Content-Length: ");
//    client.println(data.length());
//    client.println();
//    client.println(data);
//    delay(10);
//    String response = client.readString();
//    int bodypos =  response.indexOf("\r\n\r\n") + 4;
//
//    Serial.write("<Uploaded>"); //Send msg to arduino
//    return response.substring(bodypos);
// 
//  }
//  else {
//    //return error number
//    sprintf(msg, "<Error:%s>", client.connect(host,httpsPort));
//    Serial.write(msg); //Send msg to arduino
//    return String(client.connect(host,httpsPort));
//  }
}

// This method makes a HTTP connection to the server and POSTs data
//void postData(float data, String DBcolumn) {
//  // Combine yourdatacolumn header (yourdata=) with the data recorded from your arduino
//  // (yourarduinodata) and package them into the String yourdata which is what will be
//  // sent in your POST request
//   datapost = DBcolumn + data;
//
//  // If there's a successful connection, send the HTTP POST request
//  if (client.connect(server, 80)) {
//    Serial.println("connecting...");
//
//    // EDIT: The POST 'URL' to the location of your insert_mysql.php on your web-host
//    client.println("POST /insert_mysql.php HTTP/1.1");
//
//    // EDIT: 'Host' to match your domain
//    client.println("Host: www.yourdomain.com");
//    client.println("User-Agent: Arduino/1.0");
//    client.println("Connection: close");
//    client.println("Content-Type: application/x-www-form-urlencoded;");
//    client.print("Content-Length: ");
//    client.println(datapost.length());
//    client.println();
//    client.println(datapost); 
//  } 
//  else {
//    // If you couldn't make a connection:
//    Serial.println("Connection failed");
//    Serial.println("Disconnecting.");
//    client.stop();
//  }
//}
