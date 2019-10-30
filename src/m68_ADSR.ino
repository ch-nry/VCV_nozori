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

// ADSR / VCA
// Pot 1 : A
// Pot 2 : D
// Pot 3 : S
// Pot 4 : R
// Pot 5 : Mod Speed
// Pot 6 : Mod Speed
// CV 1 : Gate 1
// CV 2 : Gate 2
// CV 3 : Mod Speed 1
// CV 4 : Mod Speed 2
// IN 1 : VCA in 1
// IN 2 : VCA in 2
// Selecteur3 : ADSR Loop Mod : no loop / loop AD, loop ADSR
// OUT 1 : OUT 1
// OUT 2 : OUT 2


// ADSR : Attack curve
// this variable can be in betwwen this 2 exterm value:
// 0X47000000 : for slow curve
// 0x7FFFFFFF : for hard curve
// default is 0x7FFFFFFF
#define ADSR_Goal_value 0x4FFFFFFF

inline void ADSR_VCA_init_() {
  ADSR1_status = 2;
  ADSR1_goal = 0;  
  ADSR2_status = 2;
  ADSR2_goal = 0;
  init_chaos();
}

inline void ADSR_VCA_loop_() {
  uint32_t tmp, toggle, sustain;
  bool GATE1_local, GATE2_local;
  uint32_t ADSR1_status_local, ADSR1_goal_local;
  uint32_t ADSR2_status_local, ADSR2_goal_local;
  int32_t CV3_value, CV4_value;
  int32_t tmpS;
  
  filter16_nozori_68
  test_connect_loop_68();
  toggle = get_toggle();
  //chaos(15); // for default mod values


  if (CV3_connect < 60) CV3_value = CV_filter16_out[index_filter_cv3] - CV3_0V; else CV3_value = -0x7FFF;
  if (CV4_connect < 60) CV4_value = CV_filter16_out[index_filter_cv4] - CV4_0V; else CV4_value =  0x7FFF;
  CV3_value = min(0x7FFF,max(-0x7FFF,CV3_value));
  CV4_value = min(0x7FFF,max(-0x7FFF,CV4_value));

//  CV3_value = -0x7FFF;
//  CV4_value = -0x7FFF;

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // ADSR1

  if (CV1_connect < 60) {
    GATE1_local = CV_filter16_out[index_filter_CV1] > 0xA000;
  } else {
    GATE1_local = true;
  }
  
  ADSR1_status_local = ADSR1_status;

  sustain = CV_filter16_out[index_filter_pot3] << 14; // 0x3FFFFFFF max
  
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
    if ( (toggle == 1) && (ADSR1_out < sustain + 0x00A00000) ) { // en mode loop1, on retrig une attack qd le decay s'approche du sustain)
      ADSR1_status_local = 0;
    }
    if ( (toggle == 2) && (ADSR1_status_local == 1) && (ADSR1_out < sustain + 0x00A00000) ) { // en mode loop1, on retrig une attack qd le decay s'approche du sustain)
      ADSR1_status_local = 3; // pseudo release mode
    }
    if ( (ADSR1_status_local == 3)  && (ADSR1_out < 0x00A00000) ) { // a la fin du release en mode loop2 : on recommence l'attack
      ADSR1_status_local = 0;
    }
  }

  switch (ADSR1_status_local) {
  case 0 : // attack
    tmp = CV_filter16_out[index_filter_pot1]; 
    ADSR1_goal_local = ADSR_Goal_value;
    break;
  case 1 : // decay -> sustain
    tmp = CV_filter16_out[index_filter_pot2];
    ADSR1_goal_local = sustain;
    break;
  case 2 : // release
    tmp = CV_filter16_out[index_filter_pot4];
    ADSR1_goal_local = 0;
    break;
  case 3 : // release en mode loop2
    tmp = CV_filter16_out[index_filter_pot4];
    ADSR1_goal_local = 0;
    break;
  }

  // modulation du filtre en fct du CV
  tmpS =  CV3_value;
  tmpS *= CV_filter16_out[index_filter_pot5];
  tmpS >>= 15;
  tmpS += tmp;
  tmpS = max(0, min(0xFFFF, tmpS));
  tmp = tmpS;
 
  // curve fader -> filter coef
  tmp = (0xFFFF-tmp) / 80;
  tmp += 0xB0;
  tmp = table_CV2increment[tmp];

  noInterrupts();
  ADSR1_status = ADSR1_status_local;
  ADSR1_filter = tmp; 
  ADSR1_goal = ADSR1_goal_local;
  interrupts();

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // ADSR2

  if (CV2_connect < 60) {
    GATE2_local = CV_filter16_out[index_filter_CV2] > 0xA000;
  } else {
    GATE2_local = GATE1_local;
  }
  
  ADSR2_status_local = ADSR2_status;

  sustain = CV_filter16_out[index_filter_pot3] << 14; // 0x3FFFFFFF max
  
  // did we change state?
  if (!GATE2_local) { // no gate -> release
    ADSR2_status_local = 2;  // release status
  }
  else {
    if ((ADSR2_status_local == 0) & (ADSR2_out >= 0x3FFFFFFF)) { // attack and signal is high, so we start a decay
      ADSR2_status_local = 1; // decay status
    }
    else if (ADSR2_status_local == 2) { // we got a gate, but use to be in release mode, so we start an attack
      ADSR2_status_local = 0;
    }
    if ( (toggle == 1) && (ADSR2_out < sustain + 0x00A00000) ) { // en mode loop1, on retrig une attack qd le decay s'approche du sustain)
      ADSR2_status_local = 0;
    }
    if ( (toggle == 2) && (ADSR2_status_local == 1) && (ADSR2_out < sustain + 0x00A00000) ) { // en mode loop1, on retrig une attack qd le decay s'approche du sustain)
      ADSR2_status_local = 3; // pseudo release mode
    }
    if ( (ADSR2_status_local == 3)  && (ADSR2_out < 0x00A00000) ) { // a la fin du release en mode loop2 : on recommence l'attack
      ADSR2_status_local = 0;
    }
  }

  switch (ADSR2_status_local) {
  case 0 : // attack
    tmp = CV_filter16_out[index_filter_pot1]; 
    ADSR2_goal_local = ADSR_Goal_value;
  break;
  case 1 : // decay -> sustain
    tmp = CV_filter16_out[index_filter_pot2];
    ADSR2_goal_local = CV_filter16_out[index_filter_pot3] << 14; // 0x3FFFFFFF max
  break;
  case 2 : // release
    tmp = CV_filter16_out[index_filter_pot4];
    ADSR2_goal_local = 0;
  break;
  case 3 : // release en mode loop2
    tmp = CV_filter16_out[index_filter_pot4];
    ADSR2_goal_local = 0;
  break;
  }
  
  // modulation du filtre en fct du CV
  tmpS =  CV4_value;
  tmpS *= CV_filter16_out[index_filter_pot6];
  tmpS >>= 15;
  tmpS += tmp;
  tmpS = max(0, min(0xFFFF, tmpS));
  tmp = tmpS;
 
  // curve fader -> filter coef
  tmp = (0xFFFF-tmp) / 80;
  tmp += 0xB0;
  tmp = table_CV2increment[tmp];
  
  noInterrupts();
  ADSR2_status = ADSR2_status_local;
  ADSR2_filter = tmp;
  ADSR2_goal = ADSR2_goal_local;
  interrupts();

  // Leds ///////////////////////////////////////////////////////
  led2(ADSR1_out >> 21);
  led4(ADSR2_out >> 21);
}

