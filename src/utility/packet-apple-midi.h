/*!
 *  @file		packet-apple-midi.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		AppleMIDI Library for the Arduino
 *	Version		0.3
 *  @author		lathoub 
 *	@date		04/04/14
 *  License		Code is open source so please feel free to do anything you want with it; you buy me a beer if you use this and we meet someday (Beerware license).
 */

#pragma once

#include "AppleMidi_Settings.h"
#include "AppleMidi_Defs.h"
#include "dissector.h"

#include "utility/AppleMidi_Invitation.h"
#include "utility/AppleMidi_InvitationAccepted.h"
#include "utility/AppleMidi_InvitationRejected.h"
#include "utility/AppleMidi_ReceiverFeedback.h"
#include "utility/AppleMidi_Syncronization.h"
#include "utility/AppleMidi_BitrateReceiveLimit.h"
#include "utility/AppleMidi_EndSession.h"

#define NOT_ENOUGH_DATA 0

BEGIN_APPLEMIDI_NAMESPACE

class PacketAppleMidi {
private:

public:
	PacketAppleMidi()
	{
#ifdef APPLEMIDI_DEBUG_VERBOSE
		Serial.println("PacketAppleMidi verbose");
#endif
	}

	static int dissect_apple_midi(Dissector* dissector, IAppleMidi* appleMidi, unsigned char* packetBuffer, size_t packetSize)
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
#ifdef APPLEMIDI_DEBUG
Serial.print ("Not enough data ");
Serial.println (packetSize);
#endif
			return NOT_ENOUGH_DATA;
		}

		if (0 == memcmp(static_cast<void*>(packetBuffer + offset), amSignature, sizeof(amSignature)))
		{
			offset += sizeof(amSignature);
		}
		else
		{
			/*
			* Unknown or unsupported signature.
			*/
#ifdef APPLEMIDI_DEBUG
Serial.println("Signature not supported.");
#endif
			return sizeof(amSignature);
		}

		//
		if (0 == memcmp((void*)(packetBuffer + offset), amInvitation, sizeof(amInvitation)))
		{
			offset += sizeof(amInvitation);

			int start = 4 + 4 + 4 + 1;

			// do we have a terminating string?
			for (unsigned int i = start; /* i < packetSize , */ offset + i < packetSize; i++) {

				if (0 == memcmp((packetBuffer + offset) + i, stringTerminator, sizeof(stringTerminator))) {

					AppleMIDI_Invitation invitation;

					invitation.version        = AppleMIDI_Util::readUInt32(packetBuffer + offset);
					invitation.initiatorToken = AppleMIDI_Util::readUInt32(packetBuffer + offset + 4);
					invitation.ssrc           = AppleMIDI_Util::readUInt32(packetBuffer + offset + 8);
					strncpy(invitation.sessionName, (const char*)(packetBuffer + offset + 12), SESSION_NAME_MAX_LEN);

					appleMidi->OnInvitation(dissector, invitation);

					offset += (i + 1);

					return offset;
				}
			}

#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("Not enough data for Invitation");
#endif

			return NOT_ENOUGH_DATA;
		}
		else if (0 == memcmp(static_cast<void*>(packetBuffer + offset), amInvitationAccepted, sizeof(amInvitationAccepted)))
		{
			offset += sizeof(amInvitationAccepted);

			int start = 4 + 4 + 4 + 1;

			// do we have a terminating string?
			for (unsigned int i = start; /* i < packetSize,*/ offset + i < packetSize; i++) {

				if (0 == memcmp((packetBuffer + offset) + i, stringTerminator, sizeof(stringTerminator))) {

					AppleMIDI_InvitationAccepted invitationAccepted;

					invitationAccepted.version        = AppleMIDI_Util::readUInt32(packetBuffer + offset);
					invitationAccepted.initiatorToken = AppleMIDI_Util::readUInt32(packetBuffer + offset + 4);
					invitationAccepted.ssrc           = AppleMIDI_Util::readUInt32(packetBuffer + offset + 8);
					strncpy(invitationAccepted.sessionName, (const char*)(packetBuffer + offset + 12), SESSION_NAME_MAX_LEN);

					appleMidi->OnInvitationAccepted(dissector, invitationAccepted);

					offset += (i + 1);

					return offset;
				}
			}

#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("Not enough data for AcceptInvitation");
#endif

			return NOT_ENOUGH_DATA;
		}
		else if (0 == memcmp(static_cast<void*>(packetBuffer + offset), amInvitationRejected, sizeof(amInvitationRejected)))
		{
			offset += sizeof(amInvitationRejected);
		}
		else if (0 == memcmp(static_cast<void*>(packetBuffer + offset), amSyncronization, sizeof(amSyncronization)))
		{
			offset += sizeof(amSyncronization);

			if (packetSize >= (offset + 4 + 1 + 3 + (3 * 8)))
			{
				AppleMIDI_Syncronization syncronization;

				syncronization.ssrc          = AppleMIDI_Util::readUInt32(packetBuffer + offset);
				syncronization.count         = AppleMIDI_Util::readUInt8 (packetBuffer + offset + 4);
				syncronization.timestamps[0] = AppleMIDI_Util::readUInt64(packetBuffer + offset + 4 + 1 + 3);
				syncronization.timestamps[1] = AppleMIDI_Util::readUInt64(packetBuffer + offset + 4 + 1 + 3 + 8);
				syncronization.timestamps[2] = AppleMIDI_Util::readUInt64(packetBuffer + offset + 4 + 1 + 3 + 8 + 8);

				appleMidi->OnSyncronization(dissector, syncronization);

				offset += 4 + 1 + 3 + (3 * 8);

				return offset;
			}

#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("Not enough data for Syncronization");
#endif
			return NOT_ENOUGH_DATA;
		}
		else if (0 == memcmp(static_cast<void*>(packetBuffer + offset), amReceiverFeedback, sizeof(amReceiverFeedback)))
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
			return NOT_ENOUGH_DATA;
		}
		else if (0 == memcmp(static_cast<void*>(packetBuffer + offset), amBitrateReceiveLimit, sizeof(amBitrateReceiveLimit)))
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
			return NOT_ENOUGH_DATA;
		}
		else if (0 == memcmp(static_cast<void*>(packetBuffer + offset), amEndSession, sizeof(amEndSession)))
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
			return NOT_ENOUGH_DATA;
		}

#ifdef APPLEMIDI_DEBUG
		Serial.println("dissect_apple_midi. Signature OK, command unknown Not valid, skipping");
#endif

		return offset;
	}

};

END_APPLEMIDI_NAMESPACE
