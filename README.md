  **Weather Information Display for the TTGO T-display and information from Environment Canada (ECCC) / NavCanada weather stations**

This is a work in progress.

I haven't added any additional fonts, weather icons, or other complications at this time.  Nothing fancy . . . I did set up the left button of the T-display to control the backlight intensity.
DONE>> I might change the screen to a horizontal display with larger fonts to make it easier to read with another page or two, but it's a very small display for so much data.

So far it is a basic sketch to fetch current weather and weather alert data from Environment Canada and Nav Canada weather stations.
ECCC stations usually updates WX once per hour at the top of the hour, but when local weather is unstable some MAN stations update more frequently.
Some AUTO stations may update every 60 seconds, but data is limited.  Not all stations provide all the info that I have setup in this sketch'
You will probably want to adjust the sketch to work best for your station.

This device updates WX info every 15 minutes and the 'Alerts' fetch every 20 minutes.  

The 'Alert' text is colour coded to match the alert status.  Grey text indicates 'no active Alerts'.  Blue text for 'Special' weather statements, Yellow text for Yellow 'Alerts', etc. 
Jan 6/25 Added RGB LED Alert to the code.  There is no onboard RGB LED so you will need to add your own common-cathode RGB LED.  Red GPIO 25 via 220–330Ω resistor, Green GPIO 26 via resistor, Blue GPIO 27 via resistor, Cathode GND common ground. The LED flashes 3 times when an alert is initially received using the same colours as noted for the alert text.

    Jan 5/26
    Added vertical layout, so i could make the text larger.  Looks better and is much easier to read.  
    Also with the additional pages I was able to add more data.
    FYI: Sometime ECCC doesn't provide all the data for each field during every update.  
    If lines are blank, it is not an error, but just that the data wasn't availble during that update. 
    For example if there are no wind gusts, the field will show "--" if "MSNG" appears it is becuse ECCC didn't send the data but did update the line with MSNG.

GO HERE>> https://dd.weather.gc.ca/today/observations/swob-ml/latest/  ->> to obtain the the SWOB Weather feed for your local weather station or airport. 
Note: Not all data fields are available for all stations all the time.  Fields showing "--" may not be available for all fetches.
Sortable Station List here>> https://dd.weather.gc.ca/today/observations/doc/swob-xml_station_list.csv

GO HERE>> https://weather.gc.ca/  ->> to obtain the the ALERT RSS feed for your local alerts
Use the top right search bar and then scroll to the bottom.  Look for 'follow', the orange RSS feed icon, and select ALERTS

    IMPORTANT - Edits required for User_Setup.h file uncomment these lines and edit as required for TTGO T_Display.
^ line 12 - #define USER_SETUP_INFO "User_Setup" 
^ line 55 - #define ST7789_DRIVER 
^ line 84 - #define TFT_WIDTH  135
^ line 90 - #define TFT_HEIGHT 240 
^ line 131 - #define TFT_BL   4 
^ line 207 - #define TFT_MOSI 19
^ line 208 - #define TFT_SCLK 18
^ line 209 - #define TFT_CS   -1
^ line 210 - #define TFT_DC    16
^ line 211 - #define TFT_RST   23
^ Leave other lines as they are.

    IMPORTANT - one edit required for User_Setup_Select.h file uncomment this line.
line 61 - #include <User_Setups/Setup25_TTGO_T_Display.h>    // Setup file for ESP32 and TTGO T-Display ST7789V SPI bus TFT

Horizontal Layout
![signal-2026-01-05-204349](https://github.com/user-attachments/assets/7ce37b1f-3414-4995-a689-dc3c45fdd966)

![signal-2026-01-05-204349_006](https://github.com/user-attachments/assets/c1c86122-faa9-4fd7-beac-4dfb4fbb8947)

![signal-2026-01-05-204349_003](https://github.com/user-attachments/assets/0808bc18-031a-4a4f-ab33-047ea8318a9c)

![signal-2026-01-05-204349_004](https://github.com/user-attachments/assets/f2ab6b5a-88b7-49c1-943f-e4070a7143b5)

![signal-2026-01-05-204349_002](https://github.com/user-attachments/assets/dc2145d2-97a8-466c-a035-94b8bc59b186)

Vertical Layout

https://github.com/user-attachments/assets/c1f51284-04df-4b3f-b00b-24fffe2893fb  

![signal-2026-01-02-211820](https://github.com/user-attachments/assets/6d41c97f-862c-4c13-a880-23f4088796c3)
