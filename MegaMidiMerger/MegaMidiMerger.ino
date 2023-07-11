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

MIDI_CREATE_INSTANCE(HardwareSerial, Serial,     midiUsbMidiKlik);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1,    midiA);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2,    midiB);
bool toggle = false;
void setup()
{
  // Make Arduino transparent for serial communications from and to USB(client-hid)
  pinMode(0,INPUT); // Arduino RX - ATMEGA8U2 TX
  pinMode(1,INPUT); // Arduino TX - ATMEGA8U2 RX
  Serial.begin(31250);
  
  pinMode(LED_BUILTIN, OUTPUT);
  // Initiate MIDI communications, listen to all channels

  midiUsbMidiKlik.begin(MIDI_CHANNEL_OMNI);
  midiA.begin(MIDI_CHANNEL_OMNI);
  midiB.begin(MIDI_CHANNEL_OMNI);  
  midiUsb.begin(MIDI_CHANNEL_OMNI);
  midiUsb1.begin(MIDI_CHANNEL_OMNI);
  midiUsb2.begin(MIDI_CHANNEL_OMNI);
  midiUsb3.begin(MIDI_CHANNEL_OMNI);
  
  midiUsbMidiKlik.turnThruOff();
  midiA.turnThruOff();
  midiB.turnThruOff();
  midiUsb.turnThruOff();
  midiUsb1.turnThruOff();
  midiUsb2.turnThruOff();
  midiUsb3.turnThruOff();

  if (Usb.Init() == -1) {
    while (1); //halt
  }//if (Usb.Init() == -1...
  delay( 200 );
}

void loop()
{
  Usb.Task();
  
  if (midiUsbMidiKlik.read())
  {
    digitalWrite(LED_BUILTIN, toggle = !toggle ? LOW : HIGH);
    midi::MidiType t = midiUsbMidiKlik.getType();
    midi::DataByte d1 = midiUsbMidiKlik.getData1();
    midi::DataByte d2 = midiUsbMidiKlik.getData2();
    midi::Channel  c = midiUsbMidiKlik.getChannel();

    midiA.send(t, d1, d2, c);
    midiB.send(t, d1, d2, c);
    midiUsb.send(t, d1, d2, c);
    midiUsb1.send(t, d1, d2, c);
    midiUsb2.send(t, d1, d2, c);
    midiUsb3.send(t, d1, d2, c);
  }
  
  if (midiUsb.read())
  {
    digitalWrite(LED_BUILTIN, toggle = !toggle ? LOW : HIGH);
    midi::MidiType t = midiUsb.getType();
    midi::DataByte d1 = midiUsb.getData1();
    midi::DataByte d2 = midiUsb.getData2();
    midi::Channel  c = midiUsb.getChannel();

    midiUsbMidiKlik.send(t, d1, d2, c);
    midiA.send(t, d1, d2, c);
    midiB.send(t, d1, d2, c);
    midiUsb1.send(t, d1, d2, c);
    midiUsb2.send(t, d1, d2, c);
    midiUsb3.send(t, d1, d2, c);
  }

  if (midiUsb1.read())
  {
    digitalWrite(LED_BUILTIN, toggle = !toggle ? LOW : HIGH);
    midi::MidiType t = midiUsb1.getType();
    midi::DataByte d1 = midiUsb1.getData1();
    midi::DataByte d2 = midiUsb1.getData2();
    midi::Channel  c = midiUsb1.getChannel();

    midiUsbMidiKlik.send(t, d1, d2, c);
    midiA.send(t, d1, d2, c);
    midiB.send(t, d1, d2, c);
    midiUsb.send(t, d1, d2, c);
    midiUsb2.send(t, d1, d2, c);
    midiUsb3.send(t, d1, d2, c);
  }

  if (midiUsb2.read())
  {
    digitalWrite(LED_BUILTIN, toggle = !toggle ? LOW : HIGH);
    midi::MidiType t = midiUsb2.getType();
    midi::DataByte d1 = midiUsb2.getData1();
    midi::DataByte d2 = midiUsb2.getData2();
    midi::Channel  c = midiUsb2.getChannel();

    midiUsbMidiKlik.send(t, d1, d2, c);
    midiA.send(t, d1, d2, c);
    midiB.send(t, d1, d2, c);
    midiUsb.send(t, d1, d2, c);
    midiUsb1.send(t, d1, d2, c);
    midiUsb3.send(t, d1, d2, c);
  }

  if (midiUsb3.read())
  {
    digitalWrite(LED_BUILTIN, toggle = !toggle ? LOW : HIGH);
    midi::MidiType t = midiUsb3.getType();
    midi::DataByte d1 = midiUsb3.getData1();
    midi::DataByte d2 = midiUsb3.getData2();
    midi::Channel  c = midiUsb3.getChannel();

    midiUsbMidiKlik.send(t, d1, d2, c);
    midiA.send(t, d1, d2, c);
    midiB.send(t, d1, d2, c);
    midiUsb.send(t, d1, d2, c);
    midiUsb1.send(t, d1, d2, c);
    midiUsb2.send(t, d1, d2, c);
  }
  
  if (midiA.read())
  {
    digitalWrite(LED_BUILTIN, toggle = !toggle ? LOW : HIGH);
    midi::MidiType t = midiA.getType();
    midi::DataByte d1 = midiA.getData1();
    midi::DataByte d2 = midiA.getData2();
    midi::Channel  c = midiA.getChannel();

    midiUsbMidiKlik.send(t, d1, d2, c);
    midiB.send(t, d1, d2, c);
    midiUsb.send(t, d1, d2, c);
  }
  
  if (midiB.read())
  {
    digitalWrite(LED_BUILTIN, toggle = !toggle ? LOW : HIGH);
    midi::MidiType t = midiB.getType();
    midi::DataByte d1 = midiB.getData1();
    midi::DataByte d2 = midiB.getData2();
    midi::Channel  c = midiB.getChannel();

    midiUsbMidiKlik.send(t, d1, d2, c);
    midiA.send(t, d1, d2, c);
    midiUsb.send(t, d1, d2, c);
  }
  
}
