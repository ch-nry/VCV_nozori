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

// AR couble with independant modulation for A and R
// Pot 1 : A
// Pot 2 : A
// Pot 3 : R
// Pot 4 : R
// Pot 5 : Mod Speed
// Pot 6 : Mod Speed
// CV 1 : mod A
// CV 2 : mod R
// CV 3 : Mod A
// CV 4 : Mod R
// IN 1 : GATE 1
// IN 2 : GATE 2
// Selecteur3 : 1;2 // 1&2; 1|2 // 1>2; 1<2
// OUT 1 : OUT 1
// OUT 2 : OUT 2


// ADSR : Attack curve
// this variable can be in betwwen this 2 exterm value:
// 0X47000000 : for slow curve
// 0x7FFFFFFF : for hard curve
// default is 0x7FFFFFFF
#define ADSR_Goal_value 0x4FFFFFFF

inline void AR_init_() {
  ADSR1_status = 2;
  ADSR1_goal = 0;  
  ADSR2_status = 2;
  ADSR2_goal = 0;
}

inline void AR_loop_() {
  uint32_t tmp, toggle;
  bool GATE1_local, GATE2_local;
  uint32_t ADSR1_status_local, ADSR1_goal_local;
  uint32_t ADSR2_status_local, ADSR2_goal_local;
  int32_t CV1_value, CV2_value, CV3_value, CV4_value;
  int32_t tmpS, tmpS1, tmpS2;
  
  filter16_nozori_68
  test_connect_loop_68();
  toggle = get_toggle();

  if (CV1_connect < 60) CV1_value = CV_filter16_out[index_filter_cv1] - CV1_0V; else CV1_value = 0;
  if (CV2_connect < 60) CV2_value = CV_filter16_out[index_filter_cv2] - CV2_0V; else CV2_value = 0;
  if (CV3_connect < 60) CV3_value = CV_filter16_out[index_filter_cv3] - CV3_0V; else CV3_value = 0;
  if (CV4_connect < 60) CV4_value = CV_filter16_out[index_filter_cv4] - CV4_0V; else CV4_value = 0;

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // AR1

    switch(toggle) {
    case 0: // 2 independant
        if (IN1_connect < 60) {
        GATE1_local = audio_inL > 0xA0000000;
        } else {
        GATE1_local = false;
        }
        if (IN2_connect < 60) {
        GATE2_local = audio_inR > 0xA0000000;
        } else {
        GATE2_local = GATE1_local;
        }
    break;
    case 1: // 1&2 ; 1|2
        if (IN1_connect < 60) {
        GATE1_local = audio_inL > 0xA0000000;
        } else {
        GATE1_local = false;
        }
        if (IN2_connect < 60) {
        GATE2_local = audio_inR > 0xA0000000;
        } else {
        GATE2_local = false;
        }
        tmp = GATE1_local & GATE2_local;
        GATE2_local = GATE2_local | GATE1_local;
        GATE1_local = tmp;
    break;
    case 2: // 1>2; 2>1
        if (IN1_connect < 60) {
        tmpS1 = audio_inL - IN1_0V;
        } else {
        tmpS1 = 0x10000000;
        }
        if (IN2_connect < 60) {
        tmpS2 = audio_inR - IN2_0V;
        } else {
        tmpS2 =  0x10000000;
        }
        GATE1_local = tmpS1 > tmpS2;
        GATE2_local = tmpS1 <= tmpS2;
    break;
    }

  ADSR1_status_local = ADSR1_status;
  
  // did we change state?
  if (!GATE1_local) { // no gate -> release
    ADSR1_status_local = 1;  // release status
  }
  else { // We have a gate
      ADSR1_status_local=0; // attack
  }

  switch (ADSR1_status_local) {
  case 0 : // attack
    tmp = CV_filter16_out[index_filter_pot1]; 
    ADSR1_goal_local = ADSR_Goal_value;

    // modulation du filtre en fct du CV
    tmpS =  CV1_value;
    tmpS *= CV_filter16_out[index_filter_pot5];
    tmpS >>= 16;
    tmpS += tmp;
    tmpS = max(-0, min(0xFFFF, tmpS));
    tmp = tmpS;

    break;
  case 1 : // release
    tmp = CV_filter16_out[index_filter_pot3];
    ADSR1_goal_local = 0;

    // modulation du filtre en fct du CV
    tmpS =  CV2_value;
    tmpS *= CV_filter16_out[index_filter_pot5];
    tmpS >>= 16;
    tmpS += tmp;
    tmpS = max(0, min(0xFFFF, tmpS));
    tmp = tmpS;
    break;
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

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // AR2
  
  ADSR2_status_local = ADSR2_status;

  
  // did we change state?
  if (!GATE2_local) { // no gate -> release
    ADSR2_status_local = 1;  // release status
  }
  else {
    ADSR2_status_local = 0;
  }

  switch (ADSR2_status_local) {
  case 0 : // attack
    tmp = CV_filter16_out[index_filter_pot2]; 
    ADSR2_goal_local = ADSR_Goal_value;

    // modulation du filtre en fct du CV
    tmpS =  CV3_value;
    tmpS *= CV_filter16_out[index_filter_pot6];
    tmpS >>= 16;
    tmpS += tmp;
    tmpS = max(0, min(0xFFFF, tmpS));
    tmp = tmpS;
  break;
  case 1 : // release
    tmp = CV_filter16_out[index_filter_pot4];
    ADSR2_goal_local = 0;

    // modulation du filtre en fct du CV
    tmpS =  CV4_value;
    tmpS *= CV_filter16_out[index_filter_pot6];
    tmpS >>= 16;
    tmpS += tmp;
    tmpS = max(0, min(0xFFFF, tmpS));
    tmp = tmpS;
  break;
  }

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

inline void AR_audio_() { 
  uint32_t ADSR1_out_tmp, ADSR2_out_tmp;
  
  ADSR1_out_tmp = ADSR1_out;
  ADSR1_out_tmp += m_s32xs32_s32H(((int32_t)ADSR1_goal - (int32_t)(ADSR1_out_tmp)), ADSR1_filter); 
  ADSR1_out_tmp = min(ADSR1_out_tmp, 0x3FFFFFFF);
  ADSR1_out = ADSR1_out_tmp;
  ADSR1_out_tmp = ADSR1_out_tmp<<1;
  ADSR1_out_tmp -= ADSR1_out_tmp>>2;
  audio_outL = ADSR1_out_tmp + OUT1_0V;

  ADSR2_out_tmp = ADSR2_out;
  ADSR2_out_tmp += m_s32xs32_s32H(((int32_t)ADSR2_goal - (int32_t)(ADSR2_out_tmp)), ADSR2_filter); 
  ADSR2_out_tmp = min(ADSR2_out_tmp, 0x3FFFFFFF);
  ADSR2_out = ADSR2_out_tmp;
  ADSR2_out_tmp = ADSR2_out_tmp<<1;
  ADSR2_out_tmp -= ADSR2_out_tmp>>2;
  audio_outR = ADSR2_out_tmp + OUT2_0V;
}
