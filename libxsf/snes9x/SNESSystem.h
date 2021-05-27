#pragma once

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#include <vector>

struct SNESSoundOut;

class SNESSystem
{
public:
	SNESSystem();
	virtual ~SNESSystem();

        bool Load(const uint8_t * rom, uint32_t romsize, const uint8_t * sram, uint32_t sramsize);
        void SoundInit(std::vector<uint8_t> *buffer, unsigned long *byte);
	void SoundReset();
	void Init();
	void Reset();
	void Term();

	void CPULoop();

        uint32_t soundSampleRate;
        uint8_t soundEnableFlag;

protected:
        SNESSoundOut * output;

private:
        uint8_t * sound_buffer;
        uint8_t soundEnableFlagOld;
};
