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

// parametric LFO, with modulation control

// Pot 1 : FQ
// Pot 2 : Mod Fq (chaotic CV if nothing is connected)
// Pot 3 : WF (tri / sin / round square / square)
// Pot 4 : MOD WF
// Pot 5 : SYM
// Pot 6 : Mod SYM
// CV 1 : syncro
// CV 2 : Mod Fq 
// CV 3 : Mod WF
// CV 4 : Mod SYM
// IN 1 : Trig and Hold
// IN 2 : RESET PHASE
// Selecteur3 : FQ Range
// OUT 1 : OUT 
// OUT 2 : OUT dephasé de 180°

#define LFO1_range 2000             // increase for more diference beetween low and high frequency, decrease for less
                                    // should not be higher than 4095
#define LFO1_offset1 0x6000000      // offset on fader position, (increase for higer frequency, decrease for lower freq)
#define LFO1_offset2 0x3000000      // offset depend of the switch position
#define LFO1_offset3 0              // 

inline void LFO_MOD_init_() {
  init_chaos();
  LFO1_phase = 0x00000000;
  last_clock_1 = 0;
  symetrie_1 = 0x1000000;
}

inline void LFO_MOD_loop_() {
  int32_t freq, tmpS, tmpS2;
  uint32_t tmp, tmp2;
  uint32_t tmp_symetrie, tmp_distortion, tmp_distortion2, tmp_gain, tmp_offset_gain;
  int32_t  tmp_offset_signed;
  uint32_t freq_MSB, freq_LSB;
  uint32_t toggle;
  int32_t CV2_value, CV3_value, CV4_value;
  
  filter16_nozori_68
  test_connect_loop_68();
  toggle = get_toggle();
  chaos_div(1<<15); // for default mod values

  if (CV2_connect < 60) CV2_value = CV_filter16_out[index_filter_cv2] - CV2_0V; else CV2_value = chaos_dx>>16;
  if (CV3_connect < 60) CV3_value = CV_filter16_out[index_filter_cv3] - CV3_0V; else CV3_value = chaos_dy>>16;
  if (CV4_connect < 60) CV4_value = CV_filter16_out[index_filter_cv4] - CV4_0V; else CV4_value = chaos_dz>>16;

  CV2_value = min(0x7FFF,max(-0x7FFF,CV2_value));
  CV3_value = min(0x7FFF,max(-0x7FFF,CV3_value));
  CV4_value = min(0x7FFF,max(-0x7FFF,CV4_value));

  // Freq
  if (CV1_connect < 60) { // syncro sur l'entree
    //freq = CV_filter16_out[index_filter_pot1] / 7282; // from 0 to 8 (8.999)
    freq = (CV_filter16_out[index_filter_pot1] + 4095) / 8192;
    tmp = (CV_filter16_out[index_filter_pot2] + 5461)/10923; // from 0.5 to 6.5 //  i.e: 0..6 // i.e :7 values
    clock1_diviseur = tab_diviseur[freq]  * tab_diviseur2[tmp];;
    clock1_multiplieur = tab_multiplieur[freq] * tab_multiplieur2[tmp];;    
  }
  else {
    tmpS2 = CV_filter16_out[index_filter_pot1];
    tmpS = CV2_value;
    tmpS *= CV_filter16_out[index_filter_pot2];
    tmpS >>= 16;
    tmpS2 += tmpS;
    tmpS2 = min(0xFFFF, max(0,tmpS2));
    tmp = tmpS2;
  
    freq = tmp * LFO1_range; // << 11.5
    switch (toggle) {
      case 0 :
        freq += LFO1_offset1;
      break;
      case 1 :
        freq += LFO1_offset2;
      break;
      case 2 :
        freq += LFO1_offset3;
      break;
    }
    freq_MSB = freq >> 18; // keep the 1st 10 bits
    freq_LSB = freq & 0x3FFFF; // other 18  bits
    tmp2 = table_CV2increment[freq_MSB];
    tmp = table_CV2increment[freq_MSB+1];
    tmp -= tmp2;
    tmp2 += ((tmp>>8)*(freq_LSB>>2))>>8;
    LFO1_increment = tmp2 << 3;
  }
  
  // symetry
  tmpS2 = CV_filter16_out[index_filter_pot5];
  tmpS = CV4_value;
  tmpS *= CV_filter16_out[index_filter_pot6];
  tmpS >>= 16;
  tmpS2 += tmpS;
  tmpS2 = min(0xFFFF, max(0,tmpS2));
  tmp = tmpS2;
  
  //tmp_symetrie = (0xFFFF - CV_filter16_out[index_filter_pot5])<<16; // 32 bits
  tmp_symetrie = (0xFFFF - tmp)<<16; // 32 bits
  tmp_symetrie = min(tmp_symetrie, 0xFFFF0000);
  tmp_symetrie = max(tmp_symetrie, 0x00010000);

  // WF : distortion 1, 2 and Gain
  tmpS2 = CV_filter16_out[index_filter_pot3];
  tmpS = CV3_value;
  tmpS *= CV_filter16_out[index_filter_pot4];
  tmpS >>= 16;
  tmpS2 += tmpS;
  tmpS2 = min(0xFFFF, max(0,tmpS2));
  tmp = tmpS2;
  
  //tmp = 3*(CV_filter16_out[index_filter_pot3]>>1);
  tmp = 3*(tmp>>1);
  tmp_distortion = min(tmp, 0x7FFF); // only 1/3 of the fader
  tmp_distortion2 = max(min(tmp, 0xFFFF), 0x7FFF) - 0x7FFF;
  tmp_gain = (max(tmp, 0x8000) - 0x8000) / 2;

  // offset pour le PWM
  tmp_offset_gain = tmp_gain; // 15 bits
  tmp_gain *= tmp_gain; // 30 bits max
  tmp_gain >>= 15;
  tmp_gain *= tmp_gain; // 30 bits max
  tmp_gain >>= 15;
  tmp_gain *= tmp_gain; // 30 bits max
  tmp_gain >>= 15;
  tmp_gain *= tmp_gain; // 30 bits max
  tmp_gain >>= 15;
  tmp_gain *= tmp_offset_gain;

  tmp_offset_signed = 0x7FFF - (tmp_symetrie>>16); // from -1 to 1; 15 bit + sign
  tmp_offset_signed *= tmp_offset_gain; // 15 + 15 + sign
  tmp_offset_signed >>= 15; 

  noInterrupts();
  symetrie_1 = tmp_symetrie;
  distortion_1 = tmp_distortion*2;
  distortion2_1 = tmp_distortion2;
  gain_1 = tmp_gain;
  offset_gain_1 = tmp_offset_gain;
  offset_signed_1 = tmp_offset_signed;
  interrupts();

  if ( !((IN1_connect < 60) && ((audio_inL < 0xB0000000)||(hold == 1))) ) {   // trig and hold
    hold = 1;
    actualise_LFO1 = 1;
  } else { actualise_LFO1 = 0; }
  if (audio_inL < 0xA0000000) hold = 0; // hysteresis sur le trigger

  if ( (IN2_connect < 60) && (audio_inR > 0xB0000000) && (reset1 == 0) ) {   // reset
    reset1 = 1;
    LFO1_phase = 0;
  }
  if (audio_inR < 0xA0000000) reset1 = 0; // hysteresis sur le trigger
}

