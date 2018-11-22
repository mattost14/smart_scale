/*
 
 * 2016 November 25
 * Load Cell HX711 Module Interface with Arduino to measure weight in Kgs
 Arduino 
 pin 
 Most any pin on the Arduino Uno will be compatible with DOUT/CLK.
 The HX711 board can be powered from 2.7V to 5V so the Arduino 5V power should be fine.
*/
 
#include "HX711.h"  //You must have this library in your arduino library folder
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <elapsedMillis.h>
 
#define DOUT  3
#define CLK  2
 
HX711 scale(DOUT, CLK);

// Inicializa o display no endereco 0x3F
LiquidCrystal_I2C lcd(0x3F,2,1,0,4,5,6,7,3, POSITIVE);
 
//Change this calibration factor as per your load cell once it is found you many need to vary it in thousands
float calibration_factor = 18600; //-106600 worked for my 40Kg max scale setup arduindawdacs carararararararaararsfv ewwe
float weight[100] = {}; 
int count;
float interval=3000;
bool newData = false;
const byte numChars = 20;
char receivedChars[numChars];
elapsedMillis timeElapsed, wait_time;
//=============================================================================================
//                         SETUP
//=============================================================================================
void setup() {
  Serial.begin(115200);


  scale.set_scale(18600);


  lcd.begin(20,4);
  lcd.setCursor(2,0);
  lcd.print("__SMART SCALE__");
  lcd.setCursor(2,2);
  lcd.print("Inicializando...");
  Serial.println("Press t for tare");
  delay(1500);
  scale.tare(); //Reset the scale to 0

  //lcd.backlight();
  //lcd.begin(20,4);
  //lcd.setCursor(2,2);
  //lcd.print("Suba na Balanca");


  count=0;
  timeElapsed = 0;
}
 
//=============================================================================================
//                         LOOP
//=============================================================================================
void loop() {
  bool cancel;
//  if(Serial.available())
//    Serial.println("Wifi module: ok!");
//  else
//    Serial.println("Wifi module: failed!");

  //Check weight measurement and stability after 100 cycles or during the interval time (3 sec) 
  if(count==100 || timeElapsed>interval){   
    int size_array = sizeof(weight) / sizeof(weight[0]);

    //Get in if the weight is stable on the balance
    int checkWeight = checkStability(weight,count);
    if(checkWeight==1){
      //Serial.println("Peso Estabelecido!");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Peso Fixado:");
      lcd.setCursor(13,0);
      lcd.print(weight[1], 2);
      lcd.setCursor(17,0);
      lcd.print("kg");
      cancel=false;
      lcd.setCursor(3,2);
      lcd.print("Enviando em:");
      for (int i=3; i>=0; i--){
        lcd.setCursor(16,2);
        lcd.print(i);
        delay(1500);
        if(scale.get_units()<.5){
          cancel = true; 
          break;
        }
      }
      //Cancel is true if the weight is withdrawn during the count down  
      if(cancel){
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Vc saiu da balanca!");
          lcd.setCursor(2,2);
          lcd.print("Envio Cancelado!");
          delay(1500);
          lcd.clear();
          lcd.setCursor(2,2);
          lcd.print("Recalibrando...");
          delay(1500);
          scale.tare();
      }
      //If weight is on during the count down, then send it to web server
      else{ 
          String msg;
          int timeout = 5000;
          wait_time=0;
          lcd.setCursor(5,2);
          lcd.print("Enviando...");
          msg = uploadWebServer(weight[1]);
          Serial.println();
          Serial.println(msg);    
          if(msg=="Uploaded"){
            lcd.clear();
            lcd.setCursor(2,2);
            lcd.print("Peso enviado!");
            delay(3000); 
          }
          else{
            lcd.clear();
            lcd.setCursor(8,0);
            lcd.print("Ops!");
            lcd.setCursor(2,2);
            lcd.print("Erro no Envio!"); 
            delay(3000); 
          }
      }
    }
    //If the weight is bellow the minimun threshold (0.5kg)
    if(checkWeight==0){
      lcd.clear();
      lcd.setCursor(2,2);
      lcd.print("Suba na Balanca");
      delay(2000);
      lcd.noBacklight(); //Turn the LCD off
    }
    if(checkWeight==2){ //Weight not stable enough
      lcd.clear();
      lcd.setCursor(2,2);
      lcd.print("Estabilize");    
    }
    count=0;
    timeElapsed = 0;    
  }
  //If the timelapsed is less than 3sec or the cycle number is less than 100
  else{
    weight[count] = scale.get_units();
    //If it detects some relevant weight on
    if(weight[count]>0.1){
      lcd.backlight(); //Turn the LCD lights
      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("Pesando...");
      lcd.setCursor(6,2);
      lcd.print(weight[count]);
      lcd.print(" kg");
      delay(300);
    }
    count++;
  }
//  Serial.print("count:");
//  Serial.print(count);
//  Serial.print("time elapsed: ");
//  Serial.print(timeElapsed); 
//  Serial.print(" - Weigth: ");
//  Serial.print(weight[count], 2);
//  Serial.println(" kg"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person


 
//  if(Serial.available())
//  {
//    char temp = Serial.read();
//    if(temp == 't' || temp == 'T')
//      scale.tare();
// 
//  }
  
}

String uploadWebServer(float value){
  char data[50], str_value[6];
  dtostrf(value, 4, 2, str_value);
  sprintf(data, "<{\"source_public_id\":\"%s\",\"measure\":\"%s\"}>", "1234",str_value);
  Serial.println();
  //Serial.println("Send the json string to wifi module...");
  if(Serial.available()){
    Serial.write(data,50); //send data to wifi module
    wait_time=0;
    while(!newData && wait_time < 10000){
      recvWithStartEndMarkers(); //check for wifi module answer
      Serial.print(".");
    }
    Serial.println();
    if(newData)
       return String(receivedChars);
    else
      return "Error: TimeOut!";
  }
  else{
    return "Error: Wifi module not connected!";
  }
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

int checkStability(float value[], int count){
  //find minimum value
    int index=1;
    float max_weight, min_weight;
    for (int i = 1; i < count; i++) 
    {
       //Serial.println(value[i]);
      if (value[i] <= value[index]) {
        index = i;
      }
    }

    min_weight = value[index];

    if(min_weight > .5){
      //find maximum value
      index=0;
      for (int i = 1; i < count; i++) 
      {
        if (value[i] >= value[index]) {
          index = i;
        }
      }
      max_weight = value[index];
    }
    else{ 
      return 0; //Not enough weight
    }
    if((max_weight-min_weight)< 0.2){
      return 1; //Enough weight and stability
    }
    else{
      return 2; //Not stable enough
    }  
}
//=============================================================================================
