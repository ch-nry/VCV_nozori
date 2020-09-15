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

// waveshapper
// Pot 1 : wet / dry
// Pot 2 : wet / dry mod
// Pot 3 : WS amplitude
// Pot 4 : WS amplitude modulation
// Pot 5 : disto
// Pot 6 : disto modulation
// CV 1 : bypass
// CV 2 : wet / dry  modulation value
// CV 3 : WS modulation value
// CV 4 : disto modulation value
// IN 1 : in 1
// IN 2 : in 2 / Pan
// Selecteur3 : type : stereo / oposit / mono + pan
// OUT 1 : OUT L
// OUT 2 : OUT R

int32_t shape1_goal, shape2_goal;
int32_t shape1_save, shape2_save;
int32_t disto1_goal, disto2_goal;
int32_t disto1_save, disto2_save;
int32_t audio1_filter, audio2_filter;

//state & 0b01: presence audio R
//state & 0b10: presence audio L
// state>>2 : 0 = bypass
// state>>2 : 1 = bypass + pan
// state>>2 : 2 = pan
// state>>2 : 3 = stereo

int32_t last_inL, last_inR;

inline int32_t gain2_cliped_S32(int32_t in) {  
  return(saturate_S31(in, in)<<1);
}


inline int32_t disto1 (int32_t in) {
  return(gain2_cliped_S32(in-m_s32xs32_s32H(in, abs(in)))); 
}

inline void WS_init_() {
  state = 0;
  init_chaos();

  shape1_save = 0;
  shape2_save = 0;
  disto1_save = 0;
  disto2_save = 0;
  audio1_filter = 0;
  audio2_filter = 0;
  last_inL = 0;
  last_inR = 0;
}

inline void WS_loop_() {
  uint32_t toggle, state_local;
  int32_t CV2_value, CV3_value, CV4_value;
  int32_t disto1, disto2, shape1, shape2, dry1, dry2;
  int32_t tmpS;
  
  filter16_nozori_68
  test_connect_loop_68();
  toggle = get_toggle();
  chaos(15); // for default mod values
  
  if (CV2_connect < 60) CV2_value = CV_filter16_out[index_filter_cv2] - CV2_0V; else CV2_value = chaos_dx>>16;
  if (CV3_connect < 60) CV3_value = CV_filter16_out[index_filter_cv3] - CV3_0V; else CV3_value = chaos_dy>>16;
  if (CV4_connect < 60) CV4_value = CV_filter16_out[index_filter_cv4] - CV4_0V; else CV4_value = chaos_dz>>16;
  CV2_value = min(0x7FFF,max(-0x7FFF,CV2_value));
  CV3_value = min(0x7FFF,max(-0x7FFF,CV3_value));
  CV4_value = min(0x7FFF,max(-0x7FFF,CV4_value));

  led2((CV3_value+0x7FFF)>>7);
  led4((CV4_value+0x7FFF)>>7);
  
  // waveshape
  dry1 = CV_filter16_out[index_filter_pot1];
  tmpS = CV2_value>>1;
  tmpS *= CV_filter16_out[index_filter_pot2];
  tmpS >>= 15;
  dry1 += tmpS;
  dry1 = min(max(0, dry1), 0xFFF0)<<8;
  dry1_goal = dry1;
  if (toggle == 1) {
    dry2 = CV_filter16_out[index_filter_pot1];
    //tmpS = CV2_value>>1;
    //tmpS *= CV_filter16_out[index_filter_pot4];
    //tmpS >>= 15;
    dry2 -= tmpS;
    dry2 = min(max(0, dry2), 0xFFF0)<<8;
    dry2_goal = dry2;
  } else dry2_goal = dry1;

  // shape
  shape1 = CV_filter16_out[index_filter_pot3];
  tmpS = CV3_value>>1;
  tmpS *= CV_filter16_out[index_filter_pot4];
  tmpS >>= 15;
  shape1 += tmpS;
  shape1_goal = min(max(0, shape1), 0xFFFF)<<8;
  if (toggle == 1) {
    shape2 = CV_filter16_out[index_filter_pot3];
    //tmpS = CV3_value>>1;
    //tmpS *= CV_filter16_out[index_filter_pot6];
    //tmpS >>= 15;
    shape2 -= tmpS;
    shape2_goal = min(max(0, shape2), 0xFFFF)<<8;
  } else shape2_goal = shape1_goal;

  // disto
  disto1 = CV_filter16_out[index_filter_pot5];
  tmpS = CV4_value>>1;
  tmpS *= CV_filter16_out[index_filter_pot6];
  tmpS >>= 15;
  disto1 += tmpS;
  disto1_goal = min(max(0, disto1), 0xFFFF)<<8;
  if (toggle == 1) {
    disto2 = CV_filter16_out[index_filter_pot5];
    //tmpS = CV4_value>>1;
    //tmpS *= CV_filter16_out[index_filter_pot2];
    //tmpS >>= 15;
    disto2 -= tmpS;
    disto2_goal = min(max(0, disto2), 0xFFFF)<<8;
  } else disto2_goal = disto1_goal;

  state_local = 0;
  if (IN1_connect < 60) state_local += 0b10;
  if (IN2_connect < 60) state_local += 0b01;
  if ( (CV1_connect < 60) && (CV_filter16_out[index_filter_cv1] > 0xA000) ) { // bypass
    if (toggle == 2) state_local += 1<<2; // bypass + pan
  }
  else if (toggle == 2) state_local += 2<<2; // pan
  else state_local += 3<<2; // normal mode
  state = state_local;
}

