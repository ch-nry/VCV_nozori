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

// VCF with continious control of the frequency responce, and a VCQ
// Pot 1 : FQ
// Pot 2 : Mod Fq (or fine if nothing is connected on the modulation)
// Pot 3 : Mod Fq
// Pot 4 : Mod Fq
// Pot 5 : Q
// Pot 6 : Mod Q
// CV 1 : Mod Fq (1V/Oct a full)
// CV 2 : Mod Fq (1V/Oct a full)
// CV 3 : Mod Fq (1V/Oct a full)
// CV 4 : Mod Q
// IN 1 : IN1
// IN 2 : IN2
// Selecteur : filter Mod: LP/BP/HP
// OUT 1 : OUT 1 
// OUT 2 : OUT 2 

inline void VCF_init_() {
  in1_oldL = 0;
  in2_oldL = 0;
  in3_oldL = 0;
  in4_oldL = 0;
  in5_oldL = 0;

  in1_oldR = 0;
  in2_oldR = 0;
  in3_oldR = 0;
  in4_oldR = 0;
  in5_oldR = 0;
  
  init_chaos();

  default5(0);
}

inline void VCF_loop_() {
  uint32_t toggle;
  int32_t tmpS;
  int32_t reso_local;
  int32_t CV2_value, CV3_value, CV4_value;

  filter16_nozori_68
  //test_connect_loop_68();
  
  test_connect_loop_start
  test_connect_loop_test_cv1
  test_connect_loop_test_cv2
  test_connect_loop_test_cv3
  test_connect_loop_test_cv4
  test_connect_loop_test_in2
  test_connect_loop_test_end
  //default5(0);

  toggle = get_toggle();
  toggle_global = toggle;
  chaos(13); // for default mod values
  
  // Default modulation
  //if (CV1_connect < 60) CV1_value = CV_filter16_out[index_filter_cv1] - CV1_0V; else CV1_value = 0;
  if (CV2_connect < 60) CV2_value = CV_filter16_out[index_filter_cv2] - CV2_0V; else CV2_value = chaos_dx>>16;
  if (CV3_connect < 60) CV3_value = CV_filter16_out[index_filter_cv3] - CV3_0V; else CV3_value = ((chaos_dy*3)>>16);
  if (CV4_connect < 60) CV4_value = CV_filter16_out[index_filter_cv4] - CV4_0V; else CV4_value = chaos_dz>>16;
  CV2_value = min(0x7FFF,max(-0x7FFF,CV2_value));
  CV3_value = min(0x7FFF,max(-0x7FFF,CV3_value));
  CV4_value = min(0x7FFF,max(-0x7FFF,CV4_value));
  
  // Frequency
  macro_fq_in
  freq +=  1<<25;
  macro_1VOct_CV1
  macro_FqMod_fine_simple(pot2)
  
  tmpS = CV2_value;
  tmpS *= min(4086<<4,CV_filter16_out[index_filter_pot3]);
  tmpS /= (4086<<4);
  tmpS *= CV2_1V;
  freq += tmpS;
  
  tmpS = CV3_value;
  tmpS *= min(4086<<4,CV_filter16_out[index_filter_pot4]);
  tmpS /= (4086<<4);
  tmpS *= CV3_1V;
  freq += tmpS;

  macro_fq2cutoff
  freq_global = increment1;

  // resonnance
  reso_local = CV_filter16_out[index_filter_pot5];
  tmpS = CV4_value;
  tmpS *= CV_filter16_out[index_filter_pot6]>>1;
  tmpS >>= 15;
  reso_local += tmpS;
  reso_local *= 18000;
  reso_local = min(0x46500000,max(0,reso_local));
  reso = reso_local;
 
  // filter shape
  switch (toggle) {
    case 2 :
      G1 = 0;
      G2 = 0;
      G3 = 0;
      G5 = 6;
    break;
    case 1 :
      G1 = 0;
      G2 = -4;
      G3 = 4;
      G5 = 0;   
    break;
    case 0 :
      G1 = 2;
      G2 = -4;
      G3 = 2;
      G5 = 0;
    break;
  }

  led2((CV2_value+0x7FFF)>>7);
  led4((CV3_value+0x7FFF)>>7);
}

