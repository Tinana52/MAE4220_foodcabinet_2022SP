/*
  Module:  simple_feather.ino
  Function:
        Example app matching the documentation in the project
  README.md, showing how to configure a board explicitly
  Copyright notice and License:
        See LICENSE file accompanying this project.
  Author:
        Terry Moore, MCCI Corporation November 2018
        Alex Coy, ECE 4950 TA for Spring 2021
  Notes:
  This app is not complete -- it only presents skeleton
  code for the methods you must provide in order to
  use this library. However, it compiles!
*/
#ifdef COMPILE_REGRESSION_TEST
#define FILLMEIN 0
#else
#define FILLMEIN (#Don't edit this stuff. Fill in the appropriate FILLMEIN values.)
#warning "You must fill in your keys with the right values from the TTN control panel"
#endif

#include "Catena_Fram32k.h"
#include <Arduino_LoRaWAN_ttn.h>
#include <lmic.h>
#include <hal/hal.h>
#include "keys.h"
#include "HX711.h"


//For weight sensors
float calibration_factor_top (800.0); //This value is obtained using the SparkFun_HX711_Calibration sketch
float calibration_factor_middle (1350.0); //This value is obtained using the SparkFun_HX711_Calibration sketch
float calibration_factor_bottom (540.0); //This value is obtained using the SparkFun_HX711_Calibration sketch
int zero_factor; //This large value is obtained using the SparkFun_HX711_Calibration sketch

/////// Measure voltage via pin A7////////////////
#define VBATPIN A7

#define LOADCELL_DOUT_PIN_bottom  11
#define LOADCELL_SCK_PIN_bottom  12

#define LOADCELL_DOUT_PIN_middle  10
#define LOADCELL_SCK_PIN_middle  13


#define LOADCELL_DOUT_PIN_top  A0
#define LOADCELL_SCK_PIN_top   5

HX711 loadcell_top;
HX711 loadcell_middle;
HX711 loadcell_bottom;

struct __attribute__((__packed__)) pkt_fmt{
  float top_weight;
  float middle_weight;
  float bottom_weight;
  float battery_voltage;
};
pkt_fmt myPkt;

uint64_t lastTime = 0;
static uint8_t messageBuffer[8] = {0, 1, 2, 3, 4, 5, 6, 7};
#ifdef __cplusplus
extern "C"{
#endif

void myStatusCallback(void * data, bool success){
  if(success)
    Serial.println("Succeeded!");
  else
    Serial.println("Failed!");
}

#ifdef __cplusplus 
}
#endif

class cMyLoRaWAN : public Arduino_LoRaWAN_ttn {
  using Super = Arduino_LoRaWAN_ttn;
  McciCatena::cFram32k theFram;
public:
    bool begin(const Arduino_LoRaWAN::lmic_pinmap& map);
    cMyLoRaWAN() {};

protected:
    // you'll need to provide implementations for each of the following.
    virtual bool GetOtaaProvisioningInfo(Arduino_LoRaWAN::OtaaProvisioningInfo*) override;
    virtual void NetSaveSessionInfo(const SessionInfo &Info, const uint8_t *pExtraInfo, size_t nExtraInfo) override;
    virtual void NetSaveSessionState(const SessionState &State) override;
    virtual bool NetGetSessionState(SessionState &State) override;
    virtual bool GetAbpProvisioningInfo(Arduino_LoRaWAN::AbpProvisioningInfo*) override;

};

// set up the data structures.
cMyLoRaWAN myLoRaWAN {};

// The pinmap. This form is convenient if the LMIC library
// doesn't support your board and you don't want to add the
// configuration to the library (perhaps you're just testing).
// This pinmap matches the FeatherM0 LoRa. See the arduino-lmic
// docs for more info on how to set this up.
const cMyLoRaWAN::lmic_pinmap myPinMap = {
  .nss = 8,
  .rxtx = cMyLoRaWAN::lmic_pinmap::LMIC_UNUSED_PIN,
  .rst = 4,
  .dio = { 3, 6, cMyLoRaWAN::lmic_pinmap::LMIC_UNUSED_PIN },
  .rxtx_rx_active = 0,
  .rssi_cal = 0,
  .spi_freq = 8000000,
};
void setup() {
    // simply pass the pinmap to the begin() method.
    //Serial.begin(115200);
    //loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    //loadcell.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
    //loadcell.set_offset(zero_factor); //Zero out the scale using a previously known zero_factor
    //loadcell.tare();  //Assuming there is no weight on the scale at start up, reset the scale to 0

   Serial.begin(9600);
//    Serial.println("HX711 calibration sketch");
//    Serial.println("Remove all weight from scale");
//    Serial.println("After readings begin, place known weight on scale");
//    Serial.println("Press + or a to increase calibration factor");
//    Serial.println("Press - or z to decrease calibration factor");

    /////// Measure voltage via pin A7////////////////
    float measuredvbat = analogRead(VBATPIN);
    measuredvbat *= 2; // we divided by 2, so multiply back 
    measuredvbat *= 3.3; // Multiply by 3.3V, our reference voltage 
    measuredvbat /= 1024; // convert to voltage
    
    loadcell_top.begin(LOADCELL_DOUT_PIN_top, LOADCELL_SCK_PIN_top);
    loadcell_middle.begin(LOADCELL_DOUT_PIN_middle, LOADCELL_SCK_PIN_middle);
    loadcell_bottom.begin(LOADCELL_DOUT_PIN_bottom, LOADCELL_SCK_PIN_bottom);
    
    loadcell_top.set_scale();
    loadcell_middle.set_scale();
    loadcell_bottom.set_scale();

    long zero_factor_top = loadcell_top.read_average(); //Get a baseline reading
    long zero_factor_middle = loadcell_middle.read_average(); //Get a baseline reading
    long zero_factor_bottom = loadcell_bottom.read_average(); //Get a baseline reading
    Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
    Serial.println(zero_factor);
    
    loadcell_top.tare();  //Reset the scale to 0
    loadcell_middle.tare();  //Reset the scale to 0
    loadcell_bottom.tare();  //Reset the scale to 0
    
    {
      uint64_t lt = millis();
    while(!Serial && millis() - lt < 5000);
    }
    myLoRaWAN.begin(myPinMap);
    lastTime = millis();
    Serial.println("Serial begin");

    if(myLoRaWAN.IsProvisioned())
      Serial.println("Provisioned for something");
    else
      Serial.println("Not provisioned.");
    LMIC.datarate = 10;
    myPkt.top_weight=loadcell_top.get_units();
    myPkt.middle_weight=loadcell_middle.get_units();
    myPkt.bottom_weight=loadcell_bottom.get_units();
    myPkt.battery_voltage=measuredvbat;
    myLoRaWAN.SendBuffer((uint8_t *) &myPkt, sizeof(myPkt), myStatusCallback, NULL, false, 1);
}


