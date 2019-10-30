// --------------------------------------------------------------------------
// This file is part of the NOZORI firmware.
//
//    NOZORI firmware is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    NOZORI firmware is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with NOZORI firmware. If not, see <http://www.gnu.org/licenses/>.
// --------------------------------------------------------------------------


// Stockastique clock + ADSR + VCA
// Pot 1 : Clock FQ
// Pot 2 : A
// Pot 3 : Hold %
// Pot 4 : D
// Pot 5 : Syncop %
// Pot 6 : S
// Pot 7 : MOD (ou clock diviseur)
// Pot 8 : R
// IN 1 : MOD (ou clock)
// IN 2 : audio in
// Selecteur3 : mod (FQ clock / Syncop modulation / clock )
// OUT 1 : clock
// OUT 2 : ADSR  / audio

// ADSR : Attack curve
// this variable can be in betwwen this 2 exterm value:
// 0X47000000 : for slow curve
// 0x7FFFFFFF : for hard curve
// default is 0x7FFFFFFF
#define ADSR_Goal_value 0x4FFFFFFF


uint32_t old_gate, gate_diviseur;

inline void CLK_ADSR_init_() {
  ADSR1_status = 2;
  ADSR1_goal = 0;  
  LFO1_phase = 0;
  gate = false;
  old_gate = false;
  gate_diviseur = 0;
}