inline void LFO_MOD_audio_() {
  uint32_t tmp, phase, current_tick1, increment1;
  int32_t tmpS, tmpS2;
  uint32_t symetrie;

  // syncro
  nb_tick1++;
  if( (last_clock_1 == 0) && (CV1_connect < 60) && (CV_filter16_out[index_filter_cv1] > 0xB000) ) { // mode syncro, on a une syncro
    last_clock_1 = 1;
    current_tick1 = nb_tick1;
    nb_tick1 = 0;
    increment1 = 0xFFFFFFFF / current_tick1;
    increment1 /= clock1_diviseur;
    increment1 *= clock1_multiplieur;
    LFO1_increment = increment1;
  }
  else if ( (CV1_connect < 60) && (CV_filter16_out[index_filter_cv1] < 0xA000) ) {
    last_clock_1 = 0;
  }
  
  // LFO 1
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  phase = LFO1_phase + LFO1_increment; // 32 bits
  LFO1_phase = phase;
  
  // calcul de la symetrie
  symetrie = symetrie_1;
  tmp = (phase > (symetrie))? -phase / (-symetrie >> 16): phase / (symetrie >> 16); 
  
  // gain pour passage sin -> square
  tmpS = tmp - (1<<15) + offset_signed_1; // passage en signed
  tmpS *= min((1 << 5) + (gain_1 >> 15), 0x7FFF);
  tmpS >>= 5;
  tmpS = min( 0x7FFF, max(tmpS, -0x7FFF));
  
  // distortion 
  // calcul du sinus
  tmp = fast_sin(tmpS<<15);
  tmp >>= 16;
  tmpS2 = tmp;
  tmpS2 -= 0x7FFF;
  //mix tri -> sinus
  tmpS = MIX16U(tmpS, tmpS2, distortion_1); // 15 bit + sign

  // distortion2 
  // calcul du sinus
  tmpS2 = (2*tmpS) - ((tmpS*abs(tmpS)) >> 15);
  //mix tri -> sinus
  tmpS = MIX16U(tmpS, tmpS2, distortion2_1*2); // 15 bit + sign
  //tmpS = min(tmpS, 0x7FFF);
  //tmpS = max(tmpS, -0x7FFF);

  if( actualise_LFO1 )  { 
    tmp = tmpS+0x8000; // positive only sur 16 bits
    led2(min(511,tmp>>7));
    audio_outL = (tmpS*45000)^0x80000000;
  }
   // LFO 1 + 180°
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  phase = LFO1_phase + 0x80000000; // 32 bits
  
  // calcul de la symetrie
  symetrie = symetrie_1;
  tmp = (phase > (symetrie))? -phase / (-symetrie >> 16): phase / (symetrie >> 16); 
  
  // gain pour passage sin -> square
  tmpS = tmp - (1<<15) + offset_signed_1; // passage en signed
  tmpS *= min((1 << 5) + (gain_1 >> 15), 0x7FFF);
  tmpS >>= 5;
  tmpS = min( 0x7FFF, max(tmpS, -0x7FFF));
  
  // distortion 
  // calcul du sinus
  tmp = fast_sin(tmpS<<15);
  tmp >>= 16;
  tmpS2 = tmp;
  tmpS2 -= 0x7FFF;
  //mix tri -> sinus
  tmpS = MIX16U(tmpS, tmpS2, distortion_1); // 15 bit + sign

  // distortion2 
  // calcul du sinus
  tmpS2 = (2*tmpS) - ((tmpS*abs(tmpS)) >> 15);
  //mix tri -> sinus
  tmpS = MIX16U(tmpS, tmpS2, distortion2_1*2); // 15 bit + sign
  //tmpS = min(tmpS, 0x7FFF);
  //tmpS = max(tmpS, -0x7FFF);

  if( actualise_LFO1 )  { 
    tmp = tmpS+0x8000; // positive only sur 16 bits
    led4(min(511,tmp>>7));
    audio_outR = (tmpS*45000)^0x80000000;
  }
}