void loop() {
    loadcell_top.set_scale(calibration_factor_top); //Adjust to this calibration factor
    loadcell_middle.set_scale(calibration_factor_middle); //Adjust to this calibration factor
    loadcell_bottom.set_scale(calibration_factor_bottom); //Adjust to this calibration factor
    //loadcell.set_offset(zero_factor);
    myLoRaWAN.loop();
    if (millis() - lastTime > 6000){
      myPkt.top_weight=loadcell_top.get_units();
      myPkt.middle_weight=loadcell_middle.get_units();
      myPkt.bottom_weight=loadcell_bottom.get_units();
      myPkt.battery_voltage=(((analogRead(VBATPIN) * 2) * 3.3) / 1024);
      //messageBuffer[0]++;     
      myLoRaWAN.SendBuffer((uint8_t *) &myPkt, sizeof(myPkt), myStatusCallback, NULL, false, 1);
      lastTime = millis();
    }
}

// this method is called when the LMIC needs OTAA info.
// return false to indicate "no provisioning", otherwise
// fill in the data and return true.
bool
cMyLoRaWAN::GetOtaaProvisioningInfo(
    OtaaProvisioningInfo *pInfo
    ) {
      if (pInfo){
        memcpy_P(pInfo->AppEUI, APPEUI, 8);
        memcpy_P(pInfo->DevEUI, DEVEUI, 8);
        memcpy_P(pInfo->AppKey, APPKEY, 16);
      }
    return true;
}

void
cMyLoRaWAN::NetSaveSessionInfo(
    const SessionInfo &Info,
    const uint8_t *pExtraInfo,
    size_t nExtraInfo
    ) {
       theFram.saveField(McciCatena::cFramStorage::kDevAddr, Info.V2.DevAddr);
       theFram.saveField(McciCatena::cFramStorage::kNetID, Info.V2.NetID);
       theFram.saveField(McciCatena::cFramStorage::kNwkSKey, Info.V2.NwkSKey);
       theFram.saveField(McciCatena::cFramStorage::kAppSKey, Info.V2.AppSKey);
    // save Info somewhere.
}

void
cMyLoRaWAN::NetSaveSessionState(const SessionState &State) {
    // save State somwwhere. Note that it's often the same;
    // often only the frame counters change.
    theFram.saveField(McciCatena::cFramStorage::kLmicSessionState, State);
}

bool
cMyLoRaWAN::NetGetSessionState(SessionState &State) {
    // either fetch SessionState from somewhere and return true or...
    return theFram.getField(McciCatena::cFramStorage::kLmicSessionState, State);
}

bool
cMyLoRaWAN::GetAbpProvisioningInfo(Arduino_LoRaWAN::AbpProvisioningInfo* Info){
  //either get ABP provisioning info from somewhere and return true or...
  if (!Info) return false;//Library calls with null pointer sometimes
  if (!theFram.getField(McciCatena::cFramStorage::kNwkSKey, Info->NwkSKey)) return false;
  if (!theFram.getField(McciCatena::cFramStorage::kNetID, Info->NetID)) return false;
  if (!theFram.getField(McciCatena::cFramStorage::kDevAddr, Info->DevAddr)) return false;
  if (!theFram.getField(McciCatena::cFramStorage::kAppSKey, Info->AppSKey)) return false;

  SessionState state;
  if (!NetGetSessionState(state)) return false;
  Info->FCntUp=state.V1.FCntUp;
  Info->FCntDown=state.V1.FCntDown;
  return true;
}

bool cMyLoRaWAN::begin(const Arduino_LoRaWAN::lmic_pinmap&  map){
     if(!theFram.begin()){
        Serial.println("Fram begin fail");
        return false;
      }
      if(
      !theFram.initialize()){
        Serial.println("Fram not valid");
        return false;
      }
      if (!Super::begin(map))
        return false;
      return true;
                }
