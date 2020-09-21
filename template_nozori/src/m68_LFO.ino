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

//----------------------------------------------------------------------------------
// dual LFO
//----------------------------------------------------------------------------------

// VCO with control of the WF : square to saw to double saw and PWM control
// Pot 1 : Fq LFO 1
// Pot 2 : Fq LFO 2
// Pot 3 : WF 1
// Pot 4 : WF 2
// Pot 5 : SYM 1
// Pot 6 : SYM 2
// CV 1 : Trig and Hold 1
// CV 2 : Syncro
// CV 3 : Trig and Hold 2
// CV 4 : syncro
// IN 1 : Reset phase 1
// IN 2 : Reset phase 2
// Selecteur3 : Fq range
// OUT 1 : OUT 1
// OUT 2 : OUT 2

#define LFO1_range 2000             // increase for more diference beetween low and high frequency, decrease for less
                                    // should not be higher than 4095
#define LFO1_offset1 0x6000000      // offset on fader position, (increase for higer frequency, decrease for lower freq)
#define LFO1_offset2 0x3000000      // offset depend of the switch position
#define LFO1_offset3 0              // 

#define LFO2_range 2000             // idem LFO 1
#define LFO2_offset1 0x6000000      // idem LFO1
#define LFO2_offset2 0x3000000      //
#define LFO2_offset3 0

inline void LFO_Dual_init_() {
  LFO1_phase = 0x00000000;
  LFO2_phase = 0x00000000;
  last_clock_1 = 0;
  last_clock_2 = 0;
  symetrie_1 = 0x1000000;
  symetrie_2 = 0x1000000;
}

