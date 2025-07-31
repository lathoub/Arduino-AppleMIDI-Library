#ifndef MIDIHELPERS_h
#define MIDIHELPERS_h

#define USE_EXT_CALLBACKS     // as from example => required for MIDI callbacks
#include <AppleMIDI.h>         //https://github.com/lathoub/Arduino-AppleMIDI-Library

using namespace APPLEMIDI_NAMESPACE;

/*
class Utilities for creating a generic pointer to AppleMIDISession and MidiInterface
from a WiFiUDP or EThernetUDP (template) and be able to manipulate it as a generic instance
and manipulating the pointer generically
*/

class MidiClient {
public:
  virtual void read() = 0;
  virtual ~MidiClient() {}
  virtual void setHandleConnected(void (*fptr)(const ssrc_t &, const char *))= 0;
  virtual void setHandleDisconnected(void (*fptr)(const ssrc_t &)) = 0;
  virtual void setHandleException(void (*fptr)(const ssrc_t &, const Exception &, const int32_t value))=0;
  virtual const char *getName() = 0;
  virtual const uint16_t getPort() = 0;
  virtual void begin() = 0;
  // all methods you need to be wrapped below
  virtual void sendNoteOn(byte note, byte velocity, byte channel) = 0;
  virtual void sendNoteOff(byte note, byte velocity, byte channel) = 0;  
  // etc...

};

template <typename UdpType>
class AppleMidiWithInterfaceWrapper : public MidiClient {
public:
  AppleMIDISession<UdpType>* session;
  MidiInterface<AppleMIDISession<UdpType>, AppleMIDISettings>* midi;

  AppleMidiWithInterfaceWrapper<UdpType>(const char* sessionName, uint16_t port) {
    session = new AppleMIDISession<UdpType>(sessionName, port);
    midi = new MidiInterface<AppleMIDISession<UdpType>, AppleMIDISettings>(*session);   
  }

  virtual void begin(){
    session->begin();    
  }

  virtual const char *getName(){
    return session->getName();
  }

  virtual const uint16_t getPort(){
    return session->getPort();
  }

  virtual void setHandleConnected(void (*fptr)(const ssrc_t &, const char *)){
    session->setHandleConnected(fptr);
  }

  virtual void setHandleDisconnected(void (*fptr)(const ssrc_t &)){
    session->setHandleDisconnected(fptr);
  }

  virtual void setHandleException(void (*fptr)(const ssrc_t &, const Exception &, const int32_t value)){
    session->setHandleException(fptr);
  }


  void read() override {
    midi->read();
  }

  void sendNoteOn(byte note, byte velocity, byte channel) override {
    midi->sendNoteOn(note, velocity, channel);
  }

  void sendNoteOff(byte note, byte velocity, byte channel) override {
    midi->sendNoteOff(note, velocity, channel);
  }


  ~AppleMidiWithInterfaceWrapper() {
    delete midi;
    delete session;
  }
};

#endif