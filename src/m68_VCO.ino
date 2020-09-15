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

// standard VCO with 2 modulation and 3 waveform (rectangle / double saw / sin) and PWM
// Pot 1 : FQ
// Pot 2 : Mod Fq (or fine if nothing is connected on the modulation)
// Pot 3 : MOD Fq
// Pot 4 : MOD Fq
// Pot 5 : PWM
// Pot 6 : Mod PWM
// CV 1 : Mod Fq (1V/Oct a full)
// CV 2 : Mod Fq (1V/Oct a full)
// CV 3 : Mod Fp (1V/Oct a full)
// CV 4 : Mod PWM
// IN 1 : GAIN (Exp)
// IN 2 : Pan
// Selecteur3 : Wave Form : Square / Double Saw / sin
// OUT 1 : OUT L
// OUT 2 : OUT R

inline void VCO_init_() {
  VCO1_phase = 0;
  init_chaos();
  freq_save = 0x1000;
}

inline void VCO_loop_() {
  int32_t CV2_value, CV3_value, CV4_value;
  int32_t tmpS, pot5_tmp;

  filter16_nozori_68
  test_connect_loop_68();
  chaos(15); // for default mod values
  toggle_global = get_toggle();

  if (CV2_connect < 60) CV2_value = CV_filter16_out[index_filter_cv2] - CV2_0V; else CV2_value = chaos_dx>>17;
  if (CV3_connect < 60) CV3_value = CV_filter16_out[index_filter_cv3] - CV3_0V; else CV3_value = ((chaos_dy*5)>>16);
  if (CV4_connect < 60) CV4_value = CV_filter16_out[index_filter_cv4] - CV4_0V; else CV4_value = chaos_dz>>16;
  CV2_value = min(0x7FFF,max(-0x7FFF,CV2_value));
  CV3_value = min(0x7FFF,max(-0x7FFF,CV3_value));
  CV4_value = min(0x7FFF,max(-0x7FFF,CV4_value));
  
  led2((CV2_value+0x7FFF)>>7);
  led4((CV3_value+0x7FFF)>>7);
  
  // frequency control
  macro_fq_in
  macro_1VOct_CV1
  macro_FqMod_fine_simple(pot2)
  macro_FqMod_value(pot3,CV2, CV2_value)
  macro_FqMod_value(pot4,CV3, CV3_value)
  macro_fq2increment
  freq_save = increment1<<1;

  //Waveform control
  pot5_tmp = CV_filter16_out[index_filter_pot5];
  tmpS = CV4_value;
  tmpS *= CV_filter16_out[index_filter_pot6];
  tmpS >>= 16;
  pot5_tmp += tmpS;
  pot5_tmp = min(0xFFFF, max(0,pot5_tmp));
  pwm_save = pot5_tmp;
}

