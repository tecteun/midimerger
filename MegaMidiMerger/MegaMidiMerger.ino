#include <MIDI.h>
#include <UHS2-MIDI.h>
#include <usbhub.h>
//https://github.com/TheKikGen/USBMidiKliK
//send sysex F0 77 77 77 09 F7 to reset interface to serial mode, to flash




USB Usb;

// support one hub, four midi devices
USBHub Hub(&Usb);
UHS2MIDI_CREATE_INSTANCE(&Usb, 0, midiUsb);
UHS2MIDI_CREATE_INSTANCE(&Usb, 1, midiUsb1);
UHS2MIDI_CREATE_INSTANCE(&Usb, 2, midiUsb2);
UHS2MIDI_CREATE_INSTANCE(&Usb, 3, midiUsb3);
UHS2MIDI_CREATE_INSTANCE(&Usb, 4, midiUsb4);
UHS2MIDI_CREATE_INSTANCE(&Usb, 5, midiUsb5);
UHS2MIDI_CREATE_INSTANCE(&Usb, 6, midiUsb6);
UHS2MIDI_CREATE_INSTANCE(&Usb, 7, midiUsb7);
UHS2MIDI_CREATE_INSTANCE(&Usb, 8, midiUsb8);
UHS2MIDI_CREATE_INSTANCE(&Usb, 9, midiUsb9);
#define MIDI_UHS2_DEVICE_COUNT 10

MIDI_CREATE_INSTANCE(HardwareSerial, Serial, midiUsbMidiKlik);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, midiA);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, midiB);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, midiC);
#define MIDI_SERIAL_DEVICE_COUNT 4

midi::MidiInterface<midi::SerialMIDI<HardwareSerial>>* list_devices_serial[MIDI_SERIAL_DEVICE_COUNT] = {
  &midiUsbMidiKlik, &midiA, &midiB, &midiC
};

midi::MidiInterface<uhs2Midi::uhs2MidiTransport>* list_devices_uhs2[MIDI_UHS2_DEVICE_COUNT] = {
  &midiUsb, &midiUsb1, &midiUsb2, &midiUsb3, &midiUsb4, &midiUsb5, &midiUsb6, &midiUsb7, &midiUsb8, &midiUsb9
};

bool toggle = false;
void setup() {
  // Make Arduino transparent for serial communications from and to USB(client-hid)
  pinMode(0, INPUT);  // Arduino RX - ATMEGA8U2 TX
  pinMode(1, INPUT);  // Arduino TX - ATMEGA8U2 RX
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
    while (1);  //halt
  }      //if (Usb.Init() == -1...
  delay(200);
}

void loop() {
  Usb.Task();
  for (int c = 0; c < MIDI_SERIAL_DEVICE_COUNT; c++) {
    if (list_devices_serial[c]->read()) {
      digitalWrite(LED_BUILTIN, toggle = !toggle ? LOW : HIGH);
      midi::MidiType t = list_devices_serial[c]->getType();
      midi::DataByte d1 = list_devices_serial[c]->getData1();
      midi::DataByte d2 = list_devices_serial[c]->getData2();
      midi::Channel ch = list_devices_serial[c]->getChannel();
      for (int i = 0; i < MIDI_SERIAL_DEVICE_COUNT; i++) {
        if (c == i) {  // do not send to self, no passthrough
          list_devices_serial[i]->send(t, d1, d2, ch);
        }
      }
      for (int i = 0; i < MIDI_UHS2_DEVICE_COUNT; i++) {
        list_devices_uhs2[i]->send(t, d1, d2, ch);
      }
    }
  }
  for (int c = 0; c < MIDI_UHS2_DEVICE_COUNT; c++) {
    if (list_devices_uhs2[c]->read()) {
      digitalWrite(LED_BUILTIN, toggle = !toggle ? LOW : HIGH);
      midi::MidiType t = list_devices_uhs2[c]->getType();
      midi::DataByte d1 = list_devices_uhs2[c]->getData1();
      midi::DataByte d2 = list_devices_uhs2[c]->getData2();
      midi::Channel ch = list_devices_uhs2[c]->getChannel();
      for (int i = 0; i < MIDI_SERIAL_DEVICE_COUNT; i++) {
        list_devices_serial[i]->send(t, d1, d2, ch);
      }
      for (int i = 0; i < MIDI_UHS2_DEVICE_COUNT; i++) {
        if (c == i) {  // do not send to self, no passthrough
          list_devices_uhs2[i]->send(t, d1, d2, ch);
        }
      }
    }
  }
}
