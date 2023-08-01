#include <Preferences.h>
#include <MIDI.h>
#include <M5Stack.h>
#include <usbhub.h>
#include <SPI.h>
#include <BLEMIDI_Transport.h>
#include <UHS2-MIDI.h>

#include <hardware/BLEMIDI_Client_ESP32.h>

#define BLEMIDI_CREATE_CLIENT_INSTANCE(DeviceName, Name) \
  BLEMIDI_NAMESPACE::BLEMIDI_Transport<BLEMIDI_NAMESPACE::BLEMIDI_Client_ESP32> BLE##Name(DeviceName); \
  MIDI_NAMESPACE::MidiInterface<BLEMIDI_NAMESPACE::BLEMIDI_Transport<BLEMIDI_NAMESPACE::BLEMIDI_Client_ESP32>, BLEMIDI_NAMESPACE::MySettings> Name((BLEMIDI_NAMESPACE::BLEMIDI_Transport<BLEMIDI_NAMESPACE::BLEMIDI_Client_ESP32> &)BLE##Name);

#include <hardware/BLEMIDI_ESP32_NimBLE.h>

USB Usb;
USBHub Hub(&Usb);
UHS2MIDI_CREATE_INSTANCE(&Usb, 0, midiUsb);
UHS2MIDI_CREATE_INSTANCE(&Usb, 0, midiUsb1);
UHS2MIDI_CREATE_INSTANCE(&Usb, 0, midiUsb2);
UHS2MIDI_CREATE_INSTANCE(&Usb, 0, midiUsb3);
#define MIDI_UHS2_DEVICE_COUNT 4

midi::MidiInterface<uhs2Midi::uhs2MidiTransport>* list_devices_uhs2[MIDI_UHS2_DEVICE_COUNT] = {
  &midiUsb, &midiUsb1, &midiUsb2, &midiUsb3 //, &midiUsb4, &midiUsb5, &midiUsb6, &midiUsb7, &midiUsb8, &midiUsb9, &midiUsb10, &midiUsb11
};

void ReadCB(void *parameter);  //Continuos Read function (See FreeRTOS multitasks)
unsigned long t0 = millis();

BLEMIDI_CREATE_INSTANCE("mmmMegaMidiMergerBT", midiBle);
BLEMIDI_CREATE_CLIENT_INSTANCE("", midiBleClient);
#define ONBOARD_LED 2
#define ONBOARD_BUTTON_LABELED_BOOT 0

bool bleClientMode = false;
bool toggle = false;
bool isConnected = false;
bool isClientConnected = false;
Preferences preferences;

void send_uhs(midi::MidiType t, midi::DataByte d1, midi::DataByte d2, midi::Channel ch, int exclude = -1) {
  for (int i = 0; i < MIDI_UHS2_DEVICE_COUNT; i++) {
    if (exclude != i) {  // do not send to self, no passthrough
      list_devices_uhs2[i]->send(t, d1, d2, ch);
    }
  }
}
void send_uhs_sysex(midi::DataByte d1, midi::DataByte d2, const byte* sysexArray, int exclude = -1) {
  for (int i = 0; i < MIDI_UHS2_DEVICE_COUNT; i++) {
    if (exclude != i) {  // do not send to self, no passthrough
      int mSxLen = d1 + 256 * d2;
      list_devices_uhs2[i]->sendSysEx(mSxLen, sysexArray, true);
    }
  }
}
void send_ble(midi::MidiType t, midi::DataByte d1, midi::DataByte d2, midi::Channel ch) {
  if (bleClientMode) {
    midiBleClient.send(t, d1, d2, ch);
  } else {
    midiBle.send(t, d1, d2, ch);
  }
}
void send_ble_sysex(midi::DataByte d1, midi::DataByte d2, const byte* sysexArray) {
  int mSxLen = d1 + 256 * d2;
    if (bleClientMode) {
    midiBleClient.sendSysEx(mSxLen, sysexArray, true);
  } else {
    midiBle.sendSysEx(mSxLen, sysexArray, true);
  }
}

void toggleLED() {
  M5.Lcd.fillRect(5, 5, 10, 10, toggle = !toggle ? TFT_RED : TFT_BLACK);
}


void setup() {
  preferences.begin("my-app", false);

  M5.begin();
  M5.Lcd.fillScreen(TFT_NAVY);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setTextColor(TFT_MAGENTA, TFT_YELLOW);
  M5.Lcd.fillRect(0, 0, 320, 30, TFT_MAGENTA);
  M5.Lcd.println("MIDI Dumper");

  //Serial.begin(115200);
  bleClientMode = preferences.getBool("bleclientmode", false);

  pinMode(ONBOARD_LED, OUTPUT);
  // Initiate MIDI communications, listen to all channels
  if (bleClientMode) {
    midiBleClient.begin(MIDI_CHANNEL_OMNI);
    midiBleClient.turnThruOff();
  } else {
    midiBle.begin(MIDI_CHANNEL_OMNI);
    midiBle.turnThruOff();
  }

  for (int i = 0; i < MIDI_UHS2_DEVICE_COUNT; i++) {
    list_devices_uhs2[i]->begin(MIDI_CHANNEL_OMNI);
    list_devices_uhs2[i]->turnThruOff();
  }

  if (Usb.Init() == -1) {
    while (1)
      ;  //halt
  }      //if (Usb.Init() == -1...
  delay(200);

  xTaskCreatePinnedToCore(ReadCB,  //See FreeRTOS for more multitask info
                          "MIDI-READ",
                          3000,
                          NULL,
                          1,
                          NULL,
                          1);  //Core0 or Core1
                          
}


void loop() {
  Usb.Task();
  M5.update();
  if (M5.BtnA.wasReleased()) {
    bleClientMode = !bleClientMode;
    preferences.putBool("bleclientmode", bleClientMode);
    toggleLED();
    if (bleClientMode) {
      BLEmidiBle.endTransmission();
      BLEmidiBle.end();
      midiBleClient.begin(MIDI_CHANNEL_OMNI);
      midiBleClient.turnThruOff();
    } else {
      BLEmidiBleClient.endTransmission();
      BLEmidiBleClient.end();
      midiBle.begin(MIDI_CHANNEL_OMNI);
      midiBle.turnThruOff();
    }
    toggleLED();
    delay(200);
    toggleLED();
    delay(200);
    toggleLED();
    delay(200);
    toggleLED();
    delay(200);
    toggleLED();
  }
  if (bleClientMode) {
    if (midiBleClient.read()) {
      toggleLED();
      midi::MidiType t = midiBleClient.getType();
      midi::DataByte d1 = midiBleClient.getData1();
      midi::DataByte d2 = midiBleClient.getData2();
      midi::Channel c = midiBleClient.getChannel();
      send_uhs(t, d1, d2, c);
    }
  } else {
    if (midiBle.read()) {
      toggleLED();
      midi::MidiType t = midiBle.getType();
      midi::DataByte d1 = midiBle.getData1();
      midi::DataByte d2 = midiBle.getData2();
      midi::Channel c = midiBle.getChannel();
      send_uhs(t, d1, d2, c);
    }
  }
  for (int c = 0; c < MIDI_UHS2_DEVICE_COUNT; c++) {
    if (list_devices_uhs2[c]->read()) {
      toggleLED();
      midi::MidiType t = list_devices_uhs2[c]->getType();
      if (t != midi::SystemExclusive) {
        midi::DataByte d1 = list_devices_uhs2[c]->getData1();
        midi::DataByte d2 = list_devices_uhs2[c]->getData2();
        midi::Channel ch = list_devices_uhs2[c]->getChannel();
        send_ble(t, d1, d2, ch);
        send_uhs(t, d1, d2, ch, c);  // do not send to self, no passthrough
      } else {
        const byte* sysexArray = list_devices_uhs2[c]->getSysExArray();
        midi::DataByte d1 = list_devices_uhs2[c]->getData1();
        midi::DataByte d2 = list_devices_uhs2[c]->getData2();
        send_ble_sysex(d1, d2, sysexArray);
        send_uhs_sysex(d1, d2, sysexArray, c);  // do not send to self, no passthrough
      }
    }
  }
  //vTaskDelay(1 / portTICK_PERIOD_MS);  //Feed the watchdog of FreeRTOS.
}
/**
 * This function is called by xTaskCreatePinnedToCore() to perform a multitask execution.
*/

void ReadCB(void *parameter) {
  //  Serial.print("READ Task is started on core: ");
  //  Serial.println(xPortGetCoreID());
  for (;;) {  
    if (bleClientMode && BLEmidiBleClient.available() > 0) {
      BLEmidiBleClient.read();
    }
    vTaskDelay(1 / portTICK_PERIOD_MS);  //Feed the watchdog of FreeRTOS.
                                         //BLEmidiBle
    //Serial.println(uxTaskGetStackHighWaterMark(NULL)); //Only for debug. You can see the watermark of the free resources assigned by the xTaskCreatePinnedToCore() function.
  }
}
