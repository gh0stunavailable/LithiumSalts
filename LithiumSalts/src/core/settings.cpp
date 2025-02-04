#include <globals.h>
#include "settings.h"
#include "display.h"  // calling loopOptions(options, true);
#include "wifi_common.h"
#include "utils.h"
#include "mykeyboard.h"
#include "sd_functions.h"
#include "powerSave.h"
#include "modules/rf/rf.h"  // for initRfModule

#ifdef USE_CC1101_VIA_SPI
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#endif

// This function comes from interface.h
void _setBrightness(uint8_t brightval) { }

/*********************************************************************
**  Function: setBrightness
**  set brightness value
**********************************************************************/
void setBrightness(uint8_t brightval, bool save) {
  if(LithiumSaltsConfig.bright>100) LithiumSaltsConfig.setBright(100);
  _setBrightness(brightval);
  delay(10);

  if(save){
    LithiumSaltsConfig.setBright(brightval);
  }
}

/*********************************************************************
**  Function: getBrightness
**  get brightness value
**********************************************************************/
void getBrightness() {
  if(LithiumSaltsConfig.bright>100) {
    LithiumSaltsConfig.setBright(100);
    _setBrightness(LithiumSaltsConfig.bright);
    delay(10);
    setBrightness(100);
  }

  _setBrightness(LithiumSaltsConfig.bright);
  delay(10);
}

/*********************************************************************
**  Function: gsetRotation
**  get/set rotation value
**********************************************************************/
int gsetRotation(bool set){
  int getRot = LithiumSaltsConfig.rotation;
  int result = ROTATION;

  #if TFT_WIDTH>=240 && TFT_HEIGHT>=240
  getRot++;
  if(getRot>3 && set) result = 0;
  else if(set) result = getRot;
  else if(getRot<=3) result = getRot;
  else {
    set=true;
    result = ROTATION;
  }
  #else
  if(getRot==1 && set) result = 3;
  else if(getRot==3 && set) result = 1;
  else if(getRot<=3) result = getRot;
  else {
    set=true;
    result = ROTATION;
  }
  #endif

  if(set) {
    LithiumSaltsConfig.setRotation(result);
    tft.setRotation(result);
    tft.setRotation(result); // must repeat, sometimes ESP32S3 miss one SPI command and it just jumps this step and don't rotate
  }
  returnToMenu=true;

  if(result & 0b01) { // if 1 or 3
      tftWidth=TFT_HEIGHT;
      #if defined(HAS_TOUCH)
        tftHeight=TFT_WIDTH - 20;
      #else
        tftHeight=TFT_WIDTH;
      #endif
  } else { // if 2 or 0
      tftWidth=TFT_WIDTH;
      #if defined(HAS_TOUCH)
      tftHeight=TFT_HEIGHT-20;
      #else
      tftHeight=TFT_HEIGHT;
      #endif
  }
  return result;
}

/*********************************************************************
**  Function: setBrightnessMenu
**  Handles Menu to set brightness
**********************************************************************/
void setBrightnessMenu() {
  int idx=0;
  if(LithiumSaltsConfig.bright==100) idx=0;
  else if(LithiumSaltsConfig.bright==75) idx=1;
  else if(LithiumSaltsConfig.bright==50) idx=2;
  else if(LithiumSaltsConfig.bright==25) idx=3;
  else if(LithiumSaltsConfig.bright== 1) idx=4;

  options = {
    {"100%", [=]() { setBrightness((uint8_t)100); }, LithiumSaltsConfig.bright == 100 },
    {"75 %", [=]() { setBrightness((uint8_t)75);  }, LithiumSaltsConfig.bright == 75 },
    {"50 %", [=]() { setBrightness((uint8_t)50);  }, LithiumSaltsConfig.bright == 50 },
    {"25 %", [=]() { setBrightness((uint8_t)25);  }, LithiumSaltsConfig.bright == 25 },
    {" 1 %", [=]() { setBrightness((uint8_t)1);   }, LithiumSaltsConfig.bright == 1 },
    {"Main Menu", [=]() { backToMenu(); }}, // this one bugs the brightness selection
  };
  loopOptions(options, true,false,"",idx);
}


/*********************************************************************
**  Function: setSleepMode
**  Turn screen off and reduces cpu clock
**********************************************************************/
void setSleepMode() {
  sleepModeOn();
  while (1) {
    if (check(AnyKeyPress)) {
      sleepModeOff();
      returnToMenu = true;
      break;
    }
  }
}

