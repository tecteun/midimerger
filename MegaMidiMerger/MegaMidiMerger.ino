#include <MIDI.h>
#include <UHS2-MIDI.h>
#include <usbhub.h>
//https://github.com/TheKikGen/USBMidiKliK
//send sysex F0 77 77 77 09 F7 to reset interface to serial mode, to flash
// @see https://github.com/diyelectromusic/sdemp/blob/HEAD/src/SDEMP/ArduinoMultiMIDIMerge2/ArduinoMultiMIDIMerge2.ino
USB Usb;
// support one hub, four midi devices
USBHub Hub(&Usb);
UHS2MIDI_CREATE_INSTANCE(&Usb, 0, midiUsb);
UHS2MIDI_CREATE_INSTANCE(&Usb, 0, midiUsb1);
UHS2MIDI_CREATE_INSTANCE(&Usb, 0, midiUsb2);
UHS2MIDI_CREATE_INSTANCE(&Usb, 0, midiUsb3);
UHS2MIDI_CREATE_INSTANCE(&Usb, 1, midiUsb4);
UHS2MIDI_CREATE_INSTANCE(&Usb, 1, midiUsb5);
UHS2MIDI_CREATE_INSTANCE(&Usb, 1, midiUsb6);
UHS2MIDI_CREATE_INSTANCE(&Usb, 1, midiUsb7);
UHS2MIDI_CREATE_INSTANCE(&Usb, 2, midiUsb8);
UHS2MIDI_CREATE_INSTANCE(&Usb, 2, midiUsb9);
UHS2MIDI_CREATE_INSTANCE(&Usb, 2, midiUsb10);
UHS2MIDI_CREATE_INSTANCE(&Usb, 2, midiUsb11);

#define MIDI_UHS2_DEVICE_COUNT 12

MIDI_CREATE_INSTANCE(HardwareSerial, Serial, midiUsbMidiKlik);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, midiA);
//disable when not connected, introduces noise
//MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, midiB);
//MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, midiC);
#define MIDI_SERIAL_DEVICE_COUNT 2

midi::MidiInterface<midi::SerialMIDI<HardwareSerial>>* list_devices_serial[MIDI_SERIAL_DEVICE_COUNT] = {
  &midiUsbMidiKlik, &midiA //, &midiB, &midiC
};

midi::MidiInterface<uhs2Midi::uhs2MidiTransport>* list_devices_uhs2[MIDI_UHS2_DEVICE_COUNT] = {
  &midiUsb, &midiUsb1, &midiUsb2, &midiUsb3, &midiUsb4, &midiUsb5, &midiUsb6, &midiUsb7, &midiUsb8, &midiUsb9, &midiUsb10, &midiUsb11
};

#define EURORACK_TRIGGER_INTERRUPT_PIN 3

unsigned long previousMillis = 0;  // will store last time LED was updated

bool toggle = false;

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
void send_serial(midi::MidiType t, midi::DataByte d1, midi::DataByte d2, midi::Channel ch, int exclude = -1) {
  for (int i = 0; i < MIDI_SERIAL_DEVICE_COUNT; i++) {
    if (exclude != i) {  // do not send to self, no passthrough
      list_devices_serial[i]->send(t, d1, d2, ch);
    }
  }
}

void send_serial_sysex(midi::DataByte d1, midi::DataByte d2, const byte* sysexArray, int exclude = -1) {
  for (int i = 0; i < MIDI_SERIAL_DEVICE_COUNT; i++) {
    if (exclude != i) {  // do not send to self, no passthrough
      int mSxLen = d1 + 256 * d2;
      list_devices_serial[i]->sendSysEx(mSxLen, sysexArray, true);
    }
  }
}

static unsigned long last_interrupt_time = 0;
void eurorack_trigger() {
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > 200) 
  {
    send_serial(midi::NoteOn, random(0, 255), 127, 1);  // Send a Note (pitch 42, velo 127 on channel 1)
    send_uhs(midi::NoteOn, random(0, 255), 127, 1);     // Send a Note (pitch 42, velo 127 on channel 1)
    //send_serial(midi::NoteOff, 42, 0, 1);   // Send a NoteOff (pitch 42, velo 0 on channel 1)
    //send_uhs(midi::NoteOff, 42, 0, 1);      // Send a NoteOff (pitch 42, velo 0 on channel 1)
    flashLed();
  }
  last_interrupt_time = interrupt_time;
  
}

