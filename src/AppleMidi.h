#pragma once

// https://developer.apple.com/library/archive/documentation/Audio/Conceptual/MIDINetworkDriverProtocol/MIDI/MIDI.html

#include <MIDI.h>

#include "AppleMidi_Settings.h"
#include "AppleMidi_Session.h"

BEGIN_APPLEMIDI_NAMESPACE

struct AppleMIDIDefaultSettings : public DefaultSettings
{
};

END_APPLEMIDI_NAMESPACE

BEGIN_APPLEMIDI_NAMESPACE

template<class UdpClass>
class AppleMidiTransport 
{
public:
	AppleMidiTransport()
	{
	};

protected:
	inline void begin(Channel inChannel = 1)
	{
	}

	inline bool beginTransmission()
	{
		// setup
		return true;
	};

	inline void write(byte byte)
	{
	};

	inline void endTransmission()
	{
		// send it thru AppleMIDI here
		// in welke session/midi device sturen we de noten???
	};

	inline byte read()
	{
		return session.midiBuffer.read();
	};

	inline unsigned available()
	{
		// entry point for parsing content
		session.run();

		return session.midiBuffer.getLength();
	};

	friend class MidiInterface<AppleMidiTransport<UdpClass>>;

public:
	Session<UdpClass>* begin(const char* name, const uint16_t port = CONTROL_PORT)
	{
		session.begin(name, port);
		return &session;
	}

private:
	// only 1 session can be attached
	Session<UdpClass>	session;
};

#define APPLEMIDI_CREATE_INSTANCE(Type, midiName, appleMidiName, appleMidiSessionName) \
	typedef APPLEMIDI_NAMESPACE::AppleMidiTransport<Type> __amt;   \
	__amt appleMidiName;                                           \
	midi::MidiInterface<__amt> midiName((__amt&)appleMidiName);

#define APPLEMIDI_CREATE_DEFAULT_INSTANCE()               \
	APPLEMIDI_CREATE_INSTANCE(EthernetUDP, MIDI, AppleMIDI, AppleMIDISession);

END_APPLEMIDI_NAMESPACE

#include "AppleMidi.hpp"