/*********************************************************************
**  Function: setDimmerTimeMenu
**  Handles Menu to set dimmer time
**********************************************************************/
void setDimmerTimeMenu() {
  int idx=0;
  if(LithiumSaltsConfig.dimmerSet==10) idx=0;
  else if(LithiumSaltsConfig.dimmerSet==20) idx=1;
  else if(LithiumSaltsConfig.dimmerSet==30) idx=2;
  else if(LithiumSaltsConfig.dimmerSet==60) idx=3;
  else if(LithiumSaltsConfig.dimmerSet== 0) idx=4;
  options = {
    {"10s", [=]() { LithiumSaltsConfig.setDimmer(10); }, LithiumSaltsConfig.dimmerSet == 10 },
    {"20s", [=]() { LithiumSaltsConfig.setDimmer(20); }, LithiumSaltsConfig.dimmerSet == 20 },
    {"30s", [=]() { LithiumSaltsConfig.setDimmer(30); }, LithiumSaltsConfig.dimmerSet == 30 },
    {"60s", [=]() { LithiumSaltsConfig.setDimmer(60); }, LithiumSaltsConfig.dimmerSet == 60 },
    {"Disabled", [=]() { LithiumSaltsConfig.setDimmer(0); }, LithiumSaltsConfig.dimmerSet == 0 },
  };
  loopOptions(options,idx);
}

/*********************************************************************
**  Function: setUIColor
**  Set and store main UI color
**********************************************************************/
#define LIGHT_BLUE 0x96FE
void setUIColor(){
  int idx=0;
  if(LithiumSaltsConfig.priColor==DEFAULT_PRICOLOR) idx=0;
  else if(LithiumSaltsConfig.priColor==TFT_WHITE) idx=1;
  else if(LithiumSaltsConfig.priColor==TFT_RED) idx=2;
  else if(LithiumSaltsConfig.priColor==TFT_DARKGREEN) idx=3;
  else if(LithiumSaltsConfig.priColor==TFT_BLUE) idx=4;
  else if(LithiumSaltsConfig.priColor==LIGHT_BLUE) idx=5;
  else if(LithiumSaltsConfig.priColor==TFT_YELLOW) idx=6;
  else if(LithiumSaltsConfig.priColor==TFT_MAGENTA) idx=7;
  else if(LithiumSaltsConfig.priColor==TFT_ORANGE) idx=8;
  else idx=9;  // custom theme

  options = {
    {"Default",   [=]() { LithiumSaltsConfig.setTheme(DEFAULT_PRICOLOR);}, LithiumSaltsConfig.priColor==DEFAULT_PRICOLOR},
    {"White",     [=]() { LithiumSaltsConfig.setTheme(TFT_WHITE);     }, LithiumSaltsConfig.priColor==TFT_WHITE     },
    {"Red",       [=]() { LithiumSaltsConfig.setTheme(TFT_RED);       }, LithiumSaltsConfig.priColor==TFT_RED       },
    {"Green",     [=]() { LithiumSaltsConfig.setTheme(TFT_DARKGREEN); }, LithiumSaltsConfig.priColor==TFT_DARKGREEN },
    {"Blue",      [=]() { LithiumSaltsConfig.setTheme(TFT_BLUE);      }, LithiumSaltsConfig.priColor==TFT_BLUE      },
    {"Light Blue",[=]() { LithiumSaltsConfig.setTheme(LIGHT_BLUE);    }, LithiumSaltsConfig.priColor==LIGHT_BLUE    },
    {"Yellow",    [=]() { LithiumSaltsConfig.setTheme(TFT_YELLOW);    }, LithiumSaltsConfig.priColor==TFT_YELLOW    },
    {"Magenta",   [=]() { LithiumSaltsConfig.setTheme(TFT_MAGENTA);   }, LithiumSaltsConfig.priColor==TFT_MAGENTA   },
    {"Orange",    [=]() { LithiumSaltsConfig.setTheme(TFT_ORANGE);    }, LithiumSaltsConfig.priColor==TFT_ORANGE    },
  };

  if (idx == 9) options.push_back({"Custom Theme", [=]() { backToMenu(); }, true});
  options.push_back({"Main Menu", [=]() { backToMenu(); }});

  loopOptions(options, idx);
  tft.setTextColor(LithiumSaltsConfig.bgColor, LithiumSaltsConfig.priColor);
}