void setup() {
  // random
  randomSeed(analogRead(0));
  // interrupts
  pinMode(EURORACK_TRIGGER_INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(EURORACK_TRIGGER_INTERRUPT_PIN), eurorack_trigger, RISING);  //CHANGE LOW RISING FALLING

  // Make Arduino transparent for serial communications from and to USB(client-hid)
  pinMode(0, INPUT);  // Arduino RX - ATMEGA8U2 TX
  pinMode(1, INPUT);  // Arduino TX - ATMEGA8U2 RX

  pinMode(18, INPUT);         // Arduino Mega TX1 
  pinMode(19, INPUT_PULLUP);  // Arduino Mega RX1
  // prevent noise, even when not connected
  pinMode(16, INPUT);         // Arduino Mega TX2 
  pinMode(17, INPUT_PULLUP);  // Arduino Mega RX2
  
  pinMode(14, INPUT);         // Arduino Mega TX2 
  pinMode(15, INPUT_PULLUP);  // Arduino Mega RX2
  
  Serial.begin(31250);
  pinMode(LED_BUILTIN, OUTPUT);
  // Initiate MIDI communications, listen to all channels

  for (int i = 0; i < MIDI_SERIAL_DEVICE_COUNT; i++) {
    list_devices_serial[i]->begin(MIDI_CHANNEL_OMNI);
    list_devices_serial[i]->turnThruOff();
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
}
void flashLed() {
  toggle = true;
}
void loop() {
  //Usb.vbusPower(vbus_on);
  //using a fixed library branch: https://github.com/tmk/USB_Host_Shield_2.0/tree/upstream_fix_busprobe
  //Usb.busprobe();
  Usb.Task();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 10) {
    previousMillis = currentMillis;
    digitalWrite(LED_BUILTIN, toggle ? HIGH : LOW);
    if (toggle) {
      toggle = false;
    }
  }

  for (int c = 0; c < MIDI_SERIAL_DEVICE_COUNT; c++) {
    if (list_devices_serial[c]->read()) {
      flashLed();
      midi::MidiType t = list_devices_serial[c]->getType();
      if (t != midi::SystemExclusive) {
        midi::DataByte d1 = list_devices_serial[c]->getData1();
        midi::DataByte d2 = list_devices_serial[c]->getData2();
        midi::Channel ch = list_devices_serial[c]->getChannel();
        send_serial(t, d1, d2, ch, c);  // do not send to self, no passthrough
        send_uhs(t, d1, d2, ch);
      } else {
        byte* sysexArray = list_devices_serial[c]->getSysExArray();
        midi::DataByte d1 = list_devices_serial[c]->getData1();
        midi::DataByte d2 = list_devices_serial[c]->getData2();
        send_serial_sysex(d1, d2, sysexArray, c);  // do not send to self, no passthrough
        send_uhs_sysex(d1, d2, sysexArray);
      }
    }
  }
  for (int c = 0; c < MIDI_UHS2_DEVICE_COUNT; c++) {
    if (list_devices_uhs2[c]->read()) {
      flashLed();
      midi::MidiType t = list_devices_uhs2[c]->getType();
      if (t != midi::SystemExclusive) {
        midi::DataByte d1 = list_devices_uhs2[c]->getData1();
        midi::DataByte d2 = list_devices_uhs2[c]->getData2();
        midi::Channel ch = list_devices_uhs2[c]->getChannel();
        send_serial(t, d1, d2, ch);
        send_uhs(t, d1, d2, ch, c);  // do not send to self, no passthrough
      } else {
        byte* sysexArray = list_devices_uhs2[c]->getSysExArray();
        midi::DataByte d1 = list_devices_uhs2[c]->getData1();
        midi::DataByte d2 = list_devices_uhs2[c]->getData2();
        send_serial_sysex(d1, d2, sysexArray);
        send_uhs_sysex(d1, d2, sysexArray, c);  // do not send to self, no passthrough
      }
    }
  }
}
