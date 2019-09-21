#include "AppleMidi_Defs.h"

#include "AppleMidi_Parser.h"
#include "rtpMidi_Parser.h"

#include "rtpMidi_Clock.h"

BEGIN_APPLEMIDI_NAMESPACE

template<class UdpClass>
class Participant
{
public:
	char			name[SESSION_NAME_MAX_LEN + 1];
	uint32_t		ssrc;

	ParticipantMode initiator;

	IPAddress		ip;
	uint16_t		port; // controlPort, and dataPort = controlPort + 1
public:
	Participant()
	{
		ssrc = 0;
		port = 0;
		initiator = ParticipantMode::Undefined;
	}

	void run()
	{
	}
};

template<class UdpClass>
class Session
{
	UdpClass		controlPort;
	UdpClass		dataPort;

	midi::RingBuffer<byte, BUFFER_MAX_SIZE> controlBuffer;
	midi::RingBuffer<byte, BUFFER_MAX_SIZE> dataBuffer;

	rtpMidi_Clock _rtpMidiClock;

private:
	typedef int(*FPPARSER)(midi::RingBuffer<byte, BUFFER_MAX_SIZE>&, Session<UdpClass>*, const amPortType&);

	FPPARSER controlParsers[1];
	FPPARSER dataParsers[2];

	void(*_connectedCallback)(uint32_t, const char*);
	void(*_disconnectedCallback)(uint32_t);

	friend class AppleMIDIParser<UdpClass>;
	friend class rtpMIDIParser<UdpClass>;

public:
	Participant<UdpClass> participants[MAX_PARTICIPANTS];

	bool			enabled = false;

	uint32_t		ssrc = 0;

	char			localName[SESSION_NAME_MAX_LEN + 1];
	char			bonjourName[SESSION_NAME_MAX_LEN + 1];

	uint16_t		port; // controlPort, and dataPort = controlPort + 1

	inline void setHandleConnected(void(*fptr)(uint32_t, const char*)) { _connectedCallback = fptr; }
	inline void setHandleDisconnected(void(*fptr)(uint32_t)) { _disconnectedCallback = fptr; }

public:
	Session()
	{
		enabled = false;

		ssrc = 0;
		port = 0;
		localName[0] = '\0';
		bonjourName[0] = '\0';

		for (int i = 0; i < MAX_PARTICIPANTS; i++)
			participants[0].ssrc = 0;

		// attach the parsers
		controlParsers[0] = &AppleMIDIParser<UdpClass>::Parser;

		dataParsers[0] = &rtpMIDIParser<UdpClass>::Parser;
		dataParsers[1] = &AppleMIDIParser<UdpClass>::Parser;
	}

	void begin(const char* name, const uint16_t port)
	{
		this->ssrc = random(1, INT32_MAX);
		this->port = port;
		strncpy(this->localName, name, SESSION_NAME_MAX_LEN);
		strncpy(this->bonjourName, name, SESSION_NAME_MAX_LEN);

		controlPort.begin(this->port);	// UDP socket for control messages
		dataPort.begin(this->port + 1);	// UDP socket for data messages

		uint32_t initialTimestamp = 0;
		_rtpMidiClock.Init(initialTimestamp, MIDI_SAMPLING_RATE_DEFAULT);

		enabled = true;
	}

	void run()
	{
		// sessions are by default not enabled, a call to .begin() 
		// enables a session.
		if (!enabled) 
			return;

		byte packetBuffer[UDP_TX_PACKET_MAX_SIZE];

		auto packetSize = controlPort.parsePacket();
		if (packetSize > 0) {
			while (packetSize > 0)
			{
				auto bytesRead = controlPort.read(packetBuffer, sizeof(packetBuffer));
				controlBuffer.write(packetBuffer, bytesRead);

				packetSize -= bytesRead;
			}
			controlParsers[0](controlBuffer, this, amPortType::Control);
			// TODO
		}

		packetSize = dataPort.parsePacket();
		if (packetSize > 0) {
			while (packetSize > 0)
			{
				auto bytesRead = dataPort.read(packetBuffer, sizeof(packetBuffer));
				dataBuffer.write(packetBuffer, bytesRead);

				packetSize -= bytesRead;
			}
			dataParsers[0](dataBuffer, this, amPortType::Data);
			dataParsers[1](dataBuffer, this, amPortType::Data);
			// TODO
		}

		ManagePendingInvites();
		ManageTiming();
	}

