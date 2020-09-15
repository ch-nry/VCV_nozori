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

//Peter de Jong chaotic attractos, used as LFO or audio source
// Pot 1 : FQ
// Pot 2 : Mod FQ
// Pot 3 : curve
// Pot 4 : Mod curve
// Pot 5 : A
// Pot 6 : B
// Pot 7 : C
// Pot 8 : D
// IN 1 : FQ mod value
// IN 2 : curve mod value
// Selecteur3 : interpolation type (none, linear, cubic)
// OUT 1 : OUT X
// OUT 2 : OUT Y

int32_t out_oldX1, out_oldX2, out_oldX3, out_oldX4;
int32_t out_oldY1, out_oldY2, out_oldY3, out_oldY4;

inline void DEJONG_LFO_init_() {
  send_dac(0x08,0b000000001); // sampling control (usb , 250fs,  48K, clock div 2, clk out, active)
  out_oldX1 = random32();
  out_oldY1 = random32();
  init_chaos();

  out_oldX1 = 0;
  out_oldX2 = 0;
  out_oldX3 = 0;
  out_oldX4 = 0;
  out_oldY1 = 0;
  out_oldY2 = 0;
  out_oldY3 = 0;
  out_oldY4 = 0;
}

inline void DEJONG_AUDIO_init_() {
  DEJONG_LFO_init_();
}

inline void DEJONG_base_loop_(uint32_t offset) {
  int32_t tmpS, pot3_tmp, CV2_value;
  uint32_t toggle;
  int32_t freq;

  filter16_nozori_84
  test_connect_loop_84();
  toggle = get_toggle();
  toggle_global = toggle;

  chaos(13); // for default mod values

  // clock
  freq = (CV_filter16_out[index_filter_pot1])<<11;
  freq += offset;
  
  macro_FqMod_fine_IN1(pot2)
  macro_fq2increment
  increment_1 = 16*increment1; //save

  if (IN2_connect < 60) CV2_value = (audio_inR>>16) - (IN2_0V>>16); else CV2_value = chaos_dy>>16;
  CV2_value = min(0x7FFF,max(-0x7FFF,CV2_value));

  // Wave Form control 
  pot3_tmp = CV_filter16_out[index_filter_pot3];
  tmpS = CV2_value;
  tmpS *= CV_filter16_out[index_filter_pot4];
  tmpS >>= 16;
  pot3_tmp += tmpS;
  pot3_tmp = min(0xFFFF, max(0,pot3_tmp));
  curve_exp = 0x7FFF - min(pot3_tmp, 0x7FFF); 
  curve_log = max(pot3_tmp, 0x7FFF) - 0x7FFF;

  led2(audio_outL>>23);
  led4(audio_outR>>23);
}

inline void DEJONG_LFO_loop_() {
  //DEJONG_base_loop_(0x02000000);

  int32_t tmpS, pot3_tmp, CV1_value, CV2_value;
  uint32_t toggle;
  int32_t freq;

  filter16_nozori_84
  test_connect_loop_84();
  toggle = get_toggle();
  toggle_global = toggle;

  chaos(15); // for default mod values

  if (IN1_connect < 60) CV1_value = (audio_inL>>16) - (IN1_0V>>16); else CV1_value = chaos_dx>>16;
  CV1_value = min(0x7FFF,max(-0x7FFF,CV1_value));
  if (IN2_connect < 60) CV2_value = (audio_inR>>16) - (IN2_0V>>16); else CV2_value = chaos_dy>>16;
  CV2_value = min(0x7FFF,max(-0x7FFF,CV2_value));

  // clock
  freq = (CV_filter16_out[index_filter_pot1])<<11;
  freq += 0x02000000;

  //macro_FqMod_fine_IN1(pot2)
  tmpS = CV1_value;
  tmpS *= CV_filter16_out[index_filter_pot2];
  tmpS >>= 6;
  freq += tmpS;
    
  macro_fq2increment
  increment_1 = 16*increment1; //save

  // Wave Form control 
  pot3_tmp = CV_filter16_out[index_filter_pot3];
  tmpS = CV2_value;
  tmpS *= CV_filter16_out[index_filter_pot4];
  tmpS >>= 16;
  pot3_tmp += tmpS;
  pot3_tmp = min(0xFFFF, max(0,pot3_tmp));
  curve_exp = 0x7FFF - min(pot3_tmp, 0x7FFF); 
  curve_log = max(pot3_tmp, 0x7FFF) - 0x7FFF;

  led2(audio_outL>>23);
  led4(audio_outR>>23);

  
}

inline void DEJONG_AUDIO_loop_() {
  DEJONG_base_loop_(0x08000000);
}

