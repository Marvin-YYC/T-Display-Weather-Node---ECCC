Weather Information Display for the TTGO T-display and information from Environment Canada (ECCC) / NavCanada weather stations

This is a work in progress.

I haven't added any additional fonts or other complications at this time.  Nothing fancy . . . I did set up the left button of the T-display to control the backlight intensity.
I might change the screen to a horizontal display with larger fonts to make it easier to read with another page or two, but it's a very small display for so much data.

So far it is a basic sketch to fetch current weather and weather alert data from Environment Canada and Nav Canada weather stations.
ECCC usually updates WX once per hour at the top of the hour, but when local weather is unstable some manual stations update more frequently.  
This device updates WX info every 20 minutes and the 'Alerts' fetch every 15 minutes.  
The 'Alert' text is colour coded to match the alert status.  Grey text indicates 'no active Alerts'.  Pink text for 'Special' weather statements, Yellow text for Yellow 'Alerts', etc.

GO HERE>> https://dd.weather.gc.ca/today/observations/swob-ml/latest/  ->> to obtain the the SWOB Weather feed for your local weather station or airport. 
Note: Not all data fields are availbe for all stations all the time.  Fields showing "--" may not be available for all fetches.

GO HERE>> https://weather.gc.ca/  ->> to obtain the the ALERT RSS feed for your local alerts
Use the top right search bar and then scroll to the bottom.  Look for 'follow', the orange RSS feed icon, and select ALERTS


https://github.com/user-attachments/assets/c1f51284-04df-4b3f-b00b-24fffe2893fb  

![signal-2026-01-02-211820](https://github.com/user-attachments/assets/6d41c97f-862c-4c13-a880-23f4088796c3)
