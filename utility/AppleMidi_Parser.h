/*!
 *  @file		AppleMIDI_Parser.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		AppleMIDI Library for the Arduino
 *	Version		0.3
 *  @author		lathoub 
 *	@date		04/04/14
 *  License		GPL
 */

#pragma once

#include "utility/AppleMidi_Settings.h"
#include "utility/AppleMidi_Defs.h"
#include "utility/AppleMidi_Util.h"

#include "utility/AppleMidi_Invitation.h"
#include "utility/AppleMidi_EndSession.h"
#include "utility/AppleMIDI_BitrateReceiveLimit.h"

#define APPLEMIDI_PACKET_MAX_SIZE 96

BEGIN_APPLEMIDI_NAMESPACE
	
class AppleMIDI_Parser {
private:
	unsigned char _protocolBuffer[APPLEMIDI_PACKET_MAX_SIZE];
	int  _protocolBufferIndex;

	void* _appleMidi;

	void (*mReceivedInvitationCallback)(void*, Invitation_t&);
	void (*mReceivedEndSessionCallback)(void*, EndSession_t&);
	void (*mReceivedReceiverFeedbackCallback)(void*, ReceiverFeedback_t&);
	void (*mReceivedSyncronizationCallback)(void*, Syncronization_t&);

private:
	void resetBuffer(int index)
	{
		memcpy(_protocolBuffer, _protocolBuffer + index, APPLEMIDI_PACKET_MAX_SIZE - index);
		_protocolBufferIndex -= index;
	}

public:

	AppleMIDI_Parser()
	{
		mReceivedInvitationCallback = NULL;
		mReceivedEndSessionCallback = NULL;
		mReceivedReceiverFeedbackCallback = NULL;
		mReceivedSyncronizationCallback = NULL;
	}

	//
	void init(void* appleMidi)
	{
		_appleMidi = appleMidi;
		reset();
	}

	//
	void setHandleReceivedInvitation(void (*fptr)(void*, Invitation_t&)) { mReceivedInvitationCallback = fptr; }
	void setHandleReceivedEndSession(void (*fptr)(void*, EndSession_t&)) { mReceivedEndSessionCallback = fptr; }
	void setHandleReceivedReceiverFeedback(void (*fptr)(void*, ReceiverFeedback_t&)) { mReceivedReceiverFeedbackCallback = fptr; }
	void setHandleReceivedSyncronization(void (*fptr)(void*, Syncronization_t&)) { mReceivedSyncronizationCallback = fptr; }

	void reset()
	{
		_protocolBufferIndex = 0;
	}

	void add(unsigned char* packetBuffer, int packetSize)
	{
		// enough room in buffer? If so, reset protocolBuffer back to zero
		if (_protocolBufferIndex + packetSize > APPLEMIDI_PACKET_MAX_SIZE)
			resetBuffer(0);

		// Add to the end of the protocolBuffer
		memcpy(_protocolBuffer + _protocolBufferIndex, packetBuffer, packetSize);
		_protocolBufferIndex += packetSize;

		while (true)
		{
			//// begin start of protocol
			//// Start to look for leading 0xff 0xff
			//while (_protocolBufferIndex > 0)
			//{
			//	if (_protocolBuffer[0] != amSignature[0])
			//	{
			//		memmove(_protocolBuffer, _protocolBuffer + 1, APPLEMIDI_PACKET_MAX_SIZE - 1);
			//		_protocolBufferIndex--;
			//	}
			//	else
			//		break;
			//}
			// end start of protocol
				
			// start look for Apple Midi signature
			// check for minimum length for amSignature
			if (_protocolBufferIndex < sizeof(amSignature))
				return;

			while (_protocolBufferIndex > 0)
			{
				if (0 != memcmp(_protocolBuffer, amSignature, sizeof(amSignature)))
				{
					// shift out, until we find the correct prefix
					memmove(_protocolBuffer, _protocolBuffer + 1, APPLEMIDI_PACKET_MAX_SIZE - 1);
					_protocolBufferIndex--;
				}
				else
					break;
			}
			// end look for sip header

			// enough bytes to see what command it is?
			if (_protocolBufferIndex < sizeof(amSignature) + 2)
				return;

			// so, now we have enough bytes to parse the command
			if (0 == memcmp(_protocolBuffer + sizeof(amSignature), amInvitation, sizeof(amInvitation)))
			{
				int position = AppleMIDI_Invitation::findEndOfInvitation(_protocolBuffer, _protocolBufferIndex);
				if (position > 0)
				{
					AppleMIDI_Invitation invitation(_protocolBuffer, position);
						 
					if (mReceivedInvitationCallback != 0)
						mReceivedInvitationCallback(_appleMidi, invitation);

					resetBuffer(position);
				}
				else
					return;
			}
			else if (0 == memcmp(_protocolBuffer + sizeof(amSignature), amEndSession, sizeof(amEndSession)))
			{
				int position = AppleMIDI_EndSession::findEndOfEndSession(_protocolBuffer, _protocolBufferIndex);
				if (position > 0)
				{
					AppleMIDI_EndSession endSession(_protocolBuffer, position);
						 
					if (mReceivedEndSessionCallback != 0)
						mReceivedEndSessionCallback(_appleMidi, endSession);

					resetBuffer(position);
				}
				else
					return;
			}
			else if (0 == memcmp(_protocolBuffer + sizeof(amSignature), amSyncronization, sizeof(amSyncronization)))
			{
				int position = AppleMIDI_Syncronization::findEndOfSyncronization(_protocolBuffer, _protocolBufferIndex);
				if (position > 0)
				{
					AppleMIDI_Syncronization syncronization(_protocolBuffer, position);

					if (mReceivedSyncronizationCallback != 0)
						mReceivedSyncronizationCallback(_appleMidi, syncronization);

					resetBuffer(position);
				}
				else
					return;
			}
			else if (0 == memcmp(_protocolBuffer + sizeof(amSignature), amReceiverFeedback, sizeof(amReceiverFeedback)))
			{
				int position = AppleMIDI_ReceiverFeedback::findEndOfReceiverFeedback(_protocolBuffer, _protocolBufferIndex);
				if (position > 0)
				{
					AppleMIDI_ReceiverFeedback receiverFeedback(_protocolBuffer, position);

					if (mReceivedReceiverFeedbackCallback != 0)
						mReceivedReceiverFeedbackCallback(_appleMidi, receiverFeedback);

					resetBuffer(position);
				}
				else
					return;
			}
			else
			{
				// Unknown command.
				Serial.println("Unknown command: ");
		//		Serial.println(_protocolBuffer, HEX);
				resetBuffer(_protocolBufferIndex);
			}
		}
	}
};

END_APPLEMIDI_NAMESPACE