inline void DEJONG_LFO_audio_() {
  int32_t A, B, C, D, tmpS, outXS, outYS;
  uint32_t toggle, tmp2;
  toggle = toggle_global;
  
  sin1_phase += increment_1;
  
  if (sin1_phase < increment_1) { // depassement, dc nouvelle clock
    out_oldX4 = out_oldX3;
    out_oldX3 = out_oldX2;
    out_oldX2 = out_oldX1;
    out_oldY4 = out_oldY3;
    out_oldY3 = out_oldY2;
    out_oldY2 = out_oldY1;

    A = out_oldY1>>14;
    A *= CV_filter16_out[index_filter_pot5]+0x3000;
    B = out_oldX1>>14;
    B *= CV_filter16_out[index_filter_pot6]+0x3000;
    C = out_oldX1>>14;
    C *= CV_filter16_out[index_filter_pot7]+0x3000;
    D = out_oldY1>>14;
    D *= CV_filter16_out[index_filter_pot8]+0x3000;
    out_oldX1 = (fast_sin(A)>>2) - (fast_sin(B+0xA0000000)>>2);
    out_oldY1 = (fast_sin(C)>>2) - (fast_sin(D+0xA0000000)>>2);
  }

  switch(toggle) { 
  case 0: 
    tmp2 = sin1_phase >> 16;

    tmpS = (out_oldX1>>1) + (out_oldX1>>1);
    tmpS = tmpS >> 16;
    tmpS *= tmp2;
    outXS = (-(out_oldX1>>1) + tmpS)<<2;

    tmpS = (out_oldY1>>1) + (out_oldY1>>1);
    tmpS = tmpS >> 16;
    tmpS *= tmp2;
    outYS = (-(out_oldY1>>1) + tmpS)<<2;
    break;
/*
  case 0: // saw
    tmp2 = sin1_phase >> 16;

    tmpS = (out_oldX1>>1);
    tmpS = tmpS >> 16;
    tmpS *= tmp2;
    outXS = tmpS<<2;

    tmpS = (out_oldY1>>1);
    tmpS = tmpS >> 16;
    tmpS *= tmp2;
    outYS = tmpS<<2;
    break;
    */
  /*
  case 1: // linear interpilation (triangle)
    tmp2 = sin1_phase >> 16;

    tmpS = (out_oldX1>>1) - (out_oldX2>>1);
    tmpS = tmpS >> 16;
    tmpS *= tmp2;
    outXS = ((out_oldX2>>1) + tmpS)<<2;

    tmpS = (out_oldY1>>1) - (out_oldY2>>1);
    tmpS = tmpS >> 16;
    tmpS *= tmp2;
    outYS = ((out_oldY2>>1) + tmpS)<<2;
    break;
  */
  case 1: // no interpolation (square)
    outXS = out_oldX1<<1;
    outYS = out_oldY1<<1;
    break;

  case 2: // cubic interpolation
    tmp2 = sin1_phase >> 16;
    outXS = tabread4(out_oldX4>>18, out_oldX3>>18, out_oldX2>>18, out_oldX1>>18, tmp2)<<17;
    outYS = tabread4(out_oldY4>>18, out_oldY3>>18, out_oldY2>>18, out_oldY1>>18, tmp2)<<17;
    outXS *= 3;
    outYS *= 3; 
    outXS += outXS>>3;
    outYS += outYS>>3;
    break;   
  }
  if (curve_exp > 0) {
    tmpS = outXS >> 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    outXS = MIX16S(outXS>>16, tmpS>>16, curve_exp)<<16;

    tmpS = outYS >> 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    outYS = MIX16S(outYS>>16, tmpS>>16, curve_exp)<<16;

    tmpS = outXS >> 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    outXS = MIX16S(outXS>>16, tmpS>>16, curve_exp)<<16;

    tmpS = outYS >> 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    outYS = MIX16S(outYS>>16, tmpS>>16, curve_exp)<<16;

    tmpS = outXS >> 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    outXS = MIX16S(outXS>>16, tmpS>>16, curve_exp)<<16;

    tmpS = outYS >> 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    outYS = MIX16S(outYS>>16, tmpS>>16, curve_exp)<<16;
  } 
  else { // curve log 
    tmpS = outXS ^0x80000000;
    tmpS >>= 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    tmpS ^= 0x80000000;
    outXS = MIX16S(outXS>>16, tmpS>>16, curve_log)<<16;

    tmpS = outYS ^0x80000000;
    tmpS >>= 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    tmpS ^= 0x80000000;
    outYS = MIX16S(outYS>>16, tmpS>>16, curve_log)<<16;

    tmpS = outXS ^0x80000000;
    tmpS >>= 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    tmpS ^= 0x80000000;
    outXS = MIX16S(outXS>>16, tmpS>>16, curve_log)<<16;

    tmpS = outYS ^0x80000000;
    tmpS >>= 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    tmpS ^= 0x80000000;
    outYS = MIX16S(outYS>>16, tmpS>>16, curve_log)<<16;

    tmpS = outXS ^0x80000000;
    tmpS >>= 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    tmpS ^= 0x80000000;
    outXS = MIX16S(outXS>>16, tmpS>>16, curve_log)<<16;

    tmpS = outYS ^0x80000000;
    tmpS >>= 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    tmpS ^= 0x80000000;
    outYS = MIX16S(outYS>>16, tmpS>>16, curve_log)<<16;
  }
  outXS -= outXS>>2;
  outYS -= outYS>>2;  
  audio_outL=outXS^0x80000000;
  audio_outR=outYS^0x80000000;
}

inline void DEJONG_AUDIO_audio_() {
  DEJONG_LFO_audio_();
}