inline void LFO_Dual_loop_() {
  int32_t freq;
  uint32_t tmp, tmp2;
  uint32_t tmp_symetrie, tmp_distortion, tmp_distortion2, tmp_gain, tmp_offset_gain;
  int32_t  tmp_offset_signed;
  uint32_t freq_MSB, freq_LSB;
  uint32_t toggle;
  
  filter16_nozori_68
  test_connect_loop_68();
  toggle = get_toggle();
  
  // LFO 1
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // frequency
  if (CV2_connect < 60) { // syncro sur l'entree
    //freq = CV_filter16_out[index_filter_pot1] / 7282; // from 0 to 8 (8.999)
    freq = (CV_filter16_out[index_filter_pot1] + 4095) / 8192;
    clock1_diviseur = tab_diviseur[freq];
    clock1_multiplieur = tab_multiplieur[freq];    
  }
  else {
    freq = CV_filter16_out[index_filter_pot1] * LFO1_range; // << 11.5
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
  tmp_symetrie = (0xFFFF - CV_filter16_out[index_filter_pot5])<<16; // 32 bits
  tmp_symetrie = min(tmp_symetrie, 0xFFFF0000);
  tmp_symetrie = max(tmp_symetrie, 0x00010000);

  // WF : distortion 1, 2 and Gain
  tmp = 3*(CV_filter16_out[index_filter_pot3]>>1);
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

  if ( !((CV1_connect < 60) && ((CV_filter16_out[index_filter_cv1] < 0xB000)||(hold == 1))) ) {   // trig and hold
    hold = 1;
    actualise_LFO1 = 1;
  } else { actualise_LFO1 = 0; }
  if (CV_filter16_out[index_filter_cv1] < 0xA000) hold = 0; // hysteresis sur le trigger

  if ( (IN1_connect < 60) && (audio_inL > 0xB0000000) && (reset1 == 0) ) {   // reset
    reset1 = 1;
    LFO1_phase = 0;
  }
  if (audio_inL < 0xA0000000) reset1 = 0; // hysteresis sur le trigger

  // LFO 2
  ////////////////////////////////////////////////////////////////////////////////////////////////////

  // Frequency
  if (CV4_connect < 60) { // syncro sur l'entree
    //freq = CV_filter16_out[index_filter_pot2] / 7282; // from 0 to 9
    freq = (CV_filter16_out[index_filter_pot2] + 4095) / 8192;
    clock2_diviseur = tab_diviseur[freq];
    clock2_multiplieur = tab_multiplieur[freq];    
  }
  else {
    freq = CV_filter16_out[index_filter_pot2] * LFO2_range; // 
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
    LFO2_increment = tmp2 << 3;
  }
  
  tmp_symetrie = (0xFFFF - CV_filter16_out[index_filter_pot6])<<16; // 32 bits
  tmp_symetrie = min(tmp_symetrie, 0xFFFF0000);
  tmp_symetrie = max(tmp_symetrie, 0x00010000);
  
  // WF : distortion 1, 2 and Gain
  tmp = 3*(CV_filter16_out[index_filter_pot4]>>1);
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
  symetrie_2 = tmp_symetrie;
  distortion_2 = tmp_distortion*2;
  distortion2_2 = tmp_distortion2;
  gain_2 = tmp_gain;
  offset_gain_2 = tmp_offset_gain;
  offset_signed_2 = tmp_offset_signed;
  interrupts();

  if ( !((CV3_connect < 60) && ((CV_filter16_out[index_filter_cv3] < 0xB000)||(hold2 == 1))) ) {   // trig and hold
    hold2 = 1;
    actualise_LFO2 = 1;
  } else { actualise_LFO2 = 0; }
  if (CV_filter16_out[index_filter_cv3] < 0xA000) hold2 = 0; // hysteresis sur le trigger

  if ( (IN2_connect < 60) && (audio_inR > 0xB0000000) && (reset2 == 0) ) {   // trig and hold
    reset2 = 1;
    LFO2_phase = 0;
  } 
  if (audio_inR < 0xA0000000) reset2 = 0; // hysteresis sur le trigger

}

inline void LFO_Dual_audio_() {
  uint32_t tmp, phase, current_tick1, current_tick2, increment1;
  int32_t tmpS, tmpS2;
  uint32_t symetrie;

  // syncro 1
  nb_tick1++;
  if( (last_clock_1 == 0) && (CV2_connect < 60) && (CV_filter16_out[index_filter_cv2] > 0xB000) ) { // mode syncro, on a une syncro
    last_clock_1 = 1;
    current_tick1 = nb_tick1;
    nb_tick1 = 0;
    increment1 = 0xFFFFFFFF / current_tick1;
    increment1 /= clock1_diviseur;
    increment1 *= clock1_multiplieur;
    LFO1_increment = increment1;
  }
  else if ( (CV2_connect < 60) && (CV_filter16_out[index_filter_cv2] < 0xA000) ) {
    last_clock_1 = 0;
  }

  // syncro 2
  nb_tick2++;
  if( (last_clock_2 == 0) && (CV4_connect < 60) && (CV_filter16_out[index_filter_cv4] > 0xB000) ) { // mode syncro, on a une syncro
    last_clock_2 = 1;
    current_tick2 = nb_tick2;
    nb_tick2 = 0;
    increment1 = 0xFFFFFFFF / current_tick2;
    increment1 /= clock2_diviseur;
    increment1 *= clock2_multiplieur;
    LFO2_increment = increment1;
  }
  else if ( (CV4_connect < 60) && (CV_filter16_out[index_filter_cv4] < 0xA000) ) {
    last_clock_2 = 0;
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

  // LFO 2
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  phase = LFO2_phase + LFO2_increment; // 32 bits
  LFO2_phase = phase;

  // calcul de la symetrie
  symetrie = symetrie_2;
  tmp = (phase > (symetrie))? -phase / (-symetrie >> 16): phase / (symetrie >> 16); 
  
  // gain pour passage sin -> square
  tmpS = tmp - (1<<15) + offset_signed_2; // passage en signed
  tmpS *= min((1 << 5) + (gain_2 >> 15), 0x7FFF);
  tmpS >>= 5;
  tmpS = min( 0x7FFF, max(tmpS, -0x7FFF));

  // distortion 
  // calcul du sinus
  tmp = fast_sin(tmpS<<15);
  tmp >>= 16;
  tmpS2 = tmp;
  tmpS2 -= 0x7FFF;
  //mix tri -> sinus
  tmpS = MIX16U(tmpS, tmpS2, distortion_2); // 15 bit + sign

  // distortion2 
  // calcul du sinus
  tmpS2 = (2*tmpS) - ((tmpS*abs(tmpS)) >> 15);
  //mix tri -> sinus
  tmpS = MIX16U(tmpS, tmpS2, distortion2_2*2); // 15 bit + sign
  //tmpS = min(tmpS, 0x7FFF);
  //tmpS = max(tmpS, -0x7FFF);

  if( actualise_LFO2 )  { 
    tmp = tmpS+0x8000; // positive only sur 16 bits
    led4(min(511,tmp>>7));
    audio_outR = (tmpS*45000)^0x80000000;
  }
}
