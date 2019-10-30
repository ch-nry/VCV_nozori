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

// dual VCF with continious control of the frequency responce
// Pot 1 : FQ 1
// Pot 2 : FQ 2
// Pot 3 : MOD (Fine qd rien n'est connecté), filtre1
// Pot 4 : idem filtre 2
// Pot 5 : Q
// Pot 6 : idem filtre 2
// Pot 7 : frequency responce (lp24, lp12, bp12, HP12), filtre 1
// Pot 8 : idem filtre 2
// IN 1 : audio in 1
// IN 2 : audio in 2 (or pan)
// Selecteur3 : mode (stereo / inverted modulation / mono + pan)
// OUT 1 : OUT
// OUT 2 : OUT

uint32_t sel_input;

inline void VCF_Param84_init_() {
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

  default5(0);
}

inline void VCF_Param84_loop_() {
  uint32_t toggle, sel_input_local;
  int32_t tmpS, freq, IN2;
  int32_t fader,faderA, faderB, faderC, faderD;
  int32_t G1_tmp, G2_tmp, G3_tmp, G5_tmp;

  filter16_nozori_84
  //test_connect_loop_84();

  test_connect_loop_start
  test_connect_loop_test_in2
  test_connect_loop_test_end
  //default5(0);
  
  toggle = get_toggle();
  toggle_global = toggle;

  if (IN2_connect < 60) { IN2 = (audio_inR>>16) - (IN2_0V>>16);} else {IN2 = 0;}
  
  // Filter1
  //macro_fq_in
  freq = (CV_filter16_out[index_filter_pot1])<<11;
  freq += 0x06000000;
  freq +=  1<<25;
  if (toggle != 0) { // modulation
    tmpS = IN2;
    tmpS *= CV_filter16_out[index_filter_pot3]; 
    freq += tmpS>>5;
  }
  else { // fine tune
    tmpS = 48<<2;
    tmpS *= CV_filter16_out[index_filter_pot3]; 
    freq += tmpS;
  }
  macro_fq2cutoff
  freq_global = increment1;  
  
  reso = CV_filter16_out[index_filter_pot5]* 18000;

  fader = CV_filter16_out[index_filter_pot7];
  fader *= 4;
  faderA = max(0,0xFFFF-fader);
  faderB = max(0,min(fader, 0x1FFFF-fader));
  faderC = max(0,min(fader-0xFFFF,(0x3FFFF-fader)>>1));
  faderD = max(0,fader-0x1FFFF)>>1;

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
  

  // Filter2
  //macro_fq_in
  freq = (CV_filter16_out[index_filter_pot2])<<11;
  freq += 0x06000000;
  freq +=  1<<25;
  if (toggle != 0) {
    tmpS = IN2;
    tmpS *= CV_filter16_out[index_filter_pot4];
    if (toggle == 2) // mode inversé
      freq -= tmpS>>5;
    else
      freq += tmpS>>5;
  }  
  else { // fine tune
    tmpS = 48<<2;
    tmpS *= CV_filter16_out[index_filter_pot4]; 
    freq += tmpS;
  }
  macro_fq2cutoff_novar
  freq_global2 = increment1;  
  
  reso2 = CV_filter16_out[index_filter_pot6]* 18000;

  fader = CV_filter16_out[index_filter_pot8];
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
    G12 = G1_tmp>>4;
    G22 = G2_tmp>>4;
    G32 = G3_tmp>>4;
    G52 = G5_tmp>>4;
  //interrupts();

/*
  if (IN1_connect < 60) 
    sel_input_local = 2; 
  else
    sel_input_local = 0;
*/
  sel_input_local = 0;
  if ( (toggle == 0) && (IN2_connect < 60) ){
    sel_input_local += 1; // audio in R
  }

  sel_input = sel_input_local;

  if (toggle == 0) { 
    set_led2(0);
    set_led4(0);
  } 
  else if (toggle == 1) {
    led2(audio_inR>>23);
    led4(audio_inR>>23);
  }
  else if(toggle == 2) {
    led2(audio_inR>>23);
    led4(512-(audio_inR>>23));
  }
}

inline void VCF_Param84_audio_() {
  int32_t in, inL, tmp, tmp1, VCF_out, freq;
  uint32_t sel_input_local;

  sel_input_local = sel_input;
  
  // Left
  ////////////////////////////////////////////////////////////////////////////////////
  freq = freq_global;

/*
  if (sel_input_local >= 2) { 
    in = audio_inL;
    in ^= 0x80000000;
  }
  else { 
    in = 0;
  }
  */

  in = audio_inL;
  in ^= 0x80000000;
  
  inL = in;
  in >>= 4; // 28 bits

  // feedback
  tmp = in5_oldL - (in >> 1) ;
  tmp = m_s32xs32_s32H(tmp, reso);
  tmp <<= 3;
 
  // distortion
  tmp1 = tmp;
  tmp = m_s32xs32_s32H(tmp1, abs(tmp1));
  tmp = tmp1 - (tmp<<4);
  tmp += tmp >> 2;
  tmp += tmp1;
  
  //tmp += (tmp>>2) + (tmp>>3); // a bit more gain

  tmp = in - tmp;

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
  saturate_S31(VCF_out, VCF_out);
  VCF_out <<= 1;
  audio_outL = VCF_out^0x80000000;

   // Right
  ////////////////////////////////////////////////////////////////////////////////////

  freq = freq_global2;

  if (sel_input_local & 1) {
    in = audio_inR;
    in ^= 0x80000000;
    }
  else { 
    in = inL; 
  }
  in >>= 4; // 28 bits

  // feedback
  tmp = in5_oldR - (in >> 1) ;
  tmp = m_s32xs32_s32H(tmp, reso2);
  tmp <<= 3;
 
  // distortion
  tmp1 = tmp;
  tmp = m_s32xs32_s32H(tmp1, abs(tmp1));
  tmp = tmp1 - (tmp<<4);
  tmp += tmp >> 2;
  tmp += tmp1;
  
  //tmp += (tmp>>2) + (tmp>>3); // a bit more gain

  tmp = in - tmp;

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
  saturate_S31(VCF_out, VCF_out);
  VCF_out <<= 1;
  audio_outR = VCF_out^0x80000000;
}


