BEGIN_APPLEMIDI_NAMESPACE

#include <midi_RingBuffer.h>
using namespace MIDI_NAMESPACE;

#include "AppleMidi_Settings.h"
#include "AppleMidi_Util.h"

template<class UdpClass>
class AppleMidiTransport;

template<class UdpClass>
class rtpMIDIParser
{
public:
	static int Parser(midi::RingBuffer<byte, BUFFER_MAX_SIZE>& buffer, AppleMidiTransport<UdpClass>* rtpMidi, const amPortType& portType)
	{
		return 0;
	}
};

END_APPLEMIDI_NAMESPACE