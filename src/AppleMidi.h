#pragma once

// https://developer.apple.com/library/archive/documentation/Audio/Conceptual/MIDINetworkDriverProtocol/MIDI/MIDI.html

#include <MIDI.h>
using namespace MIDI_NAMESPACE;

#include <AppleMidi_Defs.h>
#include <AppleMidi_Settings.h>

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

public:
	inline void begin(Channel inChannel = 1)
	{
	}

public:
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
		// entry point for parsing content
		run();

		return 0x00;
	};

	inline unsigned available()
	{
		return 0;
	};

public:
	inline void begin(const char* sessionName, const Channel inChannel = 1)
	{
		begin(inChannel);

		strncpy(_sessionName, sessionName, SESSION_NAME_MAX_LEN);

		// open UDP socket for control messages
		_controlPort.begin(_port);
		// open UDP socket for rtp messages
		_dataPort.begin(_port + 1);

		// TODO setup parsers
	}

protected:
	inline void run()
	{
		// Process one control packet, if available
		int packetSize = _controlPort.parsePacket();
		if (packetSize) {
			packetSize = _controlPort.read(_packetBuffer, sizeof(_packetBuffer));
			//_controlPortDissector.addPacket(_packetBuffer, packetSize);
		}

		// Process one control packet, if available
		packetSize = _dataPort.parsePacket();
		if (packetSize) {
			packetSize = _dataPort.read(_packetBuffer, sizeof(_packetBuffer));
			//_dataPortDissector.addPacket(_packetBuffer, packetSize);
		}

		// resend invitations
		ManageInvites();

		// do syncronization here
		ManageTiming();
	}

public:
	inline void setHandleConnected(void(*fptr)(uint32_t, char*)) { _connectedCallback = fptr; }
	inline void setHandleDisconnected(void(*fptr)(uint32_t)) { _disconnectedCallback = fptr; }

public:
	// Session management
	inline void ManageInvites();
	inline void ManageTiming();

private:
	UdpClass _dataPort;
	UdpClass _controlPort;

	uint16_t _port = CONTROL_PORT;

	char _sessionName[SESSION_NAME_MAX_LEN + 1];

	byte _packetBuffer[PACKET_MAX_SIZE];

	// SSRC, Synchronization source.
	// (RFC 1889) The source of a stream of RTP packets, identified by a 32-bit numeric SSRC identifier
	// carried in the RTP header so as not to be dependent upon the network address. All packets from a
	// synchronization source form part of the same timing and sequence number space, so a receiver groups
	// packets by synchronization source for playback. Examples of synchronization sources include the
	// sender of a stream of packets derived from a signal source such as a microphone or a camera, or an
	// RTP mixer. A synchronization source may change its data format, e.g., audio encoding, over time.
	// The SSRC identifier is a randomly chosen value meant to be globally unique within a particular RTP
	// session. A participant need not use the same SSRC identifier for all the RTP sessions in a
	// multimedia session; the binding of the SSRC identifiers is provided through RTCP. If a participant
	// generates multiple streams in one RTP session, for example from separate video cameras, each must
	// be identified as a different SSRC.
	uint32_t _ssrc = 0;

private:
	void(*_connectedCallback)(uint32_t, char*);
	void(*_disconnectedCallback)(uint32_t);
};

#define APPLEMIDI_CREATE_INSTANCE(Type, Name)             \
	typedef APPLEMIDI_NAMESPACE::AppleMidiTransport<Type> __st;   \
	__st AppleMIDI;                                           \
	midi::MidiInterface<__st> Name((__st&)AppleMIDI);

#define APPLEMIDI_CREATE_DEFAULT_INSTANCE()               \
	APPLEMIDI_CREATE_INSTANCE(EthernetUDP, MIDI);

END_APPLEMIDI_NAMESPACE

#include "AppleMidi.hpp"