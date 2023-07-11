
#include <MIDI.h>
#include <BLEMIDI_Transport.h>

#include <hardware/BLEMIDI_Client_ESP32.h>

#define BLEMIDI_CREATE_CLIENT_INSTANCE(DeviceName, Name) \
  BLEMIDI_NAMESPACE::BLEMIDI_Transport<BLEMIDI_NAMESPACE::BLEMIDI_Client_ESP32> BLE##Name(DeviceName); \
  MIDI_NAMESPACE::MidiInterface<BLEMIDI_NAMESPACE::BLEMIDI_Transport<BLEMIDI_NAMESPACE::BLEMIDI_Client_ESP32>, BLEMIDI_NAMESPACE::MySettings> Name((BLEMIDI_NAMESPACE::BLEMIDI_Transport<BLEMIDI_NAMESPACE::BLEMIDI_Client_ESP32> &)BLE##Name);

#include <hardware/BLEMIDI_ESP32_NimBLE.h>

void ReadCB(void *parameter);  //Continuos Read function (See FreeRTOS multitasks)
unsigned long t0 = millis();

BLEMIDI_CREATE_INSTANCE("mmmMegaMidiMergerBT", midiBle);
BLEMIDI_CREATE_CLIENT_INSTANCE("", midiBleClient)
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, midiA);
//MIDI_CREATE_INSTANCE(HardwareSerial, Serial, midiMonitor);
#define ONBOARD_LED 2
#define ONBOARD_BUTTON_LABELED_BOOT 0
#define MIDI_BAUDRATE 31250
#define RXD2 16
#define TXD2 17
#define RXD1 0
#define TXD1 4
bool bleClientMode = false;
bool toggle = false;
bool isConnected = false;
bool isClientConnected = false;


void IRAM_ATTR toggleLED() {
  bleClientMode = !bleClientMode;
  digitalWrite(ONBOARD_LED, toggle = !toggle ? HIGH : LOW);
}

void setup() {
  Serial.begin(115200);

  pinMode(ONBOARD_BUTTON_LABELED_BOOT, INPUT_PULLUP);
  attachInterrupt(ONBOARD_BUTTON_LABELED_BOOT, toggleLED, RISING);

  // Serial2 (GPIO 16 & 17) for hardware-midi in/out
  // only 2 is connected
  // midiB is RX
  // midiA is Tx
  Serial2.begin(MIDI_BAUDRATE, SERIAL_8N1, RXD2, TXD2);
  //pinMode(RXD2, INPUT_PULLUP);
  pinMode(ONBOARD_LED, OUTPUT);
  // Initiate MIDI communications, listen to all channels
  midiA.begin(MIDI_CHANNEL_OMNI);
  //midiMonitor.begin(MIDI_CHANNEL_OMNI);
  midiBle.begin(MIDI_CHANNEL_OMNI);
  midiBleClient.begin(MIDI_CHANNEL_OMNI);
  midiA.turnThruOff();
  midiBle.turnThruOff();
  //midiMonitor.turnThruOff();
  BLEmidiBle.setHandleConnected([]() {
    isConnected = true;
    digitalWrite(LED_BUILTIN, HIGH);
  });

  BLEmidiBle.setHandleDisconnected([]() {
    isConnected = false;
    digitalWrite(LED_BUILTIN, LOW);
    //BLEmidiBle.end();
    //BLEmidiBle.begin();
  });



  /*
  xTaskCreatePinnedToCore(ReadCB,  //See FreeRTOS for more multitask info
                          "MIDI-READ",
                          3000,
                          NULL,
                          1,
                          NULL,
                          1);  //Core0 or Core1
           */
}


void loop() {
  if(bleClientMode){
    while (midiBleClient.read()) {
      digitalWrite(ONBOARD_LED, toggle = !toggle ? HIGH : LOW);
      midi::MidiType t = midiBleClient.getType();
      midi::DataByte d1 = midiBleClient.getData1();
      midi::DataByte d2 = midiBleClient.getData2();
      midi::Channel c = midiBleClient.getChannel();
      midiA.send(t, d1, d2, c);
    }
  }else{
    while (midiBle.read()) {
      digitalWrite(ONBOARD_LED, toggle = !toggle ? HIGH : LOW);
      midi::MidiType t = midiBle.getType();
      midi::DataByte d1 = midiBle.getData1();
      midi::DataByte d2 = midiBle.getData2();
      midi::Channel c = midiBle.getChannel();
      midiA.send(t, d1, d2, c);
    }
  }
  while (midiA.read()) {
    digitalWrite(ONBOARD_LED, toggle = !toggle ? HIGH : LOW);
    midi::MidiType t = midiA.getType();
    midi::DataByte d1 = midiA.getData1();
    midi::DataByte d2 = midiA.getData2();
    midi::Channel c = midiA.getChannel();
    //if(t != midi::SystemExclusive){

    if (isConnected) {
      midiBle.send(t, d1, d2, c);
    }
    midiBleClient.send(t, d1, d2, c);

    //}
  }
}
/**
 * This function is called by xTaskCreatePinnedToCore() to perform a multitask execution.
*/

void ReadCB(void *parameter) {
  //  Serial.print("READ Task is started on core: ");
  //  Serial.println(xPortGetCoreID());
  for (;;) {

    vTaskDelay(1 / portTICK_PERIOD_MS);  //Feed the watchdog of FreeRTOS.
                                         //BLEmidiBle
    //Serial.println(uxTaskGetStackHighWaterMark(NULL)); //Only for debug. You can see the watermark of the free resources assigned by the xTaskCreatePinnedToCore() function.
  }
}