	void stop()
	{
		enabled = false;

		// release resources held by the sockets
		controlPort.stop();
		dataPort.stop();
	}

	void receivedInvitation(AppleMIDI_Invitation& invitation, const amPortType& portType)
	{
		//Serial.println("receivedInvitation");

		if (portType == amPortType::Control)
			receivedControlInvitation(invitation);
		else
			receivedDataInvitation(invitation);
	}

	void receivedControlInvitation(AppleMIDI_Invitation& invitation)
	{
		//Serial.println("ReceivedControlInvitation");

		//Serial.print("initiatorToken: 0x");
		//Serial.print(invitation.initiatorToken, HEX);
		//Serial.print(", senderSSRC: 0x");
		//Serial.print(invitation.ssrc, HEX);
		//Serial.print(", sessionName: ");
		//Serial.println(invitation.sessionName);

		// Do we know this ssrc already?
		// In case initiator reconnects (after a crash of some sort)
		auto participant = getParticipant(invitation.ssrc);
		if (NULL == participant)
		{
			// No, not existing; must be a new initiator
			// Find a free slot to remember this participant in
			participant = createParticipant();
			if (NULL == participant)
			{
				//Serial.println("No free slots");
		//		// no free slots, we cant accept invite
		//		//AppleMIDI_InvitationRejected invitationRejected(invitation.ssrc, invitation.initiatorToken, invitation.sessionName);
		//		//write(_controlPort, invitationRejected, _controlPort.remoteIP(), _controlPort.remotePort());
				return;
			}

			participant->ssrc = invitation.ssrc;
			strncpy(participant->name, invitation.sessionName, SESSION_NAME_MAX_LEN);
			participant->ip = controlPort.remoteIP();
			participant->port = controlPort.remotePort();
			participant->initiator = ParticipantMode::Undefined;

		}
		else
		{
			// Participant exists
		}

		// dada
		// Invitation Accepted
		auto success = controlPort.beginPacket(controlPort.remoteIP(), controlPort.remotePort());
			controlPort.write((uint8_t*)amSignature,          sizeof(amSignature));
			controlPort.write((uint8_t*)amInvitationAccepted, sizeof(amInvitationAccepted));
			controlPort.write((uint8_t*)amProtocolVersion,    sizeof(amProtocolVersion));
			invitation.initiatorToken = htonl(invitation.initiatorToken);
			invitation.ssrc           = htonl(ssrc);
			controlPort.write(reinterpret_cast<uint8_t*>(&invitation), invitation.getLength());
		success = controlPort.endPacket();
		controlPort.flush();
	}

	void receivedDataInvitation(AppleMIDI_Invitation& invitation)
	{
		//Serial.println("ReceivedControlInvitation");

		//Serial.print("initiatorToken: 0x");
		//Serial.print(invitation.initiatorToken, HEX);
		//Serial.print(", senderSSRC: 0x");
		//Serial.print(invitation.ssrc, HEX);
		//Serial.print(", sessionName: ");
		//Serial.println(invitation.sessionName);

		auto participant = getParticipant(invitation.ssrc); // Participant!!!
		if (NULL == participant)
		{
			//AppleMIDI_InvitationRejected invitationRejected(invitation.ssrc, invitation.initiatorToken, invitation.sessionName);
			//write(_controlPort, invitationRejected, _controlPort.remoteIP(), _controlPort.remotePort());
			return;
		}

		// dada
		// Invitation Accepted
		auto success = dataPort.beginPacket(dataPort.remoteIP(), dataPort.remotePort());
			dataPort.write((uint8_t*)amSignature, sizeof(amSignature));
			dataPort.write((uint8_t*)amInvitationAccepted, sizeof(amInvitationAccepted));
			dataPort.write((uint8_t*)amProtocolVersion, sizeof(amProtocolVersion));
			invitation.initiatorToken = htonl(invitation.initiatorToken);
			invitation.ssrc = htonl(ssrc);
			dataPort.write(reinterpret_cast<uint8_t*>(&invitation), invitation.getLength());
		success = dataPort.endPacket();
		dataPort.flush();

		// indicate our buffer size
		//success = _controlPort.beginPacket(_controlPort.remoteIP(), _controlPort.remotePort());
		//	_controlPort.write((uint8_t*)amSignature, sizeof(amSignature));
		//	_controlPort.write((uint8_t*)amBitrateReceiveLimit, sizeof(amBitrateReceiveLimit));
		//	_controlPort.write((uint8_t*)amProtocolVersion, sizeof(amProtocolVersion));
		//success = _controlPort.endPacket();
		//_controlPort.flush();

		// indicates that the 2-step invitation has occured
	//	participant->

		//_sessionManager[i].contentIP = _dataPort.remoteIP();
		//_sessionManager[i].contentPort = _dataPort.remotePort();
		//_sessionManager[i].invite.status = None;
		//_sessionManager[i].syncronization.enabled = true; // synchronisation can start

		participant->initiator = ParticipantMode::Slave;

		if (_connectedCallback != 0)
			_connectedCallback(invitation.ssrc, invitation.sessionName);
	}

