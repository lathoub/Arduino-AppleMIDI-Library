BEGIN_APPLEMIDI_NAMESPACE

#include <midi_RingBuffer.h>
using namespace MIDI_NAMESPACE;

#include "AppleMidi_Defs.h"

template<class UdpClass>
class AppleMidiTransport;

template<class UdpClass>
class rtpMIDIParser
{
public:
	static int Parser(midi::RingBuffer<byte, BUFFER_MAX_SIZE>& buffer, Session<UdpClass>* session, const amPortType& portType)
	{
		return 0;
	}
};

END_APPLEMIDI_NAMESPACE