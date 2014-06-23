/*!
 *  @file		packet-apple-midi.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		AppleMIDI Library for the Arduino
 *	Version		0.3
 *  @author		lathoub 
 *	@date		04/04/14
 *  License		GPL
 */

#pragma once

#include "AppleMidi_Settings.h"
#include "dissector.h"

#include "utility/AppleMidi_Invitation.h"
#include "utility/AppleMidi_AcceptInvitation.h"
#include "utility/AppleMidi_ReceiverFeedback.h"
#include "utility/AppleMidi_Syncronization.h"
#include "utility/AppleMidi_BitrateReceiveLimit.h"
#include "utility/AppleMidi_EndSession.h"

const uint8_t amInvitation          [] = {'I', 'N'};
const uint8_t amAcceptInvitation    [] = {'O', 'K'};
const uint8_t amEndSession          [] = {'B', 'Y'};
const uint8_t amSyncronization      [] = {'C', 'K'};
const uint8_t amReceiverFeedback    [] = {'R', 'S'};
const uint8_t amBitrateReceiveLimit [] = {'R', 'L'};

const unsigned char stringTerminator [] = { 0x00 };

BEGIN_APPLEMIDI_NAMESPACE

class PacketAppleMidi {
private:

public:
	PacketAppleMidi()
	{
	}

	static int dissect_apple_midi(Dissector* dissector, AppleMidi_Class* appleMidi, unsigned char* packetBuffer, size_t packetSize)
	{

#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.print ("dissect_apple_midi ");
Serial.print (dissector->_identifier);
Serial.print (", packetSize is ");
Serial.println (packetSize);
#endif

		size_t offset = 0;

		if (packetSize < sizeof(amSignature))
		{
#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.print ("Not enough data ");
Serial.println (packetSize);
#endif
			return offset; // Will be zero
		}

		if (0 != memcmp((void*)(packetBuffer + offset), amSignature, sizeof(amSignature)))
		{
			/*
			* Unknown or unsupported signature.
			*/
#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("Signature not supported.");
#endif
			return offset; // use these 2 bytes
		}

		// first 2 bytes OK
		offset += sizeof(amSignature);

		//
		if (0 == memcmp((void*)(packetBuffer + offset), amInvitation, sizeof(amInvitation)))
		{
			offset += sizeof(amInvitation);

			int start = 4 + 4 + 4 + 1;

			// do we have a terminating string?
			for (int i = start; i < packetSize, offset + i < packetSize; i++) {

				if (0 == memcmp((packetBuffer + offset) + i, stringTerminator, sizeof(stringTerminator))) {

					AppleMIDI_Invitation invitation;

					invitation.version        = AppleMIDI_Util::readUInt32(packetBuffer + offset);
					invitation.initiatorToken = AppleMIDI_Util::readUInt32(packetBuffer + offset + 4);
					invitation.ssrc           = AppleMIDI_Util::readUInt32(packetBuffer + offset + 8);
					strcpy(invitation.name, (const char*)(packetBuffer + offset + 12));

					appleMidi->OnInvitation(dissector, invitation);

					offset += (i + 1);

					return offset;
				}
			}

#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("Not enough data for Invitation");
#endif

			return 0;
		}
		else if (0 == memcmp((void*)(packetBuffer + offset), amSyncronization, sizeof(amSyncronization)))
		{
			offset += sizeof(amSyncronization);

			if (packetSize >= (offset + 4 + 1 + 3 + (3 * 8)))
			{
				AppleMIDI_Syncronization syncronization;

				syncronization.ssrc          = AppleMIDI_Util::readUInt32 (packetBuffer + offset);
				syncronization.count         = AppleMIDI_Util::readUInt8  (packetBuffer + offset + 4);
				syncronization.timestamps[0] = AppleMIDI_Util::readTimeval(packetBuffer + offset + 4 + 1 + 3);
				syncronization.timestamps[1] = AppleMIDI_Util::readTimeval(packetBuffer + offset + 4 + 1 + 3 + 8);
				syncronization.timestamps[2] = AppleMIDI_Util::readTimeval(packetBuffer + offset + 4 + 1 + 3 + 8 + 8);

				appleMidi->OnSyncronization(dissector, syncronization);

				offset += 4 + 1 + 3 + (3 * 8);

				return offset;
			}

#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("Not enough data for Syncronization");
#endif
			return 0;
		}
		else if (0 == memcmp((void*)(packetBuffer + offset), amReceiverFeedback, sizeof(amReceiverFeedback)))
		{
			offset += sizeof(amReceiverFeedback);

			if (packetSize >= (offset + 4 + 2 + 2))
			{
				AppleMIDI_ReceiverFeedback receiverFeedback;

				receiverFeedback.ssrc          = AppleMIDI_Util::readUInt32 (packetBuffer + offset);
				receiverFeedback.sequenceNr    = AppleMIDI_Util::readUInt16 (packetBuffer + offset + 4);

				appleMidi->OnReceiverFeedback(dissector, receiverFeedback);

				offset += (4 + 2 + 2);

				return offset;
			}

#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("Not enough data for ReceiverFeedback");
#endif
			return 0;
		}
		else if (0 == memcmp((void*)(packetBuffer + offset), amBitrateReceiveLimit, sizeof(amBitrateReceiveLimit)))
		{
			offset += sizeof(amBitrateReceiveLimit);

			if (packetSize >= (offset + 4))
			{
				AppleMIDI_BitrateReceiveLimit bitrateReceiveLimit;

				bitrateReceiveLimit.bitratelimit = AppleMIDI_Util::readUInt32 (packetBuffer + offset);

				appleMidi->OnBitrateReceiveLimit(dissector, bitrateReceiveLimit);

				offset += (4);

				return offset;
			}

#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("Not enough data for BitrateReceiveLimit");
#endif
			return 0;
		}
		else if (0 == memcmp((void*)(packetBuffer + offset), amEndSession, sizeof(amEndSession)))
		{
			offset += sizeof(amEndSession);

			if (packetSize >= (offset + 4 + 4 + 4))
			{
				AppleMIDI_EndSession endSession;

				endSession.version        = AppleMIDI_Util::readUInt32(packetBuffer + offset);
				endSession.initiatorToken = AppleMIDI_Util::readUInt32(packetBuffer + offset + 4);
				endSession.ssrc           = AppleMIDI_Util::readUInt32(packetBuffer + offset + 8);

				appleMidi->OnEndSession(dissector, endSession);

				offset += 4 + 4 + 4;

				return offset;
			}

#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("Not enough data for EndSession");
#endif
			return 0;
		}

#ifdef APPLEMIDI_DEBUG_VERBOSE
		Serial.println("dissect_apple_midi. Signature OK, command unknown Not valid, skipping");
#endif

		return offset;
	}

};

END_APPLEMIDI_NAMESPACE
