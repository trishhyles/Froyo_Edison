#include<WiFi.h>
#include <Ethernet.h>
#include<SPI.h>
#include<LiquidCrystal.h>
#include<math.h>
#include<TimerOne.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int sec = 0;
int DEF_MIN = 0;

#define APIKEY         "your_api_key" // replace your pachube api key here
#define SENSORID       "your_sensor_id"   //light sensor id
#define DEVICEID       "your_device_id"    //edison device id

#define HEATSENSERID       "your_switch_sensor_id"   //switch sensor id
bool ResponseBegin = false;

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

#define BUZZER_PIN 0
#define TEMP_PIN A1
#define SELECT_PIN 2
int temp = 0;
int target_temp = 50;
int start_but = 0;
int adc_key_in = 0;
int PWM_PIN = 3;
int status = WL_IDLE_STATUS;
char ssid[] = "xmh"; //  your network SSID (name)
char pass[] = "123456789";    // your network password (use for WPA, or use as key for WEP)
char server[] = "api.yeelink.net";

int data_count = 0;
float testdata=0.0;

WiFiClient client;

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void sendData(int thisData) {
  // if there's a successful connection:
  if (client.connect(server, 80))
  {
    client.setTimeout(100);
    Serial.print("\r\n==========connected to server,I'll send:");
    Serial.println(thisData);

    String cmd;
        cmd = "POST /v1.1/device/";
        cmd += String(DEVICEID);
        cmd += "/sensor/";
        cmd += String(SENSORID);
        cmd += "/datapoints";
        cmd += " HTTP/1.1\r\n";
        cmd += "Host: api.yeelink.net\r\n";
        cmd += "Accept: *";
        cmd += "/";
        cmd += "*\r\n";
        cmd += "U-ApiKey: ";
        cmd += APIKEY;
        cmd += "\r\n";
        cmd += "Content-Length: ";
        int thisLength = 10 + getLength(thisData);
    cmd += String(thisLength);
        cmd += "\r\n";
        cmd += "Content-Type: application/x-www-form-urlencoded\r\n";
        cmd += "Connection: close\r\n";
        cmd += "\r\n";
        cmd += "{\"value\":";
        cmd += String(thisData);
        cmd += "}\r\n";

        //Serial.println(cmd);

    client.print(cmd);

    //print server back message
    while (client.available())
    {
      char c = client.read();
      Serial.write(c);
    }

    client.stop();
    Serial.print("\r\n==========connection closed");
    Serial.println();
    Serial.println();
  } 
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed,disconnecting.");
    client.stop();
  delay(200);
  }
}

int getLength(int someValue) {
  // there's at least one byte:
  int digits = 1;
  // continually divide the value by ten, 
  // adding one to the digit count for each
  // time you divide, until you're at 0:
  int dividend = someValue /10;
  while (dividend > 0) {
    dividend = dividend /10;
    digits++;
  }
  // return the number of digits:
  return digits;
}

int read_LCD_buttons()
{               // read the buttons
    adc_key_in = analogRead(0);       // read the value from the sensor 
 
    // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
    // we add approx 50 to those values and check to see if we are close
    // We make this the 1st option for speed reasons since it will be the most likely result
 
    if (adc_key_in > 1000) return btnNONE; 
 
    // For V1.1 us this threshold
    if (adc_key_in < 50)   return btnRIGHT;  
    if (adc_key_in < 250)  return btnUP; 
    if (adc_key_in < 450)  return btnDOWN; 
    if (adc_key_in < 650)  return btnLEFT; 
    if (adc_key_in < 850)  return btnSELECT;  
 
   // For V1.0 comment the other threshold and use the one below:
   /*
     if (adc_key_in < 50)   return btnRIGHT;  
     if (adc_key_in < 195)  return btnUP; 
     if (adc_key_in < 380)  return btnDOWN; 
     if (adc_key_in < 555)  return btnLEFT; 
     if (adc_key_in < 790)  return btnSELECT;   
   */
 
    return btnNONE;                // when all others fail, return this.
}


