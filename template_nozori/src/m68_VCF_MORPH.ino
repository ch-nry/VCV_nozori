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

// VCF with continious control of the frequency responce (LP/BP/HP), and a VCQ
// Pot 1 : FQ
// Pot 2 : Mod Fq (or fine if nothing is connected on the modulation)
// Pot 3 : Q
// Pot 4 : MOD Q
// Pot 5 : Shape (LP24/L12/BP12/HP12)
// Pot 6 : Mod Shape
// CV 1 : Pitch (1V/Oct)
// CV 2 : Mod Fq (1V/Oct a full)
// CV 3 : Mod Q
// CV 4 : Mod Shape
// IN 1 : IN1
// IN 2 : IN2
// Selecteur : filter Mod: dual (2 diferent filter), opsosit (2 difent filter with modulation acting in an oposit way), pan (1 filter with a panoramic control of the sound)
// OUT 1 : OUT 1 / OUT L
// OUT 2 : OUT 2 / OUT R

uint32_t in_gain;

inline void VCF_Param_init_() {
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

inline void VCF_Param_loop_() {
  int32_t tmpS;
  int32_t freq2, fader, fader2, faderA, faderB, faderC, faderD;
  int32_t reso_local, reso2_local;
  uint32_t toggle;
  int32_t CV2_value, CV3_value, CV4_value;
  int32_t G1_tmp, G2_tmp, G3_tmp, G5_tmp;
  uint32_t gain;

  filter16_nozori_68
  //test_connect_loop_68();
  chaos(13); // for default mod values
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

  // Default modulation
  if (CV3_connect < 60) CV3_value = CV_filter16_out[index_filter_cv3] - CV3_0V; else CV3_value = chaos_dx>>16;
  if (CV4_connect < 60) CV4_value = CV_filter16_out[index_filter_cv4] - CV4_0V; else CV4_value = chaos_dy>>16;
  CV3_value = min(0x7FFF,max(-0x7FFF,CV3_value));
  CV4_value = min(0x7FFF,max(-0x7FFF,CV4_value));
  
  // Frequency
  macro_fq_in
  freq +=  1<<25;
  macro_1VOct_CV1
  //macro_FqMod_fine(pot2,CV2) // a cause de freq 2 pour le mode "oposit"
  if (CV2_connect < 60) {
    freq2 = freq;
    CV2_value = CV_filter16_out[index_filter_cv2] - CV2_0V;;
    CV2_value = min(0x7FFF,max(-0x7FFF,CV2_value));
    tmpS = CV2_value * min(4092<<3,CV_filter16_out[index_filter_pot2]>>1);
    tmpS /= (4092<<3);
    tmpS *= CV2_1V;
    freq += tmpS;
  }
  else { //fine tune
    tmpS = 0;
    freq += CV_filter16_out[index_filter_pot2]*(48<<2);
    freq2 = freq;
  }
  macro_fq2cutoff
  freq_global = increment1;
  
  if(toggle == 1) { // Mod oposite
    freq = freq2;
    freq -= tmpS;
    macro_fq2cutoff_novar
  }   
  freq_global2 = increment1;

  // resonnance
  reso_local = CV_filter16_out[index_filter_pot3];
  tmpS = CV3_value;
  tmpS *= CV_filter16_out[index_filter_pot4]>>1;
  tmpS >>= 15;
  reso2_local = reso_local; // save for oposit mode
  reso_local += tmpS;
  reso_local *= 18000;
  reso_local = min(0x46500000,max(0,reso_local));
  reso = reso_local;
  if(toggle==1) { // mode oposite
    reso2_local -= tmpS;
    reso2_local *= 18000;
    reso2_local = min(0x46500000,max(0,reso2_local));
    reso2 = reso2_local; 
  }
  else reso2 = reso;

  // filter shape
  fader = CV_filter16_out[index_filter_pot5];
  tmpS = CV4_value;
  tmpS *= CV_filter16_out[index_filter_pot6]>>1;
  tmpS >>= 15;
  fader2 = fader; // save for oposit mode 
  fader += tmpS;
  fader = max(0,min(0xFFFF, fader));
  fader *= 4;
  faderA = max(0,0xFFFF-fader);
  faderB = max(0,min(fader, 0x1FFFF-fader));
  faderC = max(0,min(fader-0xFFFF,(0x3FFFF-fader)>>1));
  faderD = max(0,fader-0x1FFFF) >> 1;

  G1_tmp = 4 * faderD;
  G2_tmp = -8 * (faderC + faderD);
  G3_tmp = (8 * faderB) + (8 * faderC) + ( 4 * faderD);
  G5_tmp = 12 * faderA; 

  //noInterrupts();
    G1 = G1_tmp>>4;
    G2 = G2_tmp>>4;
    G3 = G3_tmp>>4;
    G5 = G5_tmp>>4;
  //interrupts();

  fader = fader2; 
  fader -= tmpS;
  fader = max(0,min(0xFFFF, fader));
  fader *= 4;
  faderA = max(0,0xFFFF-fader);
  faderB = max(0,min(fader, 0x1FFFF-fader));
  faderC = max(0,min(fader-0xFFFF,(0x3FFFF-fader)>>1));
  faderD = max(0,fader-0x1FFFF)>>1;
  G1_tmp = 4 * faderD;
  G2_tmp = -8 * (faderC + faderD);
  G3_tmp = 8 * (faderB + faderC) + 4 * faderD;
  G5_tmp = 12 * faderA; 

  
  //noInterrupts(); 
  if(toggle==1) { // mode oposite
    G12 = G1_tmp>>4;
    G22 = G2_tmp>>4;
    G32 = G3_tmp>>4;
    G52 = G5_tmp>>4;
  } else { 
    G12 = G1; 
    G22 = G2; 
    G32 = G3; 
    G52 = G5; 
  }
  //interrupts();

/*
  // in_gain : IN1(L) / in1(R) / IN2(R)
  if (IN1_connect < 60) {
    gain = 1<<2;
    if (IN2_connect < 60) 
      {gain += 1;} 
    else 
      {gain += 1<<1;}
  } else 
  {
    gain = 0;
    if (IN2_connect < 60) 
      {gain += 1;} 
  }
*/
  if (IN2_connect < 60) { 
    gain = 1; 
  }
  else {
    gain = 2;
  }

  in_gain = gain;

  led2((CV3_value+0x7FFF)>>7);
  led4((CV4_value+0x7FFF)>>7);
}

inline void VCF_Param_audio_() {
  int32_t in, out, tmp, tmp1, VCF_out, inR;
  int32_t freq;
  uint32_t gain;

  gain = in_gain;
  
  // Left
  ////////////////////////////////////////////////////////////////////////////////////
  freq = freq_global;

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
 
  VCF_out = G1*(in1_oldL>>13) + G2*(in2_oldL>>13) + G3*(in3_oldL>>13) + G5*(in5_oldL>>13); // G4 is not used with this filter selection 
  //VCF_out = _min(0x3FFFFFFF, _max(-0x3FFFFFFF, VCF_out)) << 1;
  saturate_S31(VCF_out, VCF_out);
  VCF_out <<= 1;
  audio_outL = VCF_out^0x80000000;

   // Right
  ////////////////////////////////////////////////////////////////////////////////////
  freq = freq_global2;
   
  if (toggle_global != 2) // no need to compute it in PAN mode 
  {
    in = (audio_inR^0x80000000) * (gain & 0x1);
    in += inR * ((gain >> 1) & 0x1);
    in >>= 4; // 28 bits
    
    // feedback
    tmp = in5_oldR - (in >> 1) ;
    tmp = m_s32xs32_s32H(tmp, reso2);
    tmp <<= 1; // pour compenser le gain du au sign
    tmp *= -4;
     
    // distortion
    tmp1 = tmp;
    tmp = m_s32xs32_s32H(tmp1, abs(tmp1));
    //tmp = (tmp1>>16) * abs(tmp1>>16);
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
    
    VCF_out = G12*(in1_oldR>>13) + G22*(in2_oldR>>13) + G32*(in3_oldR>>13) + G52*(in5_oldR>>13); // G4 is not used with this filter selection
    //VCF_out = min(0x3FFFFFFF, max(-0x3FFFFFFF, VCF_out)) << 1;
    saturate_S31(VCF_out, VCF_out);
    VCF_out<<= 1;
    audio_outR = VCF_out^0x80000000;
  } 
  else { // PAN Mode
    out = VCF_out^0x80000000;
    macro_out_pan
  }
}


