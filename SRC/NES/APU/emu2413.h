#ifndef _EMU2413_H_
#define _EMU2413_H_

#ifdef __cplusplus
extern "C" {
#endif

/*#define OPLL_ENABLE_DEBUG*/
/*#define OPLL_LOGFILE*/
#ifdef OPLL_ENABLE_DEBUG
#include <stdio.h>
#endif

#define PI 3.14159265358979

/* Standard clock = MSX clock */
#define MSX_CLK 3579545

/* YM2413 internal clock */
#define YM2413_CLK (MSX_CLK/72.0)

typedef unsigned int uint32 ;
typedef int	int32 ;
typedef signed short int16 ;

/* voice data */
typedef struct {
  unsigned int TL,FB,EG,ML,AR,DR,SL,RR,KR,KL,AM,PM,WF ;
  int update ; /* for real-time update */
} OPLL_PATCH ;

/* slot */
typedef struct {

  OPLL_PATCH *patch;  

  int type ;          /* 0 : modulator 1 : carrier */
  int update ;

  /* OUTPUT */
  int32 output[2] ;      /* Output value of slot */

  /* for Phase Generator (PG) */
  int32 *sintbl ;     /* Wavetable */
  uint32 phase ;      /* Phase */
  uint32 dphase ;     /* Phase increment amount */
  uint32 pgout ;      /* output */

  /* for Envelope Generator (EG) */
  int fnum ;          /* F-Number */
  int block ;         /* Block */
  int volume ;        /* Current volume */
  int sustine ;       /* Sustine 1 = ON, 0 = OFF */
  uint32 tll ;	      /* Total Level + Key scale level*/
  uint32 rks ;        /* Key scale offset (Rks) */
  int eg_mode ;       /* Current state */
  uint32 eg_phase ;   /* Phase */
  uint32 eg_dphase ;  /* Phase increment amount */
  uint32 egout ;      /* output */

  /* LFO (refer to opll->*) */
  int32 *plfo_am ;
  int32 *plfo_pm ;

} OPLL_SLOT ;

/* Channel */
typedef struct {

  int patch_number ;
  int key_status ;
  OPLL_SLOT *mod, *car ;

#ifdef OPLL_ENABLE_DEBUG
  int debug_keyonpatch[4] ;
#endif

} EMU2413_OPLL_CH ;

/* opll */
typedef struct {

  int32 output[4] ;

  /* Register */
  unsigned char reg[0x40] ; 
  int slot_on_flag[18] ;

  /* Rythm Mode : 0 = OFF, 1 = ON */
  int rythm_mode ;

  /* Pitch Modulator */
  uint32 pm_phase ;
  uint32 pm_dphase ;
  int32 lfo_pm ;

  /* Amp Modulator */
  uint32 am_phase ;
  uint32 am_dphase ;
  int32 lfo_am ;

  /* Noise Generator */
  uint32 whitenoise ;

  /* Channel & Slot */
  EMU2413_OPLL_CH *ch[9] ;
  OPLL_SLOT *slot[18] ;

  /* Voice Data */
  OPLL_PATCH *patch[19] ;
  int user_patch_update[2] ; /* flag for check patch update */

  int mask[10] ; /* mask[9] = RYTHM */

  int masterVolume ; /* 0min -- 64 -- 127 max (Liner) */
  int rythmVolume ;  /* 0min -- 64 -- 127 max (Liner) */

#ifdef OPLL_ENABLE_DEBUG
  int debug_rythm_flag ;
  int debug_base_ml ;
  int feedback_type ;    /* feedback type select */
  FILE *logfile ;
#endif
  
} OPLL ;

void OPLL_init(uint32 clk, uint32 rate) ;
void OPLL_close(void) ;
OPLL *OPLL_new() ;
void OPLL_reset(OPLL *) ;
void OPLL_reset_patch(OPLL *) ;
void OPLL_delete(OPLL *) ;
void OPLL_writeReg(OPLL *, uint32, uint32) ;
int16 OPLL_calc(OPLL *) ;
void OPLL_setPatch(OPLL *, int, OPLL_PATCH *) ;
void OPLL_forceRefresh(OPLL *) ;
void dump2patch(unsigned char *dump, OPLL_PATCH *patch) ;

#ifdef OPLL_ENABLE_DEBUG
void debug_base_ml_ctrl(OPLL *,int) ;
void OPLL_changeFeedbackMode(OPLL *opll) ;
#endif

#ifdef __cplusplus
}
#endif



#endif











