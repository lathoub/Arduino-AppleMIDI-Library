#include "AppleMidi_Defs.h"

BEGIN_APPLEMIDI_NAMESPACE

class SessionManager
{
	Session_t	_sessions[MAX_SESSIONS];

	char _sessionName[SESSION_NAME_MAX_LEN + 1];

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

public:

	SessionManager()
	{
		// A call randonSeed is mandatory, with millis as a seed.
		// The time between booting and needing the SSRC for the first time (first network traffic) is
		// a good enough random seed.
		long seed = (long)micros();
		randomSeed(seed);

		// not full range of UINT32_MAX (unsigned!), but (signed) long should suffice
		_ssrc = random(1, INT32_MAX);
	}

	uint32_t getSynchronizationSource()
	{
		return _ssrc;
	}

	char* getSessionName()
	{
		return _sessionName;
	}

	void setSessionName(const char* sessionName)
	{
		strncpy(_sessionName, sessionName, SESSION_NAME_MAX_LEN);
	}

	int GetFreeSessionSlot()
	{
		for (int i = 0; i < MAX_SESSIONS; i++)
			if (0 == _sessions[i].ssrc)
				return i;
		return -1;
	}

	int GetSessionSlotUsingSSrc(const uint32_t ssrc)
	{
		for (int i = 0; i < MAX_SESSIONS; i++)
			if (ssrc == _sessions[i].ssrc)
				return i;
		return -1;
	}

	int GetSessionSlotUsingInitiatorToken(const uint32_t initiatorToken)
	{
		for (int i = 0; i < MAX_SESSIONS; i++)
			if (initiatorToken == _sessions[i].invite.initiatorToken)
				return i;
		return -1;
	}

	void CompleteLocalSessionControl(AppleMIDI_InvitationAccepted& invitationAccepted)
	{
		// Find slot, based on initiator token
		int i = GetSessionSlotUsingInitiatorToken(invitationAccepted.initiatorToken);
		if (i < 0)
		{

#if (APPLEMIDI_DEBUG)
			DEBUGSTREAM.println("hmm, initiatorToken not found");
#endif
			return;
		}

		//
		if (_sessions[i].invite.status != WaitingForControlInvitationAccepted)
		{
#if (APPLEMIDI_DEBUG) // issue warning
			DEBUGSTREAM.println("status not what expected");
#endif
		}

		// Initiate next step in the invitation process
		_sessions[i].invite.lastSend = 0;
		_sessions[i].invite.attempts = 0;
		_sessions[i].invite.ssrc = invitationAccepted.ssrc;
		_sessions[i].invite.status = SendContentInvite;
	}

	void CompleteLocalSessionContent(AppleMIDI_InvitationAccepted& invitationAccepted)
	{
		// Find slot, based on initiator token
		int i = GetSessionSlotUsingInitiatorToken(invitationAccepted.initiatorToken);
		if (i < 0)
		{
#if (APPLEMIDI_DEBUG)
			DEBUGSTREAM.println("hmm, initiatorToken not found");
#endif
			return;
		}

		//
		if (_sessions[i].invite.status != WaitingForContentInvitationAccepted)
		{
#if (APPLEMIDI_DEBUG) // issue warning
			DEBUGSTREAM.println("status not what expected");
#endif
		}

		// Finalize invitation process
		_sessions[i].ssrc = invitationAccepted.ssrc;
		_sessions[i].invite.status = None;
		_sessions[i].syncronization.enabled = true; // synchronisation can start

		//if (mConnectedCallback != 0)
		//	mConnectedCallback(Sessions[i].ssrc, invitationAccepted.sessionName);
	}

	void CreateLocalSession(const int i, const uint32_t ssrc)
	{
#if (APPLEMIDI_DEBUG)
		DEBUGSTREAM.print("New Local Session in slot ");
		DEBUGSTREAM.print(i);
		DEBUGSTREAM.print(" with SSRC 0x");
		DEBUGSTREAM.println(ssrc, HEX);
#endif

		_sessions[i].ssrc = ssrc;
		_sessions[i].seqNum = 1;
		_sessions[i].initiator = Remote;
		_sessions[i].syncronization.lastTime = 0;
		_sessions[i].syncronization.count = 0;
		_sessions[i].syncronization.busy = false;
		_sessions[i].syncronization.enabled = false;
		_sessions[i].invite.status = ReceiveControlInvitation;
	}

	void CreateRemoteSession(IPAddress ip, uint16_t port)
	{
		int i = GetFreeSessionSlot();
		if (i < 0)
		{
#if (APPLEMIDI_DEBUG)
			DEBUGSTREAM.println("Invite: No free slot availble, invitation cancelled.");
#endif
			return;
		}

#if (APPLEMIDI_DEBUG)
		DEBUGSTREAM.print("New Remote Session in slot ");
		DEBUGSTREAM.println(i);
#endif

		_sessions[i].ssrc = -1;
		_sessions[i].seqNum = 0;
		_sessions[i].initiator = Local;
		_sessions[i].contentIP = ip;
		_sessions[i].contentPort = port + 1;
		_sessions[i].syncronization.lastTime = 0;
		_sessions[i].syncronization.count = 0;
		_sessions[i].syncronization.busy = false;
		_sessions[i].syncronization.enabled = false;

		_sessions[i].invite.remoteHost = ip;
		_sessions[i].invite.remotePort = port;
		_sessions[i].invite.lastSend = 0;
		_sessions[i].invite.attempts = 0;
		_sessions[i].invite.status = SendControlInvite;
	}

	void DeleteSession(const uint32_t ssrc)
	{
		// Find the slot first
		int slot = GetSessionSlotUsingSSrc(ssrc);
		if (slot < 0)
			return;

		DeleteSession(slot);
	}

	void DeleteSession(int slot)
	{
		// Then zero-ize it
		_sessions[slot].ssrc = 0;
		_sessions[slot].seqNum = 0;
		_sessions[slot].initiator = Undefined;
		_sessions[slot].invite.status = None;
		_sessions[slot].syncronization.enabled = false;

#if (APPLEMIDI_DEBUG)
		DEBUGSTREAM.print("Freeing Session slot ");
		DEBUGSTREAM.println(slot);
#endif
	}

	void DeleteSessions()
	{
		for (int slot = 0; slot < MAX_SESSIONS; slot++)
		{
			_sessions[slot].ssrc = 0;
			_sessions[slot].initiator = Undefined;
			_sessions[slot].invite.status = None;
			_sessions[slot].invite.status = None;
			_sessions[slot].invite.attempts = 0;
			_sessions[slot].invite.lastSend = 0;
			_sessions[slot].syncronization.enabled = false;
		}
	}

	void DumpSession()
	{
#if (APPLEMIDI_DEBUG)
		for (int i = 0; i < MAX_SESSIONS; i++)
		{
			DEBUGSTREAM.print("Slot ");
			DEBUGSTREAM.print(i);
			DEBUGSTREAM.print(" ssrc = 0x");
			DEBUGSTREAM.println(_sessions[i].ssrc, HEX);
		}
#endif
	}
};

END_APPLEMIDI_NAMESPACE