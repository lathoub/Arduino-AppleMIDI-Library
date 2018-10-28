/*!
 *	@brief		AppleMIDI Library for the Arduino
 *  @author		lathoub, hackmancoltaire, chris-zen
 */

BEGIN_APPLEMIDI_NAMESPACE

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnNoteOn(void* sender, DataByte channel, DataByte note, DataByte velocity)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print(F("> Note On (c="));
	DEBUGSTREAM.print(channel);
	DEBUGSTREAM.print(", n=");
	DEBUGSTREAM.print(note);
	DEBUGSTREAM.print(", v=");
	DEBUGSTREAM.print(velocity);
	DEBUGSTREAM.println(")");
#endif

	if (mNoteOnCallback)
		mNoteOnCallback(channel, note, velocity);
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnNoteOff(void* sender, DataByte channel, DataByte note, DataByte velocity)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Note Off (c=");
	DEBUGSTREAM.print(channel);
	DEBUGSTREAM.print(", n=");
	DEBUGSTREAM.print(note);
	DEBUGSTREAM.print(", v=");
	DEBUGSTREAM.print(velocity);
	DEBUGSTREAM.println(")");
#endif

	if (mNoteOffCallback)
		mNoteOffCallback(channel, note, velocity);
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnPolyPressure(void* sender, DataByte channel, DataByte note, DataByte pressure)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Poly Pressure (c=");
	DEBUGSTREAM.print(channel);
	DEBUGSTREAM.print(", n=");
	DEBUGSTREAM.print(note);
	DEBUGSTREAM.print(", p=");
	DEBUGSTREAM.print(pressure);
	DEBUGSTREAM.println(")");
#endif

	if (mAfterTouchPolyCallback)
		mAfterTouchPolyCallback(channel, note, pressure);
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnChannelPressure(void* sender, DataByte channel, DataByte pressure)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Channel Pressure (c=");
	DEBUGSTREAM.print(channel);
	DEBUGSTREAM.print(", p=");
	DEBUGSTREAM.print(pressure);
	DEBUGSTREAM.println(")");
#endif

	if (mAfterTouchChannelCallback)
		mAfterTouchChannelCallback(channel, pressure);
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnPitchBendChange(void* sender, DataByte channel, int pitch)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Pitch Bend (c=");
	DEBUGSTREAM.print(channel);
	DEBUGSTREAM.print(", p=");
	DEBUGSTREAM.print(pitch);
	DEBUGSTREAM.println(")");
#endif

	if (mPitchBendCallback)
		mPitchBendCallback(channel, pitch);
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnProgramChange(void* sender, DataByte channel, DataByte program)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Program Change (c=");
	DEBUGSTREAM.print(channel);
	DEBUGSTREAM.print(", p=");
	DEBUGSTREAM.print(program);
	DEBUGSTREAM.print(") ");

	switch (program)
	{
	default:
		DEBUGSTREAM.println("Other");
		break;
	}

#endif

	if (mProgramChangeCallback)
		mProgramChangeCallback(channel, program);
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnControlChange(void* sender, DataByte channel, DataByte controller, DataByte value)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Control Change (c=");
	DEBUGSTREAM.print(channel);
	DEBUGSTREAM.print(", e=");
	DEBUGSTREAM.print(controller);
	DEBUGSTREAM.print(", v=");
	DEBUGSTREAM.print(value);
	DEBUGSTREAM.print(") ");

	switch (controller)
	{
	case BankSelect:
		DEBUGSTREAM.println("BankSelect");
		break;
	case ModulationWheel:
		DEBUGSTREAM.println("ModulationWheel");
		break;
	case BreathController:
		DEBUGSTREAM.println("BreathController");
		break;
	case FootController:
		DEBUGSTREAM.println("FootController");
		break;
	case PortamentoTime:
		DEBUGSTREAM.println("PortamentoTime");
		break;
	case DataEntry:
		DEBUGSTREAM.println("DataEntry");
		break;
	case ChannelVolume:
		DEBUGSTREAM.println("ChannelVolume");
		break;
	case Balance:
		DEBUGSTREAM.println("Balance");
		break;
	case Pan:
		DEBUGSTREAM.println("Pan");
		break;
	case ExpressionController:
		DEBUGSTREAM.println("ExpressionController");
		break;
	case EffectControl1:
		DEBUGSTREAM.println("EffectControl1");
		break;
	case EffectControl2:
		DEBUGSTREAM.println("EffectControl2");
		break;
	case GeneralPurposeController1:
		DEBUGSTREAM.println("GeneralPurposeController1");
		break;
	case GeneralPurposeController2:
		DEBUGSTREAM.println("GeneralPurposeController2");
		break;
	case GeneralPurposeController3:
		DEBUGSTREAM.println("GeneralPurposeController3");
		break;
	case GeneralPurposeController4:
		DEBUGSTREAM.println("GeneralPurposeController4");
		break;
	case Sustain:
		DEBUGSTREAM.println("Sustain");
		break;
	case Portamento:
		DEBUGSTREAM.println("Portamento");
		break;
	case Sostenuto:
		DEBUGSTREAM.println("Sostenuto");
		break;
	case SoftPedal:
		DEBUGSTREAM.println("SoftPedal");
		break;
	case Legato:
		DEBUGSTREAM.println("Legato");
		break;
	case Hold:
		DEBUGSTREAM.println("Hold");
		break;
	case SoundController1:
		DEBUGSTREAM.println("SoundController1");
		break;
	case SoundController2:
		DEBUGSTREAM.println("SoundController2");
		break;
	case SoundController3:
		DEBUGSTREAM.println("SoundController3");
		break;
	case SoundController4:
		DEBUGSTREAM.println("SoundController4");
		break;
	case SoundController5:
		DEBUGSTREAM.println("SoundController5");
		break;
	case SoundController6:
		DEBUGSTREAM.println("SoundController6");
		break;
	case SoundController7:
		DEBUGSTREAM.println("SoundController7");
		break;
	case SoundController8:
		DEBUGSTREAM.println("SoundController8");
		break;
	case SoundController9:
		DEBUGSTREAM.println("SoundController9");
		break;
	case SoundController10:
		DEBUGSTREAM.println("SoundController10");
		break;
	case GeneralPurposeController5:
		DEBUGSTREAM.println("GeneralPurposeController5");
		break;
	case GeneralPurposeController6:
		DEBUGSTREAM.println("GeneralPurposeController6");
		break;
	case GeneralPurposeController7:
		DEBUGSTREAM.println("GeneralPurposeController7");
		break;
	case GeneralPurposeController8:
		DEBUGSTREAM.println("GeneralPurposeController8");
		break;
	case PortamentoControl:
		DEBUGSTREAM.println("PortamentoControl");
		break;
	case Effects1:
		DEBUGSTREAM.println("Effects1");
		break;
	case Effects2:
		DEBUGSTREAM.println("Effects2");
		break;
	case Effects3:
		DEBUGSTREAM.println("Effects3");
		break;
	case Effects4:
		DEBUGSTREAM.println("Effects4");
		break;
	case Effects5:
		DEBUGSTREAM.println("Effects5");
		break;
	case AllSoundOff:
		DEBUGSTREAM.println("AllSoundOff");
		break;
	case ResetAllControllers:
		DEBUGSTREAM.println("ResetAllControllers");
		break;
	case LocalControl:
		DEBUGSTREAM.println("LocalControl");
		break;
	case AllNotesOff:
		DEBUGSTREAM.println("AllNotesOff");
		break;
	case OmniModeOff:
		DEBUGSTREAM.println("OmniModeOff");
		break;
	case OmniModeOn:
		DEBUGSTREAM.println("OmniModeOn");
		break;
	case MonoModeOn:
		DEBUGSTREAM.println("MonoModeOn");
		break;
	case PolyModeOn:
		DEBUGSTREAM.println("PolyModeOn");
		break;
	default:
		DEBUGSTREAM.println("Other");
		break;
	}
#endif

	if (mControlChangeCallback)
		mControlChangeCallback(channel, controller, value);
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnSongSelect(void* sender, DataByte songNr)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> SongSelect (s=");
	DEBUGSTREAM.print(songNr);
	DEBUGSTREAM.println(")");
#endif

	if (mSongSelectCallback)
		mSongSelectCallback(songNr);
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnSongPosition(void* sender, unsigned short value)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> SongPosition (c=");
	DEBUGSTREAM.print(value);
	DEBUGSTREAM.println(")");
#endif

	if (mSongPositionCallback)
		mSongPositionCallback(value);
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnTimeCodeQuarterFrame(void* sender, DataByte value)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> TimeCodeQuarterFrame (c=");
	DEBUGSTREAM.print(value);
	DEBUGSTREAM.println(")");
#endif

	if (mTimeCodeQuarterFrameCallback)
		mTimeCodeQuarterFrameCallback(value);
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnTuneRequest(void* sender)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> TuneRequest ()");
#endif

	if (mTuneRequestCallback)
		mTuneRequestCallback();
}


/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnClock(void* sender)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Clock ()");
#endif

	if (mClockCallback)
		mClockCallback();
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnStart(void* sender)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Start ()");
#endif

	if (mStartCallback)
		mStartCallback();
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnContinue(void* sender)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Continue ()");
#endif

	if (mContinueCallback)
		mContinueCallback();
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnStop(void* sender)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Stop ()");
#endif

	if (mStopCallback)
		mStopCallback();
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnActiveSensing(void* sender)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> ActiveSensing ()");
#endif

	if (mActiveSensingCallback)
		mActiveSensingCallback();
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnReset(void* sender)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Reset ()");
#endif

	if (mResetCallback)
		mResetCallback();
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnSysEx(void* sender, const byte* data, uint16_t length)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> SysEx ()");
#endif

	if (mSysExCallback)
		mSysExCallback(data, length);
}

END_APPLEMIDI_NAMESPACE
