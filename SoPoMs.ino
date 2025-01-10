//Arduino UNO

//SD Card Module
//CS	10
//SCK	13
//MOSI	11
//MISO	12

//DS1302 RTC
//CLK	7
//DAT	8
//RST	9

//DHT11 Temp and Humidity Sensor
//DATA	2

//LCD I2C 16x2
//SCL	A5
//SDA	A4

//5v Relay Module (pump)
//S		A3

//5v Relay Module (humidifier)
//S1    A0
//S2    A1

#include <DHT11.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <RtcDS1302.h>
#include <ThreeWire.h>

#define btn_set 3
#define btn_up 4
#define btn_down 5
#define btn_save 6
#define clk 7
#define dat 8
#define rst 9
#define cs 10
#define relay A3
#define mist0 A0
#define mist1 A1
#define countof(a) (sizeof(a) / sizeof(a[0]))

DHT11 dht11(2);
LiquidCrystal_I2C lcd(0x27, 16, 2);
ThreeWire mywire(dat, clk, rst);
RtcDS1302<ThreeWire> rtc(mywire);

File myFile;

bool set_flag = 0;
bool up_flag = 0;
bool down_flag = 0;

int mode = 0;

int temperature = 0;
int humidity = 0;

int tempVal1 = 0;
int humVal1 = 0;

int tempVal2 = 0;
int humVal2 = 0;

String printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    //Serial.print(datestring);

    return datestring;
}

void setup() {
  // put your setup code here, to run once:
    pinMode(cs, OUTPUT);
    pinMode(relay, OUTPUT);
    pinMode(mist0, OUTPUT);
    pinMode(mist1, OUTPUT);
    int result = dht11.readTemperatureHumidity(temperature, humidity);
    tempVal1 = temperature;
    humVal1 = humidity;
    tempVal2 = temperature;
    humVal2 = humidity;

    showTempHumid();
    mode = 0;

    Serial.begin(9600);
    lcd.init();
    lcd.backlight();

    pinMode(btn_set, INPUT);
    pinMode(btn_up, INPUT);
    pinMode(btn_down, INPUT);

    initSD();

}

String showTempHumid()
{
    int result = dht11.readTemperatureHumidity(temperature, humidity);
    String s_temperature = "Temp: ";
    String s_humidity = "Humidity: ";
    String finalString = "";

    if(result == 0)
    {
        s_temperature += (String)temperature;
        s_temperature += (char)223; //degrees symbol
        s_temperature += "C";
        s_humidity += (String)humidity;
        s_humidity += "%";

        lcd.setCursor(0,0);
        lcd.print(s_temperature);
        delay(100);
        lcd.setCursor(0,1);
        lcd.print(s_humidity);
    }
     else
    {
        Serial.println(DHT11::getErrorString(result));
        lcd.setCursor(0,0);
        lcd.print(DHT11::getErrorString(result));
    }

    finalString = s_temperature;
    finalString += ",";
    finalString += s_humidity;

    return finalString;
}

void setTemp(int mode)
{
    if(mode == 1)
    {
        String s_temp = "Trig Temp 1(";
        s_temp += (char)223;
        s_temp += ")";
        if(digitalRead(btn_up)) up_flag = 1;
        if(digitalRead(btn_down)) down_flag = 1;

        if(!digitalRead(btn_up) && up_flag)
        {
            up_flag = 0;
            tempVal1++;
        }

        if(!digitalRead(btn_down) && down_flag)
        {
            down_flag = 0;
            tempVal1--;
        }

        lcd.setCursor(0,0);
        lcd.print(s_temp);

        lcd.setCursor(0,1);
        lcd.print(tempVal1);

        Serial.print("Temp 1: ");
        Serial.println(tempVal1);
    }
    else if (mode == 2)
    {
        String s_temp = "Trig Temp 2(";
        s_temp += (char)223;
        s_temp += ")";
        if(digitalRead(btn_up)) up_flag = 1;
        if(digitalRead(btn_down)) down_flag = 1;

        if(!digitalRead(btn_up) && up_flag)
        {
            up_flag = 0;
            tempVal2++;
        }

        if(!digitalRead(btn_down) && down_flag)
        {
            down_flag = 0;
            tempVal2--;
        }
        lcd.setCursor(0, 0);
        lcd.print(s_temp);

        lcd.setCursor(0,1);
        lcd.print(tempVal2);
        Serial.print("Temp 2: ");
        Serial.println(tempVal2);
    }
}