/*********************************************************************
**  Function: setSoundConfig
**  Enable or disable sound
**********************************************************************/
void setSoundConfig() {
  options = {
    {"Sound off", [=]() { LithiumSaltsConfig.setSoundEnabled(0); }, LithiumSaltsConfig.soundEnabled == 0},
    {"Sound on",  [=]() { LithiumSaltsConfig.setSoundEnabled(1); }, LithiumSaltsConfig.soundEnabled == 1},
  };
  loopOptions(options, LithiumSaltsConfig.soundEnabled);
}

/*********************************************************************
**  Function: setWifiStartupConfig
**  Enable or disable wifi connection at startup
**********************************************************************/
void setWifiStartupConfig() {
  options = {
    {"Disable", [=]() { LithiumSaltsConfig.setWifiAtStartup(0); }, LithiumSaltsConfig.wifiAtStartup == 0},
    {"Enable",  [=]() { LithiumSaltsConfig.setWifiAtStartup(1); }, LithiumSaltsConfig.wifiAtStartup == 1},
  };
  loopOptions(options, LithiumSaltsConfig.wifiAtStartup);
}

/*********************************************************************
**  Function: setRFModuleMenu
**  Handles Menu to set the RF module in use
**********************************************************************/
void setRFModuleMenu() {
  int result = 0;
  int idx=0;
  if(LithiumSaltsConfig.rfModule==M5_RF_MODULE) idx=0;
  else if(LithiumSaltsConfig.rfModule==CC1101_SPI_MODULE) idx=1;

  options = {
    {"M5 RF433T/R",    [&]() { result = M5_RF_MODULE; }},
#ifdef USE_CC1101_VIA_SPI
    {"CC1101 on SPI",  [&]() { result = CC1101_SPI_MODULE; }},
#endif
/* WIP:
 * #ifdef USE_CC1101_VIA_PCA9554
 * {"CC1101+PCA9554",  [&]() { result = 2; }},
 * #endif
*/
  };
  loopOptions(options, idx);  // 2fix: idx highlight not working?
  if(result == CC1101_SPI_MODULE) {
    #ifdef USE_CC1101_VIA_SPI
    ELECHOUSE_cc1101.Init();
    if (ELECHOUSE_cc1101.getCC1101()){
      LithiumSaltsConfig.setRfModule(CC1101_SPI_MODULE);
      return;
    }
    #endif
    // else display an error
    displayError("CC1101 not found");
    while(!check(AnyKeyPress));
  }
  // fallback to "M5 RF433T/R" on errors
  LithiumSaltsConfig.setRfModule(M5_RF_MODULE);
}

/*********************************************************************
**  Function: setRFFreqMenu
**  Handles Menu to set the default frequency for the RF module
**********************************************************************/
void setRFFreqMenu() {
  float result = 433.92;
  String freq_str = keyboard(String(LithiumSaltsConfig.rfFreq), 10, "Default frequency:");
  if(freq_str.length() > 1) {
    result = freq_str.toFloat();  // returns 0 if not valid
    if(result>=300 && result<=928) { // TODO: check valid freq according to current module?
      LithiumSaltsConfig.setRfFreq(result);
      return;
    }
  }
  // else
  displayError("Invalid frequency");
  LithiumSaltsConfig.setRfFreq(433.92);  // reset to default
  delay(1000);
}

/*********************************************************************
**  Function: setRFIDModuleMenu
**  Handles Menu to set the RFID module in use
**********************************************************************/
void setRFIDModuleMenu() {
  options = {
    {"M5 RFID2",      [=]() { LithiumSaltsConfig.setRfidModule(M5_RFID2_MODULE); },  LithiumSaltsConfig.rfidModule == M5_RFID2_MODULE},
    {"PN532 on I2C",  [=]() { LithiumSaltsConfig.setRfidModule(PN532_I2C_MODULE); }, LithiumSaltsConfig.rfidModule == PN532_I2C_MODULE},
    {"PN532 on SPI",  [=]() { LithiumSaltsConfig.setRfidModule(PN532_SPI_MODULE); }, LithiumSaltsConfig.rfidModule == PN532_SPI_MODULE},
  };
  loopOptions(options, LithiumSaltsConfig.rfidModule);
}

/*********************************************************************
**  Function: addMifareKeyMenu
**  Handles Menu to add MIFARE keys into config list
**********************************************************************/
void addMifareKeyMenu() {
  String key = keyboard("", 12, "MIFARE key");
  LithiumSaltsConfig.addMifareKey(key);
}