inline void VCO_audio_() {
  uint32_t tmp1, tmp2;
  int32_t tmpS, tmpS2;
  uint32_t VCO1_PWM, VCO1_BLsize, PWM_tmp;
  int32_t pot5_tmp; 
  uint32_t out, out2;
  uint32_t increment1;
  uint32_t phase_local;
  
  increment1 = freq_save; // from the data loop

  pot5_tmp = min(pwm_save, 0xFFFF-(min(0xFFFF,increment1>>10))); // limit du PWM en fct de la frequence pour ne pas avoir d'impulsion trop petite

  pot5_tmp = filter(pot5_tmp<<8, VCO1_PWM_save, 6);
  VCO1_PWM_save = pot5_tmp;
  PWM_tmp = pot5_tmp<<8;
  VCO1_PWM = (0xFFFF00-(pot5_tmp))<<7;

  VCO1_phase += increment1<<3;
  phase_local = VCO1_phase;
  
  // compute output
  switch(toggle_global) {     
  case 0 : // rectangle 
    //VCO1_PWM = max(VCO1_PWM, 0x04000000);
    VCO1_BLsize = (0xFFFFFFFF / increment1)<<4;
    tmpS = sawBL(VCO1_phase, VCO1_BLsize) ^ 0x80000000;
    tmpS2 = sawBL(VCO1_phase + VCO1_PWM, VCO1_BLsize) ^ 0x80000000;
    tmpS -= tmpS2;
    tmpS += 3*(PWM_tmp>>3);
    out = tmpS^0x80000000;
    
    VCO1_BLsize >>= 1;
    tmpS = sawBL(VCO1_phase<<1, VCO1_BLsize) ^ 0x80000000;
    tmpS2 = sawBL((VCO1_phase<<1) + VCO1_PWM, VCO1_BLsize) ^ 0x80000000;
    tmpS -= tmpS2;
    tmpS += 3*(PWM_tmp>>3);
    out2 = tmpS^0x80000000;
    
  break;
  case 1 : // double saw
    uint32_t phase2_local, pot5_local;
    phase2_local = phase_local<<1;
    pot5_local = (pot5_tmp<<7);
    
    VCO1_BLsize = (0xFFFFFFFF / increment1)<<4;   
    tmpS = ((sawBL(phase_local, VCO1_BLsize)>>1) + (sawBL(phase_local + pot5_local, VCO1_BLsize)>>1));
    tmpS ^= 0x80000000;
    tmpS = m_s32xs32_s32H(tmpS, (pot5_tmp<<6) + (1<<30));
    //tmpS = (tmpS>>16) * (((pot5_tmp<<6) + (1<<30))>>16);
    tmpS <<= 2;
    out = tmpS^0x80000000;

    VCO1_BLsize >>= 1;   
    tmpS = ((sawBL(phase2_local, VCO1_BLsize)>>1) + (sawBL((phase2_local) + pot5_local, VCO1_BLsize)>>1));
    tmpS ^= 0x80000000;
    tmpS = m_s32xs32_s32H(tmpS, (pot5_tmp<<6) + (1<<30));
    //tmpS = (tmpS>>16) * (((pot5_tmp<<6) + (1<<30))>>16);
    tmpS <<= 2;
    out2 = tmpS^0x80000000;

  break;
  case 2 : // sin
    tmp1 = VCO1_phase;

    tmp2 = fast_sin(0xC0000000 + (tmp1>>1));
    tmpS = tmp2-tmp1;
    tmpS = (PWM_tmp>>16) * (tmpS >> 16);
    tmpS = tmp1 - tmpS;
    tmp1 = tmpS;
    
    tmp2 = fast_sin(0xC0000000 + (tmp1>>1));
    tmpS = tmp2-tmp1;
    tmpS = (PWM_tmp>>16) * (tmpS >> 16);
    tmpS = tmp1 - tmpS;
    tmp1 = tmpS;
    
    tmp2 = fast_sin(0xC0000000 + (tmp1>>1));
    tmpS = tmp2-tmp1;
    tmpS = (PWM_tmp>>16) * (tmpS >> 16);
    tmpS = tmp1 - tmpS;
    tmp1 = tmpS;
    out = fast_sin(0x40000000 + tmp1);
    out -= out>>2;
    out += 0x1FFFFFFF;

    tmp1 = VCO1_phase<<1;

    tmp2 = fast_sin(0xC0000000 + (tmp1>>1));
    tmpS = tmp2-tmp1;
    tmpS = (PWM_tmp>>16) * (tmpS >> 16);
    tmpS = tmp1 - tmpS;
    tmp1 = tmpS;
    
    tmp2 = fast_sin(0xC0000000 + (tmp1>>1));
    tmpS = tmp2-tmp1;
    tmpS = (PWM_tmp>>16) * (tmpS >> 16);
    tmpS = tmp1 - tmpS;
    tmp1 = tmpS;
    
    tmp2 = fast_sin(0xC0000000 + (tmp1>>1));
    tmpS = tmp2-tmp1;
    tmpS = (PWM_tmp>>16) * (tmpS >> 16);
    tmpS = tmp1 - tmpS;
    tmp1 = tmpS;
    out2 = fast_sin(0x40000000 + tmp1);
    out2 -= out2>>2;
    out2 += 0x1FFFFFFF;  
  break;
  }
  macro_out_gain_pan_stereo
}