inline void VCF_audio_() {
  int32_t in, tmp, tmp1, VCF_out, inR;
  int32_t freq;

  // Left
  ////////////////////////////////////////////////////////////////////////////////////
  freq = freq_global;
  
  //if (IN1_connect < 60) in = (audio_inL^0x80000000); else in = 0; 
  in = (audio_inL^0x80000000);
  
  inR = in;
  in >>= 4; // 28 bits

  // feedback amplitude
  tmp = in5_oldL - (in >> 1) ;
  tmp = m_s32xs32_s32H(tmp, reso);
  tmp <<= 1; // pour compenser le gain du au sign
  tmp *= -4;
   
  // distortion
  tmp1 = tmp;
  tmp = m_s32xs32_s32H(tmp1, abs(tmp1));
  tmp = tmp1 - (tmp<<4);
  tmp += tmp >> 2;
  tmp += tmp1;

  // freedback for resonnance
  //tmp += (tmp>>2) + (tmp>>3); // a bit more gain
  tmp += in;

  tmp1 = tmp;
  // LOP 1
  tmp = (tmp >> 8) * 197;  
  tmp += (in1_oldL >> 8) * 59;
  in1_oldL = tmp1;
  tmp = m_s32xs32_s32H(tmp-in2_oldL, freq);
  tmp <<= 2;
  tmp += in2_oldL;
  tmp1 = tmp; 
  // LOP 2
  tmp = (tmp >> 8) * 197;  
  tmp += (in2_oldL >> 8) * 59;
  in2_oldL = tmp1;
  tmp = m_s32xs32_s32H(tmp-in3_oldL, freq);
  tmp <<= 2;
  tmp += in3_oldL;
  tmp1 = tmp; 
  // LOP 3
  tmp = (tmp >> 8) * 197;  
  tmp += (in3_oldL >> 8) * 59;
  in3_oldL = tmp1;
  tmp = m_s32xs32_s32H(tmp-in4_oldL, freq);
  tmp <<= 2;
  tmp += in4_oldL;
  tmp1 = tmp; 
  // LOP 4
  tmp = (tmp >> 8) * 197;  
  tmp += (in4_oldL >> 8) * 59;
  in4_oldL = tmp1;
  tmp = m_s32xs32_s32H(tmp-in5_oldL, freq);
  tmp <<= 2;
  tmp += in5_oldL;
  in5_oldL = tmp;
  
  VCF_out = G1*in1_oldL + G2*in2_oldL + G3*in3_oldL + G5*in5_oldL; // G4 is not used with this filter selection
  saturate_S31(VCF_out, VCF_out);
  VCF_out <<= 1;
  audio_outL = VCF_out^0x80000000;

   // Right
  ////////////////////////////////////////////////////////////////////////////////////
  if (IN2_connect < 60) in = (audio_inR^0x80000000); 
  else in = inR;
  in >>= 4; // 28 bits
  
  // feedback
  tmp = in5_oldR - (in >> 1) ;
  tmp = m_s32xs32_s32H(tmp, reso);
  tmp <<= 1; // pour compenser le gain du au sign
  tmp *= -4;
    
  // distortion
  tmp1 = tmp;
  tmp = m_s32xs32_s32H(tmp1, abs(tmp1));
  tmp = tmp1 - (tmp<<4);
  tmp += tmp >> 2;
  tmp += tmp1;
  
  //tmp += (tmp>>2) + (tmp>>3); // a bit more gain
  tmp += in;

  tmp1 = tmp;
  // LOP 1
  tmp = (tmp >> 8) * 197;  
  tmp += (in1_oldR >> 8) * 59;
  in1_oldR = tmp1;
  tmp = m_s32xs32_s32H(tmp-in2_oldR, freq);
  tmp <<= 2;
  tmp += in2_oldR;
  tmp1 = tmp; 
  // LOP 2
  tmp = (tmp >> 8) * 197;  
  tmp += (in2_oldR >> 8) * 59;
  in2_oldR = tmp1;
  tmp = m_s32xs32_s32H(tmp-in3_oldR, freq);
  tmp <<= 2;
  tmp += in3_oldR;
  tmp1 = tmp; 
  // LOP 3
  tmp = (tmp >> 8) * 197;  
  tmp += (in3_oldR >> 8) * 59;
  in3_oldR = tmp1;
  tmp = m_s32xs32_s32H(tmp-in4_oldR, freq);
  tmp <<= 2;
  tmp += in4_oldR;
  tmp1 = tmp; 
  // LOP 4
  tmp = (tmp >> 8) * 197;  
  tmp += (in4_oldR >> 8) * 59;
  in4_oldR = tmp1;
  tmp = m_s32xs32_s32H(tmp-in5_oldR, freq);
  tmp <<= 2;
  tmp += in5_oldR;
  in5_oldR = tmp;
  
  VCF_out = G1*in1_oldR + G2*in2_oldR + G3*in3_oldR + G5*in5_oldR; // G4 is not used with this filter selection
  saturate_S31(VCF_out, VCF_out);
  VCF_out <<= 1;
  audio_outR = VCF_out^0x80000000;
}