/*********************************************************************
**  Function: setClock
**  Handles Menu to set timezone to NTP
**********************************************************************/
const char* ntpServer = "pool.ntp.org";
long  selectedTimezone;
const int   daylightOffset_sec = 0;
int timeHour;

TimeChangeRule BRST = {"BRST", Last, Sun, Oct, 0, timeHour};
Timezone myTZ(BRST, BRST);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, selectedTimezone, daylightOffset_sec);

void setClock() {
  bool auto_mode=true;

  #if defined(HAS_RTC)
    RTC_TimeTypeDef TimeStruct;
    cplus_RTC _rtc;
    _rtc.GetBm8563Time();
  #endif

  options = {
    {"NTP Timezone", [&]() { auto_mode=true; }},
    {"Manually set", [&]() { auto_mode=false; }},
    {"Main Menu",    [=]() { backToMenu(); }},
  };
  loopOptions(options);

  if (returnToMenu) return;

  if (auto_mode) {
    if(!wifiConnected) wifiConnectMenu();

    options = {
      {"Los Angeles", [&]() { LithiumSaltsConfig.setTmz(-8); }, LithiumSaltsConfig.tmz==-8 },
      {"Chicago",     [&]() { LithiumSaltsConfig.setTmz(-6); }, LithiumSaltsConfig.tmz==-6 },
      {"New York",    [&]() { LithiumSaltsConfig.setTmz(-5); }, LithiumSaltsConfig.tmz==-5 },
      {"Brasilia",    [&]() { LithiumSaltsConfig.setTmz(-3); }, LithiumSaltsConfig.tmz==-3 },
      {"Pernambuco",  [&]() { LithiumSaltsConfig.setTmz(-2); }, LithiumSaltsConfig.tmz==-2 },
      {"Lisbon",      [&]() { LithiumSaltsConfig.setTmz(0);  }, LithiumSaltsConfig.tmz==0  },
      {"Paris",       [&]() { LithiumSaltsConfig.setTmz(1);  }, LithiumSaltsConfig.tmz==1  },
      {"Athens",      [&]() { LithiumSaltsConfig.setTmz(2);  }, LithiumSaltsConfig.tmz==2  },
      {"Moscow",      [&]() { LithiumSaltsConfig.setTmz(3);  }, LithiumSaltsConfig.tmz==3  },
      {"Dubai",       [&]() { LithiumSaltsConfig.setTmz(4);  }, LithiumSaltsConfig.tmz==4  },
      {"Hong Kong",   [&]() { LithiumSaltsConfig.setTmz(8);  }, LithiumSaltsConfig.tmz==8  },
      {"Tokyo",       [&]() { LithiumSaltsConfig.setTmz(9);  }, LithiumSaltsConfig.tmz==9  },
      {"Sydney",      [&]() { LithiumSaltsConfig.setTmz(10); }, LithiumSaltsConfig.tmz==10 },
      {"Main Menu",   [=]() { backToMenu(); }},
    };


    loopOptions(options);


    if (returnToMenu) return;

    timeClient.setTimeOffset(LithiumSaltsConfig.tmz * 3600);
    timeClient.begin();
    timeClient.update();
    localTime = myTZ.toLocal(timeClient.getEpochTime());

    #if defined(HAS_RTC)
      struct tm *timeinfo = localtime(&localTime);
      TimeStruct.Hours   = timeinfo->tm_hour;
      TimeStruct.Minutes = timeinfo->tm_min;
      TimeStruct.Seconds = timeinfo->tm_sec;
      _rtc.SetTime(&TimeStruct);
    #else
      rtc.setTime(timeClient.getEpochTime());
    #endif

    clock_set = true;
    runClockLoop();
  }
  else {
    int hr, mn, am;
    options = { };
    for(int i=0; i<12;i++) options.push_back({String(String(i<10?"0":"") + String(i)).c_str(), [&]() { delay(1); }});


    hr=loopOptions(options,false,true,"Set Hour");

    options = { };
    for(int i=0; i<60;i++) options.push_back({String(String(i<10?"0":"") + String(i)).c_str(), [&]() { delay(1); }});

    mn=loopOptions(options,false,true,"Set Minute");

    options = {
      {"AM", [&]() { am=0; }},
      {"PM", [&]() { am=12; }},
    };

    loopOptions(options);


    #if defined(HAS_RTC)
      TimeStruct.Hours   = hr+am;
      TimeStruct.Minutes = mn;
      TimeStruct.Seconds = 0;
      _rtc.SetTime(&TimeStruct);
    #else
      rtc.setTime(0,mn,hr+am,20,06,2024); // send me a gift, @Pirata!
    #endif
    clock_set = true;
    runClockLoop();
  }
}

