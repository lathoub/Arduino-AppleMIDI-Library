#pragma once

// https://developer.apple.com/library/archive/documentation/Audio/Conceptual/MIDINetworkDriverProtocol/MIDI/MIDI.html

#include <MIDI.h>
#include <midi_RingBuffer.h>
using namespace MIDI_NAMESPACE;

#include "AppleMidi_Defs.h"
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
		return true;
	};

	inline void write(byte byte)
	{
	};

	inline void endTransmission()
	{
	};

	inline byte read()
	{
		return 0x00;
	};

	inline unsigned available()
	{
		// entry point for parsing content
		run();

		return 0;
	};

	friend class MidiInterface<AppleMidiTransport<UdpClass>>;

public:
	Session<UdpClass>* addSession(const char* name, const uint16_t port = CONTROL_PORT)
	{
		auto session = &_sessions[0];
		session->begin(name, port);

		return session;
	}

protected:
	inline void run()
	{
		for (int i = 0; i < MAX_SESSIONS; i++)
			_sessions[i].run();
	}

private:
	Session<UdpClass>	_sessions[MAX_SESSIONS];
};

#define APPLEMIDI_CREATE_INSTANCE(Type, Name)             \
	typedef APPLEMIDI_NAMESPACE::AppleMidiTransport<Type> __st;   \
	__st AppleMIDI;                                           \
	midi::MidiInterface<__st> Name((__st&)AppleMIDI); \
	typedef APPLEMIDI_NAMESPACE::Session<Type> AppleMIDI_Session;

#define APPLEMIDI_CREATE_DEFAULT_INSTANCE()               \
	APPLEMIDI_CREATE_INSTANCE(EthernetUDP, MIDI);

END_APPLEMIDI_NAMESPACE

#include "AppleMidi.hpp"