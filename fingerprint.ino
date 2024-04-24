/***************************************************
  This is an example sketch for our optical Fingerprint sensor

  Designed specifically to work with the Adafruit BMP085 Breakout
  ----> http://www.adafruit.com/products/751

  These displays use TTL Serial to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  Small bug-fix by Michael cochez

  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(11, 10, 9, 8, 7, 6);
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// Set up the serial port to use softwareserial..
SoftwareSerial mySerial(2, 3);

#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is green wire, #1 is white
#define mySerial Serial1

#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id;

void setup()
{
    Serial.begin(9600);
    while (!Serial)
        ; // For Yun/Leo/Micro/Zero/...
    delay(100);
    lcd.print("\n\nAdafruit Fingerprint sensor enrollment");

    // set the data rate for the sensor serial port
    finger.begin(57600);

    if (finger.verifyPassword())
    {
        lcd.print("Found fingerprint sensor!");
    }
    else
    {
        lcd.print("Did not find fingerprint sensor :(");
        while (1)
        {
            delay(1);
        }
    }

    lcd.print(F("Reading sensor parameters"));
    finger.getParameters();
    Serial.print(F("Status: 0x"));
    lcd.print(finger.status_reg, HEX);
    Serial.print(F("Sys ID: 0x"));
    lcd.print(finger.system_id, HEX);
    Serial.print(F("Capacity: "));
    lcd.print(finger.capacity);
    Serial.print(F("Security level: "));
    lcd.print(finger.security_level);
    Serial.print(F("Device address: "));
    lcd.print(finger.device_addr, HEX);
    Serial.print(F("Packet len: "));
    lcd.print(finger.packet_len);
    Serial.print(F("Baud rate: "));
    lcd.print(finger.baud_rate);
}

uint8_t readnumber(void)
{
    uint8_t num = 0;

    while (num == 0)
    {
        while (!Serial.available())
            ;
        num = Serial.parseInt();
    }
    return num;
}

void loop() // run over and over again
{
    lcd.print("Ready to enroll a fingerprint!");
    lcd.print("Please type in the ID # (from 1 to 127) you want to save this finger as...");
    id = readnumber();
    if (id == 0)
    { // ID #0 not allowed, try again!
        return;
    }
    Serial.print("Enrolling ID #");
    lcd.print(id);

    while (!getFingerprintEnroll())
        ;
}

uint8_t getFingerprintEnroll()
{

    int p = -1;
    Serial.print("Waiting for valid finger to enroll as #");
    lcd.print(id);
    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            lcd.print("Image taken");
            break;
        case FINGERPRINT_NOFINGER:
            Serial.print(".");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            lcd.print("Communication error");
            break;
        case FINGERPRINT_IMAGEFAIL:
            lcd.print("Imaging error");
            break;
        default:
            lcd.print("Unknown error");
            break;
        }
    }

    // OK success!

    p = finger.image2Tz(1);
    switch (p)
    {
    case FINGERPRINT_OK:
        lcd.print("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        lcd.print("Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        lcd.print("Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        lcd.print("Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        lcd.print("Could not find fingerprint features");
        return p;
    default:
        lcd.print("Unknown error");
        return p;
    }

    lcd.print("Remove finger");
    delay(2000);
    p = 0;
    while (p != FINGERPRINT_NOFINGER)
    {
        p = finger.getImage();
    }
    Serial.print("ID ");
    lcd.print(id);
    p = -1;
    lcd.print("Place same finger again");
    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            lcd.print("Image taken");
            break;
        case FINGERPRINT_NOFINGER:
            Serial.print(".");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            lcd.print("Communication error");
            break;
        case FINGERPRINT_IMAGEFAIL:
            lcd.print("Imaging error");
            break;
        default:
            lcd.print("Unknown error");
            break;
        }
    }

    // OK success!

    p = finger.image2Tz(2);
    switch (p)
    {
    case FINGERPRINT_OK:
        lcd.print("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        lcd.print("Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        lcd.print("Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        lcd.print("Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        lcd.print("Could not find fingerprint features");
        return p;
    default:
        lcd.print("Unknown error");
        return p;
    }

    // OK converted!
    Serial.print("Creating model for #");
    lcd.print(id);

    p = finger.createModel();
    if (p == FINGERPRINT_OK)
    {
        lcd.print("Prints matched!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        lcd.print("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_ENROLLMISMATCH)
    {
        lcd.print("Fingerprints did not match");
        return p;
    }
    else
    {
        lcd.print("Unknown error");
        return p;
    }

    Serial.print("ID ");
    lcd.print(id);
    p = finger.storeModel(id);
    if (p == FINGERPRINT_OK)
    {
        lcd.print("Stored!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        lcd.print("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_BADLOCATION)
    {
        lcd.print("Could not store in that location");
        return p;
    }
    else if (p == FINGERPRINT_FLASHERR)
    {
        lcd.print("Error writing to flash");
        return p;
    }
    else
    {
        lcd.print("Unknown error");
        return p;
    }

    return true;
}