	void receivedSyncronization(AppleMIDI_Syncronization& syncronization, const amPortType& portType)
	{
		//Serial.println("receivedSyncronization");

		switch (syncronization.count) {
		case SYNC_CK0: /* From session initiator */
			syncronization.count = SYNC_CK1;
			syncronization.timestamps[syncronization.count] = _rtpMidiClock.Now();			
			break;
		case SYNC_CK1: /* From session responder */
			/* compute media delay */
			//uint64_t diff = (now - synchronization.timestamps[0]) / 2;
			/* approximate time difference between peer and self */
			//diff = synchronization.timestamps[2] + diff - now;
			// Send CK2
			syncronization.count = SYNC_CK2;
			syncronization.timestamps[syncronization.count] = _rtpMidiClock.Now();
			/* getting this message means that the responder is still alive! */
			/* remember the time, if it takes to long to respond, we can assume the responder is dead */
			/* not implemented at this stage*/
			//Sessions[index].syncronization.lastTime = _rtpMidiClock.Now();
			//Sessions[index].syncronization.count++;
			break;
		case SYNC_CK2: /* From session initiator */
			/* compute media delay */
			//uint64_t diff = (synchronization.timestamps[2] - synchronization.timestamps[0]) / 2;
			/* approximate time difference between peer and self */
			//diff = synchronization.timestamps[2] + diff - now;
			syncronization.count = SYNC_CK0;
			syncronization.timestamps[syncronization.count] = _rtpMidiClock.Now();
			break;
		}

		// only on the data port
		// Invitation Accepted
		auto success = dataPort.beginPacket(dataPort.remoteIP(), dataPort.remotePort());
			dataPort.write((uint8_t*)amSignature, sizeof(amSignature));
			dataPort.write((uint8_t*)amSyncronization, sizeof(amSyncronization));
			syncronization.ssrc = htonl(ssrc);
			syncronization.timestamps[0] = htonll(syncronization.timestamps[0]);
			syncronization.timestamps[1] = htonll(syncronization.timestamps[1]);
			syncronization.timestamps[2] = htonll(syncronization.timestamps[2]);
			dataPort.write(reinterpret_cast<uint8_t*>(&syncronization), sizeof(syncronization));
		success = dataPort.endPacket();
		dataPort.flush();
	}

	void receivedEndSession(AppleMIDI_EndSession& endSession, const amPortType& portType)
	{
		//Serial.println("receivedEndSession");

		//Serial.print("initiatorToken: 0x");
		//Serial.print(endSession.initiatorToken, HEX);
		//Serial.print(", senderSSRC: 0x");
		//Serial.println(endSession.ssrc, HEX);

		auto participant = getParticipant(endSession.ssrc);
		if (NULL != participant)
		{
			//Serial.println("FOUND");
			participant->ssrc = 0;
		}

	}

	void ManagePendingInvites()
	{
	}

	void ManageTiming()
	{
	}

	Participant<UdpClass>* getParticipant(const uint32_t ssrc)
	{
		for (int i = 0; i < MAX_PARTICIPANTS; i++)
			if (ssrc == participants[i].ssrc)
				return &(participants[i]);
		return NULL;
	}

	Participant<UdpClass>* createParticipant()
	{
		for (int i = 0; i < MAX_PARTICIPANTS; i++)
			if (0 == participants[i].ssrc)
				return &participants[i];
		return NULL;
	}

};

END_APPLEMIDI_NAMESPACE