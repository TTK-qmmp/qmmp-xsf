#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

#ifdef WIN32
#include <Windows.h>
#endif

#include "port.h"
#include "snes9x.h"
#include "memmap.h"
#include "apu/apu.h"
#include "SNESSystem.h"

void WinSetDefaultValues ()
{
	// TODO: delete the parts that are already covered by the default values in WinRegisterConfigItems

	// ROM Options
	memset (&Settings, 0, sizeof (Settings));

	// Tracing options
	Settings.TraceDMA =	false;
	Settings.TraceHDMA = false;
	Settings.TraceVRAM = false;
	Settings.TraceUnknownRegisters = false;
	Settings.TraceDSP =	false;

	// ROM timing options (see also	H_Max above)
	Settings.PAL = false;
	Settings.FrameTimePAL =	20000;
	Settings.FrameTimeNTSC = 16667;
	Settings.FrameTime = 16667;

	// CPU options
	Settings.Paused	= false;

	Settings.TakeScreenshot=false;

}

static void InitSnes9X()
{
    WinSetDefaultValues ();

    // Sound options
    Settings.SoundSync = TRUE;
    Settings.Mute =	FALSE;
    Settings.SoundPlaybackRate = 32000;
    Settings.SixteenBitSound = TRUE;
    Settings.Stereo	= TRUE;
    Settings.ReverseStereo = FALSE;

    Memory.Init();

    S9xInitAPU();
    S9xInitSound(10, 0);
}

void S9xMessage(int , int , const char *)
{
//#ifdef WIN32
//	OutputDebugStringA(str);
//	OutputDebugStringA("\n");
//#endif
}

bool8 S9xOpenSoundDevice(void)
{
	return TRUE;
}


struct SNESSoundOut
{
  unsigned long *bytes_in_buffer = 0;
  std::vector<uint8_t> *sample_buffer = 0;

  // Receives signed 16-bit stereo audio and a byte count
  void write(const void* samples, unsigned long bytes)
  {
    if(!sample_buffer || !bytes_in_buffer)
    {
      return;
    }

    sample_buffer->resize(*bytes_in_buffer + bytes);
    memcpy(&sample_buffer->at(*bytes_in_buffer), samples, bytes);
    *bytes_in_buffer += bytes;
  }
};


SNESSystem::SNESSystem() :
    output(NULL),
	soundSampleRate(32000),
	soundEnableFlag(0xff),
	soundEnableFlagOld(0)
{
	sound_buffer = new uint8[2 * 2 * 96000 / 5];
    output = new SNESSoundOut;
}

SNESSystem::~SNESSystem()
{
	Term();

	delete[] sound_buffer;
    delete output;
}

bool SNESSystem::Load(const uint8 * rom, uint32 romsize, const uint8 * sram, uint32 sramsize)
{
	Term();

    InitSnes9X();

	if (!Memory.LoadROMSNSF(rom, romsize, sram, sramsize))
		return false;

	Settings.SoundPlaybackRate = soundSampleRate;
	S9xSetSoundMute(FALSE);

	return true;
}

void SNESSystem::SoundInit(std::vector<uint8_t> *buffer, unsigned long *byte)
{
    output->bytes_in_buffer = byte;
    output->sample_buffer = buffer;
}

void SNESSystem::SoundReset()
{
	Settings.SoundPlaybackRate = soundSampleRate;
    S9xInitSound(10, 0);

	soundEnableFlag = 0xff;
	soundEnableFlagOld = 0;
}

void SNESSystem::Init()
{
}

void SNESSystem::Reset()
{
	S9xReset();
}

void SNESSystem::Term()
{
    Memory.Deinit();
    S9xDeinitAPU();
}

void SNESSystem::CPULoop()
{
	if (soundEnableFlag != soundEnableFlagOld)
	{
        S9xSetSoundControl(soundEnableFlag);
		soundEnableFlagOld = soundEnableFlag;
	}

    S9xSyncSound();
	S9xMainLoop();

	unsigned bytes = (S9xGetSampleCount() << 1) & ~3;
	ZeroMemory(sound_buffer, bytes);
	S9xMixSamples(sound_buffer, bytes >> 1);

    if (output != NULL)
	{
        output->write(sound_buffer, bytes);
	}
}
