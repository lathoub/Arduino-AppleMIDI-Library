#pragma once

// https://developer.apple.com/library/archive/documentation/Audio/Conceptual/MIDINetworkDriverProtocol/MIDI/MIDI.html

#include <MIDI.h>
#include <midi_RingBuffer.h>
using namespace MIDI_NAMESPACE;

#include "AppleMidi_Defs.h"
#include "AppleMidi_Settings.h"
#include "AppleMidi_SessionManager.h"
#include "AppleMidi_Parser.h"
#include "rtpMidi_Parser.h"

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
		// attach the parsers
		controlParsers[0] = &AppleMIDIParser<UdpClass>::Parser;

		dataParsers[0] = &rtpMIDIParser<UdpClass>::Parser;
		dataParsers[1] = &AppleMIDIParser<UdpClass>::Parser;
	};

protected:
	inline void begin(Channel inChannel = 1)
	{
		Serial.println("AppleMIDI begin");

		_controlPort.begin(_port);	// UDP socket for control messages
		_dataPort.begin(_port + 1);	// UDP socket for data messages
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
	inline void begin(const char* sessionName, const uint16_t port = CONTROL_PORT)
	{
		_sessionManager.setSessionName(sessionName);
		_port = port;

		// The MIDI protocol will call the underlying begin, that
		// will begin the sockets
	}

protected:
	inline void run()
	{
		byte packetBuffer[UDP_TX_PACKET_MAX_SIZE];

		auto packetSize = _controlPort.parsePacket();
		if (packetSize > 0) {
			while (packetSize > 0)
			{
				auto bytesRead = _controlPort.read(packetBuffer, sizeof(packetBuffer));
				_controlBuffer.write(packetBuffer, bytesRead);

				packetSize -= bytesRead;
			}

			controlParsers[0](_controlBuffer, this, amPortType::Control);
		}

		packetSize = _dataPort.parsePacket();
		if (packetSize > 0) {
			while (packetSize > 0)
			{
				auto bytesRead = _dataPort.read(packetBuffer, sizeof(packetBuffer));
				_dataBuffer.write(packetBuffer, bytesRead);

				packetSize -= bytesRead;
			}

			dataParsers[0](_dataBuffer, this, amPortType::Data);
			dataParsers[1](_dataBuffer, this, amPortType::Data);
		}

		// resend invitations
		ManageInvites();

		// do syncronization here
		ManageTiming();
	}

public:
	inline void setHandleConnected(void(*fptr)(uint32_t, const char*)) { _connectedCallback = fptr; }
	inline void setHandleDisconnected(void(*fptr)(uint32_t)) { _disconnectedCallback = fptr; }

protected:
	inline void ManageInvites();
	inline void ManageTiming();

	inline void ReceivedInvitation(const AppleMIDI_Invitation_t&, const amPortType&);

	friend class AppleMIDIParser<UdpClass>;
	friend class rtpMIDIParser<UdpClass>;

private:
	inline void ReceivedControlInvitation(const AppleMIDI_Invitation_t&);
	inline void ReceivedDataInvitation(const AppleMIDI_Invitation_t&);

private:
	UdpClass _dataPort;
	UdpClass _controlPort;

	uint16_t _port = 0;

	typedef int(*FPPARSER)(midi::RingBuffer<byte, BUFFER_MAX_SIZE>&, AppleMidiTransport<UdpClass>*, const amPortType&);

	FPPARSER controlParsers[1];
	FPPARSER dataParsers[2];

	midi::RingBuffer<byte, BUFFER_MAX_SIZE> _dataBuffer;
	midi::RingBuffer<byte, BUFFER_MAX_SIZE> _controlBuffer;

	SessionManager _sessionManager;

private:
	void(*_connectedCallback)(uint32_t, const char*);
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