void setHumid(int mode)
{
    if (mode == 1)
    {
        if(digitalRead(btn_up)) up_flag = 1;
        if(digitalRead(btn_down)) down_flag = 1;

        if(!digitalRead(btn_up) && up_flag)
        {
            up_flag = 0;
            humVal1++;
        }

        if(!digitalRead(btn_down) && down_flag)
        {
            down_flag = 0;
            humVal1--;
        }
        lcd.setCursor(0, 0);
        lcd.print("Trig Hmdty 1(%) ");

        lcd.setCursor(0,1);
        lcd.print(humVal1);
        Serial.print("hum val 1: ");
        Serial.println(humVal1);

    }
    else if (mode == 2)
    {
        if(digitalRead(btn_up)) up_flag = 1;
        if(digitalRead(btn_down)) down_flag = 1;

        if(!digitalRead(btn_up) && up_flag)
        {
            up_flag = 0;
            humVal2++;
        }

        if(!digitalRead(btn_down) && down_flag)
        {
            down_flag = 0;
            humVal2--;
        }
        lcd.setCursor(0, 0);
        lcd.print("Trig Hmdty 2(%) ");

        lcd.setCursor(0,1);
        lcd.print(humVal2);
        Serial.print("hum val 2: ");
        Serial.println(humVal2);

    }
}

String GetDateTime()
{
    RtcDateTime now = rtc.GetDateTime();

    Serial.print(printDateTime(now));
    Serial.print(" | ");

    if (!now.IsValid())
    {
        Serial.println("RTC lost confidence in the DateTime!");
    }

    return printDateTime(now);
}

void initSD()
{
    if(SD.begin())
    {
        Serial.println("SD Card ready");
    }
    else
    {
        Serial.println("SD Card init failed");
        return;
    }
}

void sdLog()
{
    myFile = SD.open("test.txt", FILE_WRITE);
    if(myFile)
    {
        myFile.print(GetDateTime());
        myFile.print(" | ");
        myFile.print(showTempHumid());
        myFile.print(" | ");
        myFile.print(temperature >= tempVal1 || humidity >= humVal1 ? "pump on | " : "pump off | ");
        myFile.print(temperature >= tempVal1 || humidity >= humVal1 ? "mist_1 on | " : "mist_1 off | ");
        myFile.print(temperature >= tempVal1 || humidity >= humVal1 ? "mist_2 on | " : "mist_2 off | ");
        myFile.print("trig temp 1: ");
        myFile.print(tempVal1);
        myFile.print(" | trig temp 2: ");
        myFile.print(tempVal2);
        myFile.print(" | trig hmdty 1: ");
        myFile.print(humVal1);
        myFile.print(" | trig hmdty 1: ");
        myFile.println(humVal1);
        myFile.close();
    }
    else
    {
        Serial.println("error opening file");
    }
}

void initRTC()
{
    Serial.print("Compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);

    rtc.Begin();
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    Serial.println();

    if (!rtc.IsDateTimeValid()) 
    {
        Serial.println("RTC lost confidence in the DateTime!");
        rtc.SetDateTime(compiled);
    }

    if (rtc.GetIsWriteProtected())
    {
        Serial.println("RTC was write protected, enabling writing now");
        rtc.SetIsWriteProtected(false);
    }

    if (!rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        rtc.SetIsRunning(true);
    }

    RtcDateTime now = rtc.GetDateTime();
    if (now < compiled) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled) 
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }
}

void loop() {
  // put your main code here, to run repeatedly:
    if(digitalRead(btn_set) && !set_flag) set_flag = 1;

    if(!digitalRead(btn_set) && set_flag)
    {
        lcd.clear();
        set_flag = 0;
        mode++;
        if(mode > 4) mode = 0;
    }

    switch(mode)
    {
        case 0: sdLog(); Serial.print(showTempHumid()); Serial.print(" | "); break; 
        case 1: setTemp(1); break;
        case 2: setTemp(2);  break;
        case 3: setHumid(1); break;
        case 4: setHumid(2); break;
    }

    digitalWrite(relay, (temperature >= tempVal1 || humidity >= humVal1));
    Serial.print((temperature >= tempVal1 || humidity >= humVal1) ? "relay on | " : "relay off | ");

    digitalWrite(mist0, !(temperature >= tempVal1 || humidity >= humVal1));
    Serial.print((temperature >= tempVal1 || humidity >= humVal1) ? "mist0 on | " : "mist0 off | ");

    digitalWrite(mist1, !(temperature >= tempVal2 || humidity >= humVal2));
    Serial.print((temperature >= tempVal2 || humidity >= humVal2) ? "mist1 on | " : "mist1 off | ");

    Serial.print(humidity);
    Serial.print(" | ");
    Serial.print(tempVal1);
    Serial.print(" | ");
    Serial.print(tempVal2);
    Serial.print(" | ");
    Serial.print(humVal1);
    Serial.print(" | ");
    Serial.println(humVal2);

    delay(250);
}

bool isOn = false;
void onMist(int pin)
{
    if(!isOn)
    {
        isOn = true;
        digitalWrite(pin, HIGH);
        delay(250);
        digitalWrite(pin, LOW);
    }
}

void offMist(int pin)
{
    if(isOn)
    {
        isOn = false;
        digitalWrite(pin, HIGH);
        delay(250);
        digitalWrite(pin, LOW);

        digitalWrite(pin, HIGH);
        delay(250);
        digitalWrite(pin, LOW);
    }
}