bool getOnOff(void)
{
  // if there’s a successful connection:
  if (client.connect(server, 80))
  {
    String returnValue = "";
    // send the HTTP GET request:
    //client.setTimeout(100);
    client.print("GET /v1.1/device/");
    client.print(DEVICEID);
    client.print("/sensor/");
    client.print(HEATSENSERID);
    client.print("/datapoints");
    client.println(" HTTP/1.1");
    client.println("Host: api.yeelink.net");
    client.print("Accept: *");
    client.print("/");
    client.println("*");
    client.print("U-ApiKey: ");
    client.println(APIKEY);
    client.println("Content-Length: 0");
    client.println("Connection: close");
    client.println();
    while (client.available())
    {
      char c = client.read();;
      if (c == '{')
        ResponseBegin = true;
      else if (c == '}')
      {
        ResponseBegin = false;
        break;
      }

      if (ResponseBegin)
      {
        returnValue += c;
      }
      
    }
    
    client.stop();
    //Serial.println("returnValue");
    //Serial.println(returnValue);
    if(  returnValue.charAt(9) == '1' )
    {
      Serial.println("ret true");
      return true;
    }
    else
    {
      Serial.println("ret false");
      return false;
    }
  }
  else
  {
    // if you couldn’t make a connection:
    Serial.println("get on/off failed.");
    client.stop();
    return false;
  }
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  for(int i=4; i<10; i++) {
    pinMode(i, OUTPUT);
  }
  lcd.begin(16,2);
  lcd.begin(16,2);
  lcd.setCursor(5,0);
  lcd.print("Welcome");
  lcd.setCursor(1,1);
  lcd.print("Initiating...");
  
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.print(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    Serial.print("\tstatus = ");
    Serial.println(status);
    delay(500);
  }
  Serial.println("Connected to wifi\r\n========");
  printWifiStatus();
  lcd_init();

  pinMode(PWM_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(TEMP_PIN, INPUT);
  pinMode(SELECT_PIN, OUTPUT);
  digitalWrite(SELECT_PIN, LOW);
  Timer1.initialize(1000000);
  Timer1.attachInterrupt( timerIsr );
  client.setTimeout(200);
}

void loop() {
  digitalWrite(SELECT_PIN, LOW);
  lcd.setCursor(2,0);
  lcd.print("Froyo Edison");
  lcd.setCursor(6,1);
  lcd.print("Start");
  analogWrite(PWM_PIN, 0);

  
  while (  read_LCD_buttons() != btnSELECT   )
  {
    Serial.print("Start button = ");
    Serial.println(start_but);
    if(    getOnOff()  )
    {
      break;
    }
    //continue ;// start button did not pressed, back loop
    delay(100);
  }
  int i = 0;
  
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print("Heat Now");
  DEF_MIN = 0;
  sec = 0;
  while (sec < 30) { 
    int cancelled = 0; 
    cancelled = read_LCD_buttons();
    //Serial.println(cancelled);
    if (cancelled == btnSELECT) {
      lcd.clear();
      analogWrite(PWM_PIN, 0);
      return ;
    }

    double digital_value = analogRead(TEMP_PIN);
    double Rx = 1.0*(1024*10*1000.0/digital_value) - 10*1000.0;Rx /= 1000.0;temp = -23.99 * log(Rx) + 81.22;
    Serial.print("Temp is:");Serial.println(temp);
    /*
    int delta = target_temp - temp;
    int pwm_out = delta * 15;
    if (pwm_out > 256)
      pwm_out = 255;
    if (pwm_out < 0)
      pwm_out = 0;
*/
    int pwm_out = 250;
    if  (temp >= 40)
      pwm_out = 66;
    lcd.setCursor(0,1);   lcd.print("Temp:");lcd.print((int)temp);lcd.print(" PWM:");lcd.print(pwm_out); lcd.print(" ");
    analogWrite(PWM_PIN, pwm_out); Serial.println(pwm_out);
    data_count ++;
    if (data_count >= 20) {
      sendData(temp);
      data_count = 0;
    }
  }
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Heat Done");
  sec = 0;
  while (sec <= 4) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(900);
  } 

  //=============================================
  //lcd.clear();
  //lcd.setCursor(4, 0);
  //lcd.print("Cool Now");
  //digitalWrite(SELECT_PIN, HIGH);
  analogWrite(PWM_PIN, 247);
  sec = 0;
  DEF_MIN = 0;
  /*
  lcd.setCursor(0,1);
  lcd.clear();
  lcd.print("Temp:");
  lcd.print(temp);
  Serial.print("Temp is:");
  Serial.println(temp);
  lcd.print(" Time:");
  lcd.print(DEF_MIN);
  */
  lcd.clear();
  lcd.setCursor(5,0);
  lcd.print("Cooling");
  digitalWrite(SELECT_PIN, HIGH);
  
  while (1) {
    int restart_but = read_LCD_buttons();
    //Serial.println(restart_but);
    if (restart_but == btnSELECT) {
      analogWrite(PWM_PIN, 0);
      break ;
    }

    double digital_value = analogRead(TEMP_PIN);
    double Rx = 1.0*(1024*10*1000.0/digital_value) - 10*1000.0;Rx /= 1000.0;temp = -23.99 * log(Rx) + 81.22;

    data_count ++;
    if (data_count >= 20) {
      sendData(temp);
      data_count = 0;
      delay(10000);
    }
/*
    lcd.setCursor(0,1);
    lcd.clear();
    lcd.print("Temp:");
    lcd.print(temp);
    Serial.print("Temp is:");
    Serial.println(temp);
    lcd.print(" Time:");
    lcd.print(DEF_MIN);*/
  }
  lcd.clear();
}

void lcd_init() {
  //lcd.setCursor(5,0);
  //lcd.print("Welcome");
  lcd.clear();
  lcd.setCursor(2,1);
  lcd.print("Froyo Edison");
  delay(2000);
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Froyo Edison");
  lcd.setCursor(6,1);
  lcd.print("Start");
}

void timerIsr() {
  sec ++;
  if (sec >= 60) {
    sec = 0;
    DEF_MIN ++;
  }
}

