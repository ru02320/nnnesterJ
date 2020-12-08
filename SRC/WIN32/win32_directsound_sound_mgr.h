
#ifndef _WIN32_DIRECTSOUND_SOUND_MGR_
#define _WIN32_DIRECTSOUND_SOUND_MGR_

#include <dsound.h> // directX include

#include "sound_mgr.h"
#include "win32_directsound_sound_mgr_mono.h"
#include "win32_directsound_sound_mgr_stereo.h"
#include "CSndRec.h"
#include "null_sound_mgr.h"


#define SOUND_MGR_MONO		0
#define SOUND_MGR_STEREO	1

class win32_directsound_sound_mgr
{
public:
	win32_directsound_sound_mgr(HWND window_handle, int sample_bits, int sample_rate, int buffer_length_in_frames, int st, int Volume);
	~win32_directsound_sound_mgr();

	void reset();

	sound_mgr *set_null();
	sound_mgr *set_mono();
	sound_mgr *set_stereo();

	// lock down for a period of inactivity
	void freeze();
	void thaw();

	void clear_buffer();

	boolean lock(sound_buf_pos which, void** buf, uint32* buf_len);
	void unlock();

	int get_buffer_len()  { return sound_mgr->get_buffer_len(); }

	// returns SOUND_BUF_LOW or SOUND_BUF_HIGH
	sound_buf_pos get_currently_playing_half();

	boolean IsNull() { return FALSE; }
	boolean IsStereo() { return stereo; }

	bool start_sndrec( char *fn, uint32 nSampleBits, uint32 nSampleRate );
	void end_sndrec();
	bool IsRecording();

protected:
	sound_mgr *sound_mgr;
	HWND window;
	int stereo;
	int rate;
	int s_bits;
	int buffer_length;
	int DSVolume;

private:
};


#endif
