#include "types.h"
#include "vrc7.h"
#include "nes_apu.h"
#include "nes_extsound.h"
#include "emu2413.h"


extern apu_t *apu;


static OPLL *opll;
static uint32 vrc7_address;



static void vrc7_snd_reset(void){
	OPLL_reset(opll);
	OPLL_reset_patch(opll);
}


static int vrc7_snd_init(void){
	OPLL_init(MSX_CLK, apu->sample_rate);
	opll = OPLL_new();
	opll->masterVolume = 127;
	vrc7_snd_reset();
	return 0;
}


static void vrc7_snd_shutdown(void){
	OPLL_delete(opll);
	OPLL_close();
}



static int32 vrc7_snd_process(void){
	return OPLL_calc(opll);
}


static void vrc7_snd_paramschanged(void){
}


static void vrc7_snd_write(uint32 address, uint8 value){
	if(address == 0x9010)
		vrc7_address = value;
	else if(address == 0x9030)
		OPLL_writeReg(opll, vrc7_address, value);
}


static uint8 vrc7_snd_read(uint32 address){
	return 0;
}



static apu_memwrite vrc7_memwrite[] =
{
   { 0x9010, 0x9010, vrc7_snd_write },
   { 0x9030, 0x9030, vrc7_snd_write },
   {     -1,     -1, NULL }
};


static apu_memread vrc7_memread[] =
{
//   { 0x4040, 0x40FF, vrc7_snd_read},
   {     -1,     -1, NULL }
};



apuext_t emu2413_ext =
{
   vrc7_snd_init,
   vrc7_snd_shutdown,
   vrc7_snd_reset,
   vrc7_snd_paramschanged,
   vrc7_snd_process,
   vrc7_memread,		/* apu_memread  */
   vrc7_memwrite,	/* apu_memwrite */
   NULL/* apu_memwrite */
};