inline void WS_audio_() {
  int32_t disto1_local, disto2_local, dry1_local, dry2_local, shape1_local, shape2_local;
  uint32_t out, ws;
  int32_t tmpS, tmpS2, tmpS3;
  uint32_t DISTO_gain, DISTO_gain2;
  int32_t DISTO_in;
  int32_t inL, inR;
  uint32_t state_local;

  state_local = state;
  
  dry1_local = filter(dry1_goal, dry1_save, 6); // sur 24 bits
  dry1_save = dry1_local;
  dry2_local = filter(dry2_goal, dry2_save, 6);
  dry2_save = dry2_local;

  shape1_local = filter(shape1_goal, shape1_save, 6);
  shape1_save = shape1_local;
  shape2_local = filter(shape2_goal, shape2_save, 6);
  shape2_save = shape2_local;

  disto1_local = filter(disto1_goal, disto1_save, 6);
  disto1_save = disto1_local;
  disto2_local = filter(disto2_goal, disto2_save, 6);
  disto2_save = disto2_local;

  if (state_local & 0b10) inL = audio_inL^0x80000000; else inL = 0;
  if (state_local & 0b01) inR = audio_inR^0x80000000; else inR = inL;
    
  // disto
  DISTO_in = last_inL - (last_inL >> 3) + (inL >> 3); // audio fiter to reduce aliasing to to waveshaping
  last_inL = DISTO_in;
  DISTO_gain = disto1_local << 7;
  DISTO_gain2 = (DISTO_gain<<2) & 0x7FFFFFFF; 
  DISTO_in = (DISTO_gain & 1<<29)? disto1(DISTO_in) : DISTO_in;
  DISTO_in = (DISTO_gain & 1<<30)? disto1(disto1(DISTO_in)) : DISTO_in;
  tmpS = m_s32xs32_s32H(DISTO_in, abs(DISTO_in))<<1;
  tmpS = m_s32xs32_s32H(DISTO_gain2, DISTO_in-tmpS)<<1;
  tmpS += DISTO_in;
  DISTO_gain2 = (DISTO_gain < 0x7FFFFFFF)? 0x7FFFFFFF - (DISTO_gain>>2): 0x60000000;
  tmpS = m_s32xs32_s32H(tmpS, DISTO_gain2)<<1;

  // WS
  tmpS >>= 12;
  //tmpS = filterS(tmpS, audio1_filter, 2);
  //audio1_filter = tmpS;
  tmpS *= 0x800 + (shape1_local>>9);
  ws = fast_sin(tmpS + shape1_local);
  tmpS = ws^0x80000000;
  tmpS -= tmpS>>2;
  ws = tmpS;

  // Wet / Dry
  tmpS2 = inL >>1; // 31 bits
  tmpS3 = ws; 
  tmpS3 >>= 1; // 31bit
  tmpS = tmpS2 - tmpS3; // diference entre le dry et le wet
  tmpS >>= 16; // 17 bits de decalage
  tmpS *= dry1_local>>8;
  tmpS2 -= tmpS;
  tmpS2 <<= 1;

  state_local >>= 2; // "if else" are faster than a switch in the slowest computation case (stereo)
  if(state_local == 3) { // stereo
    out =  tmpS2^0x80000000;
    
    // disto
    DISTO_in = last_inR - (last_inR >> 3) + (inR >> 3);
    last_inR = DISTO_in;
    DISTO_gain = disto2_local << 7;
    DISTO_gain2 = (DISTO_gain<<2) & 0x7FFFFFFF; 
    DISTO_in = (DISTO_gain & 1<<29)? disto1(DISTO_in) : DISTO_in;
    DISTO_in = (DISTO_gain & 1<<30)? disto1(disto1(DISTO_in)) : DISTO_in;
    tmpS = m_s32xs32_s32H(DISTO_in, abs(DISTO_in))<<1;
    tmpS = m_s32xs32_s32H(DISTO_gain2, DISTO_in-tmpS)<<1;
    tmpS += DISTO_in;
    DISTO_gain2 = (DISTO_gain < 0x7FFFFFFF)? 0x7FFFFFFF - (DISTO_gain>>2): 0x60000000;
    tmpS = m_s32xs32_s32H(tmpS, DISTO_gain2)<<1;

    // WS
    tmpS >>= 12;
    //tmpS = filterS(tmpS, audio2_filter, 2);
    //audio2_filter = tmpS;
    tmpS *= 0x800 + (shape2_local>>9);
    ws = fast_sin(tmpS + shape2_local);
    tmpS = ws^0x80000000;
    tmpS -= tmpS>>2;
    ws = tmpS;
    
    // Wet / Dry
    tmpS2 = inR >> 1; // 31 bits
    tmpS3 = ws; 
    tmpS3 >>= 1; // 31bit
    tmpS = tmpS2 - tmpS3; // diference entre le dry et le wet
    tmpS >>= 16; // 17 bits de decalage
    tmpS *= dry2_local>>8;
    tmpS2 -= tmpS;
    tmpS2 <<= 1;

    audio_outL =  out;
    audio_outR = tmpS2^0x80000000;
  } else if(state_local == 2) { // pan
    out = tmpS2^0x80000000;
    macro_out_pan
  } else if(state_local == 1) { // bypass + pan
    out = inL^0x80000000;
    macro_out_pan
  } else if(state_local == 0) { // bypass
    audio_outL = inL^0x80000000;
    audio_outR = inR^0x80000000; 
  }
}