void runClockLoop() {
  int tmp=0;

  #if defined(HAS_RTC)
    RTC_TimeTypeDef _time;
    cplus_RTC _rtc;
    _rtc.GetBm8563Time();
    _rtc.GetTime(&_time);
  #endif

  // Delay due to SelPress() detected on run
  tft.fillScreen(LithiumSaltsConfig.bgColor);
  tft.fillScreen(LithiumSaltsConfig.bgColor);
  delay(300);

  for (;;){
  if(millis()-tmp>1000) {
    #if !defined(HAS_RTC)
      updateTimeStr(rtc.getTimeStruct());
    #endif
    Serial.print("Current time: ");
    Serial.println(timeStr);
    tft.setTextColor(LithiumSaltsConfig.priColor, LithiumSaltsConfig.bgColor);
    tft.drawRect(10, 10, tftWidth-15,tftHeight-15, LithiumSaltsConfig.priColor);
    tft.setCursor(64, tftHeight/3+5);
    tft.setTextSize(4);
    #if defined(HAS_RTC)
      _rtc.GetBm8563Time();
      _rtc.GetTime(&_time);
      char timeString[9];  // Buffer para armazenar a string formatada "HH:MM:SS"
      snprintf(timeString, sizeof(timeString), "%02d:%02d:%02d", _time.Hours, _time.Minutes, _time.Seconds);
      tft.drawCentreString(timeString,tftWidth/2,tftHeight/2-13,1);
    #else
      tft.drawCentreString(timeStr,tftWidth/2,tftHeight/2-13,1);
    #endif
    tmp=millis();
  }

    // Checks para sair do loop
    if(check(SelPress) or check(EscPress)) { // Apertar o botão power dos sticks
      tft.fillScreen(LithiumSaltsConfig.bgColor);
      returnToMenu=true;
      break;
      //goto Exit;
    }
    delay(10);
  }
}

/*********************************************************************
**  Function: gsetIrTxPin
**  get or set IR Tx Pin
**********************************************************************/
int gsetIrTxPin(bool set){
  int result = LithiumSaltsConfig.irTx;

  if(result>50) LithiumSaltsConfig.setIrTxPin(LED);
  if(set) {
    options.clear();
    std::vector<std::pair<std::string, int>> pins;
    pins = IR_TX_PINS;
    int idx=100;
    int j=0;
    for (auto pin : pins) {
      if(pin.second==LithiumSaltsConfig.irTx && idx==100) idx=j;
      j++;
      #ifdef ALLOW_ALL_GPIO_FOR_IR_RF
      int i=pin.second;
      if(i!=TFT_CS && i!=TFT_RST && i!=TFT_SCLK && i!=TFT_MOSI && i!=TFT_BL && i!=TOUCH_CS && i!=SDCARD_CS && i!=SDCARD_MOSI && i!=SDCARD_MISO)
      #endif
        options.push_back({pin.first, [=]() { LithiumSaltsConfig.setIrTxPin(pin.second); }, pin.second==LithiumSaltsConfig.irTx });
    }

    loopOptions(options, idx);

    Serial.println("Saved pin: " + String(LithiumSaltsConfig.irTx));
  }

  returnToMenu=true;
  return LithiumSaltsConfig.irTx;
}

/*********************************************************************
**  Function: gsetIrRxPin
**  get or set IR Rx Pin
**********************************************************************/
int gsetIrRxPin(bool set){
  int result = LithiumSaltsConfig.irRx;

  if(result>45) LithiumSaltsConfig.setIrRxPin(GROVE_SCL);
  if(set) {
    options.clear();
    std::vector<std::pair<std::string, int>> pins;
    pins = IR_RX_PINS;
    int idx=-1;
    int j=0;
    for (auto pin : pins) {
      if(pin.second==LithiumSaltsConfig.irRx && idx<0) idx=j;
      j++;
      #ifdef ALLOW_ALL_GPIO_FOR_IR_RF
      int i=pin.second;
      if(i!=TFT_CS && i!=TFT_RST && i!=TFT_SCLK && i!=TFT_MOSI && i!=TFT_BL && i!=TOUCH_CS && i!=SDCARD_CS && i!=SDCARD_MOSI && i!=SDCARD_MISO)
      #endif
        options.push_back({pin.first, [=]() {LithiumSaltsConfig.setIrRxPin(pin.second);}, pin.second==LithiumSaltsConfig.irRx });
    }

    loopOptions(options);

  }

  returnToMenu=true;
  return LithiumSaltsConfig.irRx;
}

