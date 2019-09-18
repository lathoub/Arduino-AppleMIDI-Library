#pragma once

#include "AppleMidi_Namespace.h"

#define SESSION_NAME_MAX_LEN 24

#define SYNC_CK0 0
#define SYNC_CK1 1
#define SYNC_CK2 2

/* Signature "Magic Value" for Apple network MIDI session establishment */
#define APPLEMIDI_PROTOCOL_SIGNATURE			0xffff

/* Apple network MIDI valid commands */
#define APPLEMIDI_COMMAND_INVITATION			0x494e		/*   "IN"   */
#define APPLEMIDI_COMMAND_INVITATION_REJECTED	0x4e4f		/*   "NO"   */
#define APLLEMIDI_COMMAND_INVITATION_ACCEPTED	0x4f4b		/*   "OK"   */
#define APPLEMIDI_COMMAND_ENDSESSION			0x4259		/*   "BY"   */
#define APPLEMIDI_COMMAND_SYNCHRONIZATION		0x434b		/*   "CK"   */
#define APPLEMIDI_COMMAND_RECEIVER_FEEDBACK		0x5253		/*   "RS"   */
#define APPLEMIDI_COMMAND_BITRATE_RECEIVE_LIMIT	0x524c		/*   "RL"   */

BEGIN_APPLEMIDI_NAMESPACE

END_APPLEMIDI_NAMESPACE