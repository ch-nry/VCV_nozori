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

// VCO with control of the WF : square to saw to double saw and PWM control

// Pot 1 : FQ
// Pot 2 : Mod Fq (or fine if nothing is connected on the modulation)
// Pot 3 : WF
// Pot 4 : MOD WF (chaos if nothing is connected)
// Pot 5 : PWM
// Pot 6 : Mod PWM (chaos if nothing is connected)
// CV 1 : Pitch (1V/Oct)
// CV 2 : Mod Fq (1V/Oct a full)
// CV 3 : Mod WF
// CV 4 : Mod PWM
// IN 1 : GAIN (Exp)
// IN 2 : Pan
// Selecteur3 : Wave Form : Square / Double Saw / sin
// OUT 1 : OUT L
// OUT 2 : OUT R
// LED 1 : CV2
// LED 2 : CV3

uint32_t pot3_save, pot5_save, pot3_data, pot5_data;

inline void VCO_Param_init_() {
  VCO1_phase = 0;
  init_chaos();
  pot3_save = 0;
  pot5_save = 0;
  freq_save = 0x1000;
  increment_0 = 0x1000;
}

inline void VCO_Param_loop_() {
  int32_t pot5_tmp, pot3_tmp; 
  int32_t tmpS;
  int32_t CV3_value, CV4_value;

  filter16_nozori_68
  test_connect_loop_68();
  chaos(15); // for default mod values

  if (CV3_connect < 60) CV3_value = CV_filter16_out[index_filter_cv3] - CV3_0V; else CV3_value = chaos_dx>>16;
  if (CV4_connect < 60) CV4_value = CV_filter16_out[index_filter_cv4] - CV4_0V; else CV4_value = chaos_dy>>16;
  CV3_value = min(0x7FFF,max(-0x7FFF,CV3_value));
  CV4_value = min(0x7FFF,max(-0x7FFF,CV4_value));

  // frequency control
  macro_fq_in_tlg
  macro_1VOct_CV1
  macro_FqMod_fine(pot2,CV2)
  macro_fq2increment
  increment_0 = increment1;
  
  // Wave Form control 
  pot3_tmp = CV_filter16_out[index_filter_pot3];
  tmpS = CV3_value;
  tmpS *= CV_filter16_out[index_filter_pot4];
  tmpS >>= 16;
  pot3_tmp += tmpS;
  pot3_tmp = min(0xFFFF, max(0,pot3_tmp));
  pot3_data = pot3_tmp;

  // PWM
  pot5_tmp = CV_filter16_out[index_filter_pot5];
  tmpS = CV4_value;
  tmpS *= CV_filter16_out[index_filter_pot6];
  tmpS >>= 16;
  pot5_tmp += tmpS;
  pot5_tmp = min(0xFFFF, max(0,pot5_tmp));
  pot5_data = pot5_tmp;

  led2((CV3_value+0x7FFF)>>7);
  //led3((chaos_dz^0x80000000)>>23);
  led4((CV4_value+0x7FFF)>>7);
}

inline void VCO_Param_audio_() {
  int32_t tmp, tmp1, tmp2, tmp4;
  uint32_t VCO1_WS, VCO1_PWM, VCO1_offset, VCO1_gain, VCO1_BLsize;
  uint32_t VCO2_BLsize;
  uint32_t increment1;
  uint32_t pot5_tmp, pot3_tmp; 
  uint32_t out, out2;
  
  increment1 = increment_0;
  
  //Waveform control
  pot3_tmp = filter(pot3_data, pot3_save, 6);
  pot3_save = pot3_tmp;
  pot5_tmp = filter(pot5_data, pot5_save, 6);
  pot5_save = pot5_tmp;
  pot5_tmp = min(pot5_tmp, 0xFFFF-(min(0xFFFF,increment1>>10))); // limit du PWM en fct de la frequence pour ne pas avoir d'impulsion trop petite
  VCO1_PWM = (0xFFFF-(pot5_tmp))<<15;
  //VCO1_PWM = max(VCO1_PWM, increment1<<6);

  VCO1_WS = (pot3_tmp)<<16;
    
  tmp = pot3_tmp;
  tmp = (tmp > (1<<15))? 0:(1<<15)-tmp; // 12 bits
  tmp >>= 3; // 12 bits
  VCO1_offset = (pot5_tmp) * 3 * tmp;

  tmp = (pot3_tmp < (1<<15))? 0 : pot3_tmp - (1<<15);
  tmp *= pot5_tmp;
  VCO1_gain = 0x7FFFFFFF - (tmp>>1);

  // compute output  
  VCO1_BLsize = (0xFFFFFFFF / increment1)<<4;   

  VCO1_phase += increment1<<3;
  tmp1 = sawBL(VCO1_phase, VCO1_BLsize)>>1;
  tmp1 -= 0x40000000;

  tmp2 = sawBL(VCO1_phase + VCO1_PWM, VCO1_BLsize)>>1;
  tmp2 -= 0x40000000;

  _m_s32xs32_s32(tmp2, VCO1_WS>>1, tmp4, tmp); 
  tmp1 += tmp4<<2;
  tmp1 -= tmp2;
  
  tmp1 += VCO1_offset;
  _m_s32xs32_s32(tmp1, VCO1_gain, tmp1, tmp2);

  tmp1 <<= 2;
  out2 = tmp1^0x80000000;

  /////////////////////////////////////////////////////////////////////
  // 2eme oscillateur
  VCO2_BLsize = (0xFFFFFFFF / increment1)<<5;   

  VCO2_phase += increment1<<2;
  tmp1 = sawBL(VCO2_phase, VCO2_BLsize)>>1;
  tmp1 -= 0x40000000;
  
  tmp2 = sawBL(VCO2_phase + VCO1_PWM, VCO2_BLsize)>>1;
  tmp2 -= 0x40000000;

  _m_s32xs32_s32(tmp2, VCO1_WS>>1, tmp4, tmp); 
  tmp1 += tmp4<<2;
  tmp1 -= tmp2;
  
  tmp1 += VCO1_offset;
  _m_s32xs32_s32(tmp1, VCO1_gain, tmp1, tmp2);

  tmp1 <<= 2;
  out = tmp1^0x80000000;

  macro_out_gain_pan_stereo
}