inline void ADSR_VCA_audio_() { 
  uint32_t ADSR1_out_tmp, ADSR2_out_tmp;
  int32_t tmpS;
  
  ADSR1_out_tmp = ADSR1_out;
  ADSR1_out_tmp += m_s32xs32_s32H(((int32_t)ADSR1_goal - (int32_t)(ADSR1_out_tmp)), ADSR1_filter); 
  ADSR1_out_tmp = min(ADSR1_out_tmp, 0x3FFFFFFF);
  ADSR1_out = ADSR1_out_tmp;
  if (IN1_connect < 60) { // audio input of the VCA
    tmpS = audio_inL^0x80000000;
    tmpS >>=16;
    tmpS *= ADSR1_out_tmp >> 14;
    audio_outL = tmpS ^0x80000000;
  }
  else { // no audio in, we output only the ADSR signal
    ADSR1_out_tmp += (ADSR1_out_tmp>>1);
    audio_outL = ADSR1_out_tmp + OUT1_0V;
  }
  ADSR2_out_tmp = ADSR2_out;
  ADSR2_out_tmp += m_s32xs32_s32H(((int32_t)ADSR2_goal - (int32_t)(ADSR2_out_tmp)), ADSR2_filter); 
  ADSR2_out_tmp = min(ADSR2_out_tmp, 0x3FFFFFFF);
  ADSR2_out = ADSR2_out_tmp;
  if (IN2_connect < 60) { // audio input of the VCA
    tmpS = audio_inR^0x80000000;
    tmpS >>=16;
    tmpS *= ADSR2_out_tmp >> 14;
    audio_outR = tmpS ^0x80000000;
  }
  else { // no audio in, we output only the ADSR signal
    ADSR2_out_tmp += (ADSR2_out_tmp>>1);
    audio_outR = ADSR2_out_tmp + OUT2_0V;
  }
}
