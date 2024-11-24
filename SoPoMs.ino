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
#define countof(a) (sizeof(a) / sizeof(a[0]))

DHT11 dht11(2);
LiquidCrystal_I2C lcd(0x27, 16, 2);
ThreeWire mywire(dat, clk, rst);
RtcDS1302<ThreeWire> rtc(mywire);

File myfile;

bool set_flag = 0;
bool up_flag = 0;
bool down_flag = 0;

int mode = 0;

int temperature = 0;
int humidity = 0;

int tempVal = 0;
int humVal = 0;

void printDateTime(const RtcDateTime& dt)
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
    Serial.print(datestring);
}

void setup() {
  // put your setup code here, to run once:
    int result = dht11.readTemperatureHumidity(temperature, humidity);
    tempVal = temperature;
    humVal = humidity;

    showTempHumid();
    mode = 0;

    Serial.begin(9600);
    lcd.init();
    lcd.backlight();

    pinMode(btn_set, INPUT);
    pinMode(btn_up, INPUT);
    pinMode(btn_down, INPUT);

    pinMode(cs, OUTPUT);
}

void showTempHumid()
{
    int result = dht11.readTemperatureHumidity(temperature, humidity);
    String s_temperature = "Temp: ";
    String s_humidity = "Humidity: ";

    if(result == 0)
    {
        s_temperature += (String)temperature;
        s_temperature += (char)223; //degrees symbol
        s_temperature += "C";
        s_humidity += (String)humidity;
        s_humidity += "%";

        Serial.print(s_temperature);
        Serial.print(", ");
        Serial.println(s_humidity);

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
}

void setTemp()
{
    String s_temp = "Trigger Temp (";
    s_temp += (char)223;
    s_temp += ")";
    if(digitalRead(btn_up)) up_flag = 1;
    if(digitalRead(btn_down)) down_flag = 1;

    if(!digitalRead(btn_up) && up_flag)
    {
        up_flag = 0;
        tempVal++;
    }

    if(!digitalRead(btn_down) && down_flag)
    {
        down_flag = 0;
        tempVal--;
    }

    lcd.setCursor(0,0);
    lcd.print(s_temp);

    lcd.setCursor(0,1);
    lcd.print(tempVal);
    Serial.print("Temp val: ");
    Serial.println(tempVal);
}

void setHumid()
{
    if(digitalRead(btn_up)) up_flag = 1;
    if(digitalRead(btn_down)) down_flag = 1;

    if(!digitalRead(btn_up) && up_flag)
    {
        up_flag = 0;
        humVal++;
    }

    if(!digitalRead(btn_down) && down_flag)
    {
        down_flag = 0;
        humVal--;
    }

    lcd.setCursor(0,0);
    lcd.print("Trigger Humidity (%)");

    lcd.setCursor(0,1);
    lcd.print(humVal);
    Serial.print("hum val: ");
    Serial.println(humVal);
}

void clear()
{
    lcd.setCursor(0,0);
    for(int i = 0; i < 16; i++) lcd.write(' ');
    lcd.setCursor(0,1);
    for(int i = 0; i < 16; i++) lcd.write(' ');
}

void GetDateTime()
{
    RtcDateTime now = rtc.GetDateTime();

    printDateTime(now);
    Serial.print(" | ");

    if (!now.IsValid())
    {
        // Common Causes:
        //    1) the battery on the device is low or even missing and the power line was disconnected
        Serial.println("RTC lost confidence in the DateTime!");
    }
}

void initSD()
{
    // if(SD.begin())
    // {
    //     Serial.println("SD Card ready");
    // }
    // else
    // {
    //     Serial.println("SD Card init failed");
    //     return;
    // }

    // myfile = SD.open("test.txt", FILE_WRITE);
    // if(myfile)
    // {
    //     Serial.println("Writing ti file...");
    //     myfile.println("Testing text 123");
    //     myfile.close();
    //     Serial.println("Finished");
    // }
    // else
    // {
    //     Serial.println("Error opening test.txt");
    // }

    // myfile = SD.open("test.txt");
    // if(myfile)
    // {
    //     Serial.println("Read: ");
    //     while(myfile.available())
    //     {
    //         Serial.write(myfile.read());
    //     }
    //     myfile.close();
    // }
    // else
    // {
    //     Serial.println("Error opening test.txt");
    // }
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
        // Common Causes:
        //    1) first time you ran and the device wasn't running yet
        //    2) the battery on the device is low or even missing

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
    GetDateTime();
    if(digitalRead(btn_set) && !set_flag) set_flag = 1;

    if(!digitalRead(btn_set) && set_flag)
    {
        clear();
        set_flag = 0;
        mode++;
        if(mode > 2) mode = 0;
    }

    switch(mode)
    {
        case 0: showTempHumid(); break; 
        case 1: setTemp(); break;
        case 2: setHumid(); break;
    }

    analogWrite(relay, (temperature >= tempVal || humidity >= humVal) ? 255 : 0);
    Serial.println((temperature >= tempVal || humidity >= humVal) ? "relay on" : "relay off");
}