inline void CLK_ADSR_loop_() {
  uint32_t tmp, tmp2, toggle, sustain, seuil;
  bool GATE1_local;
  uint32_t ADSR1_status_local, ADSR1_goal_local;
  int32_t tmpS, freq;
  
  filter16_nozori_84
  test_connect_loop_84();
  toggle = get_toggle();

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // CLOCK
  freq = CV_filter16_out[index_filter_pot1] << 11;
  freq += 0x4000000;
  if ( (IN1_connect < 60) && (toggle == 0) ) {
    tmp = audio_inL>>18;
    tmp -= IN1_0V>>18;
    tmpS = tmp;
    tmpS *= CV_filter16_out[index_filter_pot7];
    //tmpS = min(max(tmpS, -0x3FFFFFFF), 0x3FFFFFFF);
    tmpS >>= 4;
    freq += tmpS; 
  }
  macro_fq2increment
  LFO1_increment = increment1;

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // RANDOM
  if ( (IN1_connect < 60) && (toggle == 2) ) {
    tmp = audio_inL > 0xA0000000;
  } else {
    tmp = (LFO1_phase < (CV_filter16_out[index_filter_pot3]<<16) ); // clock ON
  }

  if (toggle == 2) { // mod comme diviseur
    if (IN1_connect < 60) { // une prise branché
      if (hold == 1) { // on est en mode gate
        tmp2 = audio_inL > 0xA0000000;
      } else { // on n'avait pas de gate
        tmp2 = audio_inL > 0xB0000000;
      }
    }
    else  { // pas de prise branché
      tmp2 = false;
    }

    if (old_gate != tmp2) {
      old_gate = tmp2;
      if (tmp2) {
        gate_diviseur++;
      }
    }
    if (gate_diviseur >= (CV_filter16_out[index_filter_pot7] >> 13)+1) gate_diviseur = 0;

    if ((gate_diviseur == 0) && (tmp2) ){
        tmp2 = true;
    }  else { 
        tmp2 = false;
    }

    tmp = tmp2;
  } 
  else {
    tmp = (LFO1_phase < (CV_filter16_out[index_filter_pot3]<<16) ); // clock ON
  }

  if ( (hold == 0) && tmp ){ // New gate
    hold = 1;
    if (toggle == 1) { // modulation sur le seuil 
      tmpS = audio_inL >> 17;
      tmpS -= IN1_0V >> 17;
      tmpS *= CV_filter16_out[index_filter_pot7];
      tmpS >>= 14;
      tmpS += CV_filter16_out[index_filter_pot5];
      tmpS = min(max(tmpS, 0), 0xFFFF);
      seuil = tmpS;
      seuil <<= 16;
    } else {
      seuil = CV_filter16_out[index_filter_pot5]<<16;
    }
    if ( random32() >= seuil ) { // new Gate
      //audio_outL = OUT1_0V + (OUT1_1V * 5);
      gate = true;
      //set_led2(255);
    } else {
      //set_led2(0);
      //audio_outL = OUT1_0V;
      gate = false;
    }
    audio_outL = OUT1_0V + (OUT1_1V * 5);
    set_led2(255);
  }
  if (!tmp) {
      set_led2(0);
      audio_outL = OUT1_0V;
      gate = false;
      hold = 0; 
    }
  
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // ADSR
  GATE1_local= gate;
  ADSR1_status_local = ADSR1_status;

  sustain = CV_filter16_out[index_filter_pot6] << 14; // 0x3FFFFFFF max
  
  // did we change state?
  if (!GATE1_local) { // no gate -> release
    ADSR1_status_local = 2;  // release status
  }
  else { // We have a gate
    if ((ADSR1_status_local == 0) & (ADSR1_out >= 0x3FFFFFFF)) { // attack and signal is high, so we start a decay
      ADSR1_status_local = 1; // decay status
    }
    else if (ADSR1_status_local == 2) { // we got a gate, but use to be in release mode, so we start an attack
      ADSR1_status_local=0; // attack
    }
    /*
    if ( (toggle == 1) && (ADSR1_out < sustain + 0x00A00000) ) { // en mode loop1, on retrig une attack qd le decay s'approche du sustain)
      ADSR1_status_local = 0;
    }
    if ( (toggle == 2) && (ADSR1_status_local == 1) && (ADSR1_out < sustain + 0x00A00000) ) { // en mode loop1, on retrig une attack qd le decay s'approche du sustain)
      ADSR1_status_local = 3; // pseudo release mode
    }
    if ( (ADSR1_status_local == 3)  && (ADSR1_out < 0x00A00000) ) { // a la fin du release en mode loop2 : on recommence l'attack
      ADSR1_status_local = 0;
    }*/
  }

  switch (ADSR1_status_local) {
  case 0 : // attack
    tmp = CV_filter16_out[index_filter_pot2]; 
    ADSR1_goal_local = ADSR_Goal_value;
    break;
  case 1 : // decay -> sustain
    tmp = CV_filter16_out[index_filter_pot4];
    ADSR1_goal_local = sustain;
    break;
  case 2 : // release
    tmp = CV_filter16_out[index_filter_pot8];
    ADSR1_goal_local = 0;
    break;
    /*
  case 3 : // release en mode loop2
    tmp = CV_filter16_out[index_filter_pot8];
    ADSR1_goal_local = 0;
    break;
    */
  }

  // curve fader -> filter coef
  tmp = (0xFFFF-tmp) / 80;
  tmp += 0xB0;
  tmp = table_CV2increment[tmp];

  noInterrupts();
  ADSR1_status = ADSR1_status_local;
  ADSR1_filter = tmp; 
  ADSR1_goal = ADSR1_goal_local;
  interrupts();
}

inline void CLK_ADSR_audio_() { 
  uint32_t ADSR1_out_tmp;
  int32_t tmpS;
  
  LFO1_phase += LFO1_increment;
  
  ADSR1_out_tmp = ADSR1_out;
  ADSR1_out_tmp += m_s32xs32_s32H(((int32_t)ADSR1_goal - (int32_t)(ADSR1_out_tmp)), ADSR1_filter); 
  ADSR1_out_tmp = min(ADSR1_out_tmp, 0x3FFFFFFF);
  ADSR1_out = ADSR1_out_tmp;
  led4(ADSR1_out_tmp >> 21);
  if (IN2_connect < 60) { // audio input of the VCA
    tmpS = audio_inR^0x80000000;
    tmpS >>=16;
    tmpS *= ADSR1_out_tmp >> 14;
    audio_outR = tmpS ^0x80000000;
  }
  else { // no audio in, we output only the ADSR signal
    ADSR1_out_tmp += (ADSR1_out_tmp>>1);
    audio_outR = ADSR1_out_tmp + OUT1_0V;
  }
}