/*********************************************************************
**  Function: gsetRfTxPin
**  get or set RF Tx Pin
**********************************************************************/
int gsetRfTxPin(bool set){
  int result = LithiumSaltsConfig.rfTx;

  if(result>45) LithiumSaltsConfig.setRfTxPin(GROVE_SDA);
  if(set) {
    options.clear();
    std::vector<std::pair<std::string, int>> pins;
    pins = RF_TX_PINS;
    int idx=-1;
    int j=0;
    for (auto pin : pins) {
      if(pin.second==LithiumSaltsConfig.rfTx && idx<0) idx=j;
      j++;
      #ifdef ALLOW_ALL_GPIO_FOR_IR_RF
      int i=pin.second;
      if(i!=TFT_CS && i!=TFT_RST && i!=TFT_SCLK && i!=TFT_MOSI && i!=TFT_BL && i!=TOUCH_CS && i!=SDCARD_CS && i!=SDCARD_MOSI && i!=SDCARD_MISO)
      #endif
        options.push_back({pin.first, [=]() {LithiumSaltsConfig.setRfTxPin(pin.second);}, pin.second==LithiumSaltsConfig.rfTx });
    }

    loopOptions(options);

  }

  returnToMenu=true;
  return LithiumSaltsConfig.rfTx;
}

/*********************************************************************
**  Function: gsetRfRxPin
**  get or set FR Rx Pin
**********************************************************************/
int gsetRfRxPin(bool set){
  int result = LithiumSaltsConfig.rfRx;

  if(result>36) LithiumSaltsConfig.setRfRxPin(GROVE_SCL);
  if(set) {
    options.clear();
    std::vector<std::pair<std::string, int>> pins;
    pins = RF_RX_PINS;
    int idx=-1;
    int j=0;
    for (auto pin : pins) {
      if(pin.second==LithiumSaltsConfig.rfRx && idx<0) idx=j;
      j++;
      #ifdef ALLOW_ALL_GPIO_FOR_IR_RF
      int i=pin.second;
      if(i!=TFT_CS && i!=TFT_RST && i!=TFT_SCLK && i!=TFT_MOSI && i!=TFT_BL && i!=TOUCH_CS && i!=SDCARD_CS && i!=SDCARD_MOSI && i!=SDCARD_MISO)
      #endif
        options.push_back({pin.first, [=]() {LithiumSaltsConfig.setRfRxPin(pin.second);}, pin.second==LithiumSaltsConfig.rfRx });
    }

    loopOptions(options);

  }

  returnToMenu=true;
  return LithiumSaltsConfig.rfRx;
}

/*********************************************************************
**  Function: setStartupApp
**  Handles Menu to set startup app
**********************************************************************/
void setStartupApp() {
  int idx = 0;
  if (LithiumSaltsConfig.startupApp == "") idx=0;

  options = {
    {"None", [=]() { LithiumSaltsConfig.setStartupApp(""); }, LithiumSaltsConfig.startupApp == "" }
  };

  int index = 1;
  for (String appName : startupApp.getAppNames()) {
    if (LithiumSaltsConfig.startupApp == appName) idx=index++;

    options.emplace_back(
      appName.c_str(),
      [=]() { LithiumSaltsConfig.setStartupApp(appName); },
      LithiumSaltsConfig.startupApp == appName
    );
  }


  loopOptions(options, idx);

}

/*********************************************************************
**  Function: setGpsBaudrateMenu
**  Handles Menu to set the baudrate for the GPS module
**********************************************************************/
void setGpsBaudrateMenu() {
  options = {
    {"9600 bps",   [=]() { LithiumSaltsConfig.setGpsBaudrate(9600); }, LithiumSaltsConfig.gpsBaudrate == 9600},
    {"19200 bps",  [=]() { LithiumSaltsConfig.setGpsBaudrate(19200); }, LithiumSaltsConfig.gpsBaudrate == 19200},
    {"57600 bps",  [=]() { LithiumSaltsConfig.setGpsBaudrate(57600); }, LithiumSaltsConfig.gpsBaudrate == 57600},
    {"115200 bps", [=]() { LithiumSaltsConfig.setGpsBaudrate(115200); }, LithiumSaltsConfig.gpsBaudrate == 115200},
  };

  loopOptions(options, LithiumSaltsConfig.gpsBaudrate);

}

