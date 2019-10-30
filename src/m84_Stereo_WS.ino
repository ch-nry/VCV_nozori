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

// dual waveshaper based on 2 iteration of peter de jong equation
// To be used as LFO or audio source
// Pot 1 : cos 1
// Pot 2 : cos 2
// Pot 3 : cos 3
// Pot 4 : cos 4
// Pot 5 : lfo 1
// Pot 6 : lfo 2
// Pot 7 : lfo 3
// Pot 8 : lfo 4
// IN 1 : audio 1
// IN 2 : ausio 2
// Selecteur3 : WS type
// OUT 1 : OUT X
// OUT 2 : OUT Y

uint32_t curve2_exp, curve2_log;

inline void Stereo_WS_init_() {
    send_dac(0x08,0b000000001); // sampling control (usb , 250fs,  48K, clock div 2, clk out, active)
}

inline void Stereo_WS_loop_() {
  uint32_t toggle, tmp1, tmp2;
  
  filter16_nozori_84
  test_connect_loop_84();
  toggle = get_toggle();
  toggle_global = toggle;

  tmp1 = (fast_sin(sin1_phase) >> 16);
  tmp2 = (fast_sin(sin2_phase) >> 16);

  led2((tmp1 >> 7));
  led4((tmp2 >> 7));

    // Wave Form control 
  curve_exp = 0x7FFF - min(CV_filter16_out[index_filter_pot7], 0x7FFF); 
  curve_log = max(CV_filter16_out[index_filter_pot7], 0x7FFF) - 0x7FFF;
  curve2_exp = 0x7FFF - min(CV_filter16_out[index_filter_pot8], 0x7FFF); 
  curve2_log = max(CV_filter16_out[index_filter_pot8], 0x7FFF) - 0x7FFF;
}

inline void Stereo_WS_audio_() {
  int32_t A, B, C, D;
  int32_t potA, potB, potC, potD;
  int32_t sin1, sin2;
  uint32_t toggle;
  int32_t tmpS;
  int32_t sin3, outXS, outYS;
  uint32_t curve_exp_local, curve_log_local, curve_exp2_local, curve_log2_local;
  
  toggle = toggle_global;
  curve_exp_local = curve_exp;
  curve_log_local = curve_log;
  curve_exp2_local = curve2_exp;
  curve_log2_local = curve2_log;
  
  sin1_phase += (CV_filter16_out[index_filter_pot5] * CV_filter16_out[index_filter_pot5])>>12;
  sin2_phase += (CV_filter16_out[index_filter_pot6] * CV_filter16_out[index_filter_pot6])>>12;
  //sin3_phase += (CV_filter16_out[index_filter_pot7] * CV_filter16_out[index_filter_pot7])>>10;
  //sin4_phase += (CV_filter16_out[index_filter_pot8] * CV_filter16_out[index_filter_pot8])>>10;

  potA = filter(CV_filter16_out[index_filter_pot3]<<14, potA_save, 9);
  potA_save = potA;
  potB = filter(CV_filter16_out[index_filter_pot1]<<14, potB_save, 9);
  potB_save = potB;
  potC = filter(CV_filter16_out[index_filter_pot2]<<14, potC_save, 9);
  potC_save = potC;
  potD = filter(CV_filter16_out[index_filter_pot4]<<14, potD_save, 9);
  potD_save = potD;
  potA >>= 14;
  potB >>= 14;
  potC >>= 14;
  potD >>= 14;

  increment_0 +=19685267>>2; // 110Hz 
  tmpS = fast_sin(increment_0)^0x80000000;
  tmpS -= tmpS >> 2;
  sin3 = tmpS;

  if (IN2_connect < 60) sin2 = audio_inR - IN2_0V; else sin2 = sin3;
  if (IN1_connect < 60) sin1 = audio_inL - IN1_0V; else sin1 = sin3;

  A = sin2>>14;
  A *= potA;
  B = sin1>>14;
  B *= potB;
  C = sin1>>14;
  C *= potC;
  D = sin2>>14;
  D *= potD;
  
  A += sin1_phase;
  B += sin2_phase;
  C -= sin1_phase;
  D -= sin2_phase;

  //audio_outL = sin1^0x80000000;
  
  sin1 = (fast_sin(A)>>1) - (fast_sin(B+0xC0000000)>>1);
  sin2 = (fast_sin(C)>>1) - (fast_sin(D+0xC0000000)>>1);
  
  A = sin2>>14;
  A *= potA;
  B = sin1>>14;
  B *= potB;
  C = sin1>>14;
  C *= potC;
  D = sin2>>14;
  D *= potD;

  switch (toggle) {
    case 0:
      sin1 = (fast_sin(A<<1)>>1) - (fast_sin((B<<1)+0xC0000000)>>1);
      sin2 = (fast_sin(C<<1)>>1) - (fast_sin((D<<1)+0xC0000000)>>1);
    break;
    case 1:
      sin1 = (fast_sin(A)>>1) - (fast_sin(B+0xC0000000)>>1);
      sin2 = (fast_sin(C)>>1) - (fast_sin(D+0xC0000000)>>1);
    break;
  }

  outXS = sin1;
  outYS = sin2;

  if (curve_exp_local > 0) {
    tmpS = outXS >> 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    outXS = MIX16S(outXS>>16, tmpS>>16, curve_exp_local)<<16;

    tmpS = outXS >> 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    outXS = MIX16S(outXS>>16, tmpS>>16, curve_exp_local)<<16;

    tmpS = outXS >> 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    outXS = MIX16S(outXS>>16, tmpS>>16, curve_exp_local)<<16;
  } 
  else { // curve log 
    tmpS = outXS ^0x80000000;
    tmpS >>= 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    tmpS ^= 0x80000000;
    outXS = MIX16S(outXS>>16, tmpS>>16, curve_log_local)<<16;

    tmpS = outXS ^0x80000000;
    tmpS >>= 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    tmpS ^= 0x80000000;
    outXS = MIX16S(outXS>>16, tmpS>>16, curve_log_local)<<16;

    tmpS = outXS ^0x80000000;
    tmpS >>= 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    tmpS ^= 0x80000000;
    outXS = MIX16S(outXS>>16, tmpS>>16, curve_log_local)<<16;
  }

  if (curve_exp2_local > 0) {
    tmpS = outYS >> 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    outYS = MIX16S(outYS>>16, tmpS>>16, curve_exp2_local)<<16;

    tmpS = outYS >> 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    outYS = MIX16S(outYS>>16, tmpS>>16, curve_exp2_local)<<16;

    tmpS = outYS >> 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    outYS = MIX16S(outYS>>16, tmpS>>16, curve_exp2_local)<<16;
  } 
  else { // curve log 
    tmpS = outYS ^0x80000000;
    tmpS >>= 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    tmpS ^= 0x80000000;
    outYS = MIX16S(outYS>>16, tmpS>>16, curve_log2_local)<<16;

    tmpS = outYS ^0x80000000;
    tmpS >>= 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    tmpS ^= 0x80000000;
    outYS = MIX16S(outYS>>16, tmpS>>16, curve_log2_local)<<16;
    
    tmpS = outYS ^0x80000000;
    tmpS >>= 16;
    tmpS *= abs(tmpS);
    tmpS >>= 15;
    tmpS *= abs(tmpS);
    tmpS <<= 1;
    tmpS ^= 0x80000000;
    outYS = MIX16S(outYS>>16, tmpS>>16, curve_log2_local)<<16;
  }

  outXS -= outXS>>2;
  outYS -= outYS>>2;  
  audio_outL=outXS^0x80000000;
  audio_outR=outYS^0x80000000;
}

