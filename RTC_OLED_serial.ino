#include "RTClib.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define messages 1
#define debug 0

// Real TIME CLOCK ============================================================
RTC_DS3231 rtc;

// OLED SETUP ==================================================================
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// SERIAL INPUT ================================================================
#define serial_buffer_size 18
char input_buffer[serial_buffer_size];
int serial_index = 0;        // Keep track of where we are in the buffer.
#define delims ","             // Chars used to split strings

// Vars for time update ========================================================
struct Update_Date {
    int YYYY;
    int MO;
    int DD;
    int HH;
    int MM;
    int SS;
};
struct Update_Date update_date;


void setup() {
    Serial.begin(57600); // Begin serial debug

    #ifndef ESP8266
        while (!Serial); // wait for serial port to connect. Needed for native USB
    #endif

    if (! rtc.begin()) {
        Serial.println(F("Couldn't find RTC"));
        Serial.flush();
        abort();
    }
    else {
        Serial.println(F("RTC found."));
    }

    if (rtc.lostPower()) {
        Serial.println(F("RTC lost power, setting default time and date..."));
        // default is 1970 Jan 01 00:00:00
        rtc.adjust(DateTime(1970, 01, 01, 00, 00, 00));
    }

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }
    // Clear the buffer
    display.clearDisplay();
    
}

void loop() {
    DateTime now = rtc.now();
    #if debug
        Serial.print(now.hour(), DEC);
        Serial.print(F(":"));
        Serial.print(now.minute(), DEC);
        Serial.print(F(":"));
        Serial.print(now.second(), DEC);
        Serial.println();
    #endif

    if (check_serial()) {
        // new information sent via serial.
        convert_vals();
        rtc.adjust(DateTime(update_date.YYYY, 
                            update_date.MO,
                            update_date.DD,
                            update_date.HH,
                            update_date.MM,
                            update_date.SS
                            )
                   );
    }

    display.clearDisplay();
    display.setTextSize(2); // Scale text size Nx times
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    if (now.hour() < 10) {
        display.print(F("0"));
    }
    display.print(now.hour());
    display.print(F(":"));
    if (now.minute() < 10) {
        display.print(F("0"));
    }
    display.print(now.minute());
    display.print(F(":"));
    if (now.second() < 10) {
        display.print(F("0"));
    }
    display.print(now.second());

    display.setCursor(0, 15);
    display.print(now.year());
    display.print(F(":"));
    if (now.month() < 10) {
        display.print(F("0"));
    }
    display.print(now.month());
    display.print(F(":"));
    if (now.day() < 10) {
        display.print(F("0"));
    }
    display.print(now.day());
    
    display.display();      // Show initial text
    //delay(10);
}

//==================================================================================
// Check the serial buffer for input.
boolean check_serial() {
    boolean line_found = false;
    while (Serial.available() > 0) {
        // Read a char as it comes in
        // Will throw away any additional chars when the buffer is filled
        char char_buffer = Serial.read();
        delay(1);
        if (char_buffer == '\n') {
            input_buffer[serial_index] = 0;   // String needs to be terminated by a zero.
            line_found = (serial_index > 0);  // Prevent empty line from returning True
            serial_index = 0;
        }
        else if (char_buffer == '\r') {
            // Ignore return char. Only newline ('\n') matters.
        }
        else if (serial_index < serial_buffer_size && line_found == false) {
            input_buffer[serial_index++] = char_buffer;  // automatically incremented index
        }
    }
    #if debug
        Serial.print(F(""));
        Serial.println(line_found);
    #endif
    return line_found;
}

int convert_vals() {
    // Input string should be in form: 2022,04,19,19,05,30
    //                                 YYYY,MM,DD,HH,MM,SS
    #if messages
        Serial.print(F("String received: "));
        Serial.println(input_buffer);
    #endif
    // Parse input buffer
    char *YYYY, *MO, *DD, *HH, *MM, *SS;
    YYYY = strtok(input_buffer, delims);
    MO = strtok(NULL, delims);
    DD = strtok(NULL, delims);
    HH = strtok(NULL, delims);
    MM = strtok(NULL, delims);
    SS = strtok(NULL, delims);
    
    // Convert strings to appropriate format.
    update_date.YYYY = atoi(YYYY);
    update_date.MO = atoi(MO);
    update_date.DD = atoi(DD);
    update_date.HH = atoi(HH);
    update_date.MM = atoi(MM);
    update_date.SS = atoi(SS);
    
    #if messages
        Serial.print(F("YYYY: ")); Serial.print(update_date.YYYY); Serial.print(F(", "));
        Serial.print(F("MO: ")); Serial.print(update_date.MO); Serial.print(F(", "));
        Serial.print(F("DD: ")); Serial.print(update_date.DD); Serial.print(F(", "));
        Serial.print(F("HH: ")); Serial.print(update_date.HH); Serial.print(F(", "));
        Serial.print(F("MM: ")); Serial.print(update_date.MM); Serial.print(F(", "));
        Serial.print(F("SS: ")); Serial.println(update_date.SS);
    #endif
}