/*********************************************************************
**  Function: setBleNameMenu
**  Handles Menu to set BLE Gap Name
**********************************************************************/
void setBleNameMenu() {
  const String defaultBleName = "Keyboard_" + String((uint8_t)(ESP.getEfuseMac() >> 32), HEX);

  const bool isDefault = LithiumSaltsConfig.bleName == defaultBleName;

  options = {
    {"Default",   [=]() { LithiumSaltsConfig.setBleName(defaultBleName); }, isDefault},
    {"Custom",    [=]() {
      String newBleName = keyboard(LithiumSaltsConfig.bleName, 30, "BLE Device Name:");
      if (!newBleName.isEmpty()) LithiumSaltsConfig.setBleName(newBleName);
      else displayError("BLE Name cannot be empty", true);
    }, !isDefault},
    {"Main Menu", [=]() { backToMenu(); }},
  };

  loopOptions(options, isDefault ? 0 : 1);
}

/*********************************************************************
**  Function: setWifiApSsidMenu
**  Handles Menu to set the WiFi AP SSID
**********************************************************************/
void setWifiApSsidMenu() {
  const bool isDefault = LithiumSaltsConfig.wifiAp.ssid == "LithiumSaltsNet";

  options = {
    {"Default (LithiumSaltsNet)", [=]() { LithiumSaltsConfig.setWifiApCreds("LithiumSaltsNet", LithiumSaltsConfig.wifiAp.pwd); }, isDefault},
    {"Custom",             [=]() {
      String newSsid = keyboard(LithiumSaltsConfig.wifiAp.ssid, 32, "WiFi AP SSID:");
      if (!newSsid.isEmpty()) LithiumSaltsConfig.setWifiApCreds(newSsid, LithiumSaltsConfig.wifiAp.pwd);
      else displayError("SSID cannot be empty", true);
    }, !isDefault},
    {"Main Menu",          [=]() { backToMenu(); }},
  };

  loopOptions(options, isDefault ? 0 : 1);
}

/*********************************************************************
**  Function: setWifiApPasswordMenu
**  Handles Menu to set the WiFi AP Password
**********************************************************************/
void setWifiApPasswordMenu() {
  const bool isDefault = LithiumSaltsConfig.wifiAp.pwd == "LithiumSaltsnet";

  options = {
    {"Default (LithiumSaltsnet)", [=]() { LithiumSaltsConfig.setWifiApCreds(LithiumSaltsConfig.wifiAp.ssid, "LithiumSaltsnet"); }, isDefault},
    {"Custom",             [=]() {
      String newPassword = keyboard(LithiumSaltsConfig.wifiAp.pwd, 32, "WiFi AP Password:");
      if (!newPassword.isEmpty()) LithiumSaltsConfig.setWifiApCreds(LithiumSaltsConfig.wifiAp.ssid, newPassword);
      else displayError("Password cannot be empty", true);
    }, !isDefault},
    {"Main Menu",          [=]() { backToMenu(); }},
  };

  loopOptions(options, isDefault ? 0 : 1);
}

/*********************************************************************
**  Function: setWifiApCredsMenu
**  Handles Menu to configure WiFi AP Credentials
**********************************************************************/
void setWifiApCredsMenu() {
  options = {
    {"SSID",      [=]() { setWifiApSsidMenu(); }},
    {"Password",  [=]() { setWifiApPasswordMenu(); }},
    {"Main Menu", [=]() { backToMenu(); }},
  };

  loopOptions(options);
}

/*********************************************************************
**  Function: setNetworkCredsMenu
**  Main Menu for setting Network credentials (BLE & WiFi)
**********************************************************************/
void setNetworkCredsMenu() {
  options = {
    {"WiFi AP Creds", [=]() { setWifiApCredsMenu(); }},
    {"BLE Name",      [=]() { setBleNameMenu(); }},
    {"Main Menu",     [=]() { backToMenu(); }},
  };

  loopOptions(options);
}
