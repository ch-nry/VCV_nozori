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

// sinus oscillator with a waveshapper
// Pot 1 : FQ
// Pot 2 : Mod Fq (or fine if nothing is connected on the modulation)
// Pot 3 : WS
// Pot 4 : MOD WS
// Pot 5 : Phase
// Pot 6 : Mod Phase
// CV 1 : Pitch (1V/Oct
// CV 2 : Mod Fq
// CV 3 : Mod WS
// CV 4 : Mod Phase
// IN 1 : GAIN (Exp)
// IN 2 : Pan
// Selecteur3 : FQ Range
// OUT 1 : OUT L
// OUT 2 : OUT R

int32_t WS_save, PM_save;

inline void VCO_WS_init_() {
  VCO1_phase = 0;
  init_chaos();
  WS_save = 0;
  PM_save = 0;
  freq_save = 0x1000;
}

inline void VCO_WS_loop_() {
  int32_t tmpS;
  
  filter16_nozori_68
  test_connect_loop_68();
  chaos(15); // for default modulation values

  // frequency control 
  macro_fq_in_tlg
  macro_1VOct_CV1
  macro_FqMod_fine(pot2,CV2)
  macro_fq2increment
  increment_1 = increment1;
}

inline void VCO_WS_audio_() {
  int32_t tmpS, tmp1, tmp2;
  int32_t WS, PM;
  int32_t CV3_value, CV4_value, pot3_tmp, pot5_tmp;
  uint32_t out, out2;

  // Parametters controls
  if (CV3_connect < 60) CV3_value = CV_filter16_out[index_filter_cv3] - CV3_0V; else CV3_value = chaos_dx>>16;
  if (CV4_connect < 60) CV4_value = CV_filter16_out[index_filter_cv4] - CV4_0V; else CV4_value = chaos_dy>>16;
  CV3_value = min(0x7FFF,max(-0x7FFF,CV3_value));
  CV4_value = min(0x7FFF,max(-0x7FFF,CV4_value));

  led2((CV3_value+0x7FFF)>>7);
  led4((CV4_value+0x7FFF)>>7);
  
  pot3_tmp = CV_filter16_out[index_filter_pot3]<<14;
  tmpS = CV3_value;
  tmpS *= CV_filter16_out[index_filter_pot4];
  tmpS >>= 2;
  pot3_tmp += tmpS;
  pot3_tmp = max(0,pot3_tmp);
  WS = filter(pot3_tmp, WS_save, 7); 
  WS_save = WS;

  pot5_tmp = CV_filter16_out[index_filter_pot5]<<13;
  tmpS = CV4_value;
  tmpS *= CV_filter16_out[index_filter_pot6];
  tmpS >>= 3;
  pot5_tmp += tmpS;
  PM = filter(pot5_tmp, PM_save, 7); 
  PM_save = PM;

  // compute output  
  VCO1_phase += increment_1<<1;  
  tmp1 = fast_sin(VCO1_phase);
  tmp1 ^= 0x80000000;
  tmp2 = WS+0x04000000;
  _m_s32xs32_s32(tmp1, tmp2, tmp1, tmp2);
  tmp1 ^= 0x80000000;
  
  out = fast_sin((tmp1<<4)+(PM<<2));
  out -= out >> 2;
  out += 1<<29;

  // compute output2
  tmp1 = fast_sin(VCO1_phase);
  tmp1 ^= 0x80000000;
  tmp2 = WS+0x04000000;
  _m_s32xs32_s32(tmp1, tmp2, tmp1, tmp2);
  tmp1 ^= 0x80000000;

  out2 = fast_sin((tmp1<<4)+(PM<<2)+0X30000000);
  out2 -= out2 >> 2;
  out2 += 1<<29;

  macro_out_gain_pan_stereo2
}



