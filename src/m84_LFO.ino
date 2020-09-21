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
// LFO
//----------------------------------------------------------------------------------

// dual LFO with a modulation from 1 -> 2
// Pot 1 : FQ LFO 1
// Pot 2 : FQ LFO 2
// Pot 3 : WF LFO 1
// Pot 4 : idem LFO 2
// Pot 5 : SYM LFO 1
// Pot 6 : idem LFO 2
// Pot 7 : WRAP LFO 1
// Pot 8 : MOD LFO 2
// IN 1 : syncro LFO1
// IN 2 : T&H LFO 2 : retrig a chaque step
// Selecteur3 : modulation mode (MIX/FM/AM)
// OUT 1 : OUT LFO 1
// OUT 2 : OUT LFO 2
// LED1 : LFO1
// LED2 : LFO2

#define LFO1_range 2900           //increase for more diference beetween low and high frequency, decrease for less
                                  // should not be higher than 4095
#define LFO1_offset 0x1000000     // offset on fader position, (increase for higer frequency, decrease for lower freq)
#define LFO2_range 2900           // idem LFO 1
#define LFO2_offset 0x1000000     // idem LFO1


inline void LFO_Mod_init_() {
  send_dac(0x08,0b000000001); // sampling control (usb , 250fs,  48K, clock div 2, clk out, active)
  LFO1_phase = 0x00000000;
  LFO2_phase = 0x00000000;
  symetrie_1 = 0x1000000;
  symetrie_2 = 0x1000000;
  clock_diviseur = 1;
}


inline void LFO_Mod_loop_() {
  int32_t freq;
  uint32_t tmp, tmp2;
  uint32_t tmp_symetrie, tmp_distortion, tmp_distortion2, tmp_gain, tmp_offset_gain;
  int32_t  tmp_offset_signed;
  uint32_t freq_MSB, freq_LSB;
  uint32_t current_inL, current_inR;

  filter16_nozori_84
  test_connect_loop_84();

  // LFO 1
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // Frequency
  if (IN1_connect < 60) { // syncro sur l'entree
    //freq = CV_filter16_out[index_filter_pot1] / 7282; // from 0 to 9
    freq = (CV_filter16_out[index_filter_pot1] + 4095) / 8192;
    clock_diviseur = tab_diviseur[freq];
    clock_multiplieur = tab_multiplieur[freq];    
  }
  else { // pas de syncro, on calcul l'increment normallement
    freq = CV_filter16_out[index_filter_pot1] * LFO1_range; // << 11.5
    freq += LFO1_offset;
    freq += 0xC00000;
    freq = min(0xFFFFFFF, freq);
    //freq = max(0, freq);
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

 
  // LFO 2
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // symetry
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

  current_inL = audio_inL;
  current_inR = audio_inR;
  
  if(                                                                       // actualise seulement si necessaire
    !( (IN2_connect < 60) && ((current_inR < 0xB0000000)||(hold == 1)) )    // trig and hold
    )  { 
    hold = 1;
    actualise_LFO2 = 1;
  } else { actualise_LFO2 = 0; }
  if (current_inR < 0xA0000000) hold = 0; // hysteresis sur le trigger
}

inline void LFO_Mod_audio_() {
  uint32_t tmp, phase, tmp2, current_tick, increment1;
  int32_t tmpS, tmpS2, LFO1_value;
  uint32_t symetrie, toggle;
  int32_t LFO2_increment;
  int32_t freq;
  uint32_t freq_MSB, freq_LSB;
  
  // syncro
  nb_tick++;
  if( (last_clock_ == 0) && (IN1_connect < 60) && (audio_inL > 0xB0000000) ) { // mode syncro, on a une syncro
    last_clock_ = 1;
    current_tick = nb_tick;
    nb_tick = 0;
    increment1 = 0xFFFFFFFF / current_tick;
    increment1 /= clock_diviseur;
    increment1 *= clock_multiplieur;
    LFO1_increment = increment1;
  }
  else if (audio_inL < 0xA0000000){
    last_clock_ = 0;
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
  
  // distortion 1
    // calcul du sinus
  //tmpS2 = (2*tmpS) - ((tmpS*abs(tmpS)) >> 15);
  tmp = fast_sin(tmpS<<15);
  tmp >>= 16;
  tmpS2 = tmp;
  tmpS2 -= 0x7FFF;
    //mix tri -> sinus
  tmpS = MIX16U(tmpS, tmpS2, distortion_1); // 15 bit + sign

  // WRAP modulation (positive only)
  tmpS += 0x7FFF;
  tmpS *= (CV_filter16_out[index_filter_pot7]>>2) + 0x2000;
  tmpS = min(tmpS, 0x3FFFFFFF);
  tmpS *= 4;
  tmpS = abs(tmpS);
  tmpS >>= 15;
  tmpS -= (1<<15); // To make it symetrical

  /*
  // WRAP modulation (both direction)
  tmpS *= ((ADC_IN[7]-hysteresis_size)<<3) + 0x3FFF;
  tmpS = max(tmpS, -0x3FFFFFFF);
  tmpS = min(tmpS, 0x3FFFFFFF);
  if (tmpS > 0x1FFFFFFF) {tmpS = 0x3FFFFFFF-tmpS;}
  if (tmpS < -0x1FFFFFFF) {tmpS = -0x3FFFFFFF-tmpS;}
  tmpS >>= 14;
  */

  // distortion2 
    // calcul du sinus
  tmpS2 = (2*tmpS) - ((tmpS*abs(tmpS)) >> 15);
    //mix tri -> sinus
  tmpS = MIX16U(tmpS, tmpS2, distortion2_1*2); // 15 bit + sign
  //tmpS = min(tmpS, 0x7FFF);
  //tmpS = max(tmpS, -0x7FFF);


  tmp = tmpS+0x8000; // positive only sur 16 bits
  led2(min(511,tmp>>7));

  audio_outL = (tmpS*45000)^0x80000000;
  LFO1_value = tmpS;

  // LFO 2
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  toggle = get_toggle();

  freq = CV_filter16_out[index_filter_pot2] * LFO2_range; // 
  freq += LFO2_offset;
  freq += 0xC00000;
  if (toggle == 1) { // FM
    tmpS = LFO1_value * CV_filter16_out[index_filter_pot8];
    freq += tmpS>>6;
    freq = min(0xFFFFFFF, freq);
    freq = max(0, freq);    
  }
  freq_MSB = freq >> 18; // keep the 1st 10 bits
  freq_LSB = freq & 0x3FFFF; // other 18  bits
  tmp2 = table_CV2increment[freq_MSB];
  tmp = table_CV2increment[freq_MSB+1];
  tmp -= tmp2;
  tmp2 += ((tmp>>8)*(freq_LSB>>2))>>8;
  LFO2_increment = tmp2 << 3;

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
  //tmpS2 = (2*tmpS) - ((tmpS*abs(tmpS)) >> 15);
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

  if (toggle == 0) { // MIX
    tmpS2 = LFO1_value-tmpS;
    tmpS2 *= CV_filter16_out[index_filter_pot8]>>1;
    tmpS2 >>= 15;
    tmpS += tmpS2;
  } 
  else if (toggle == 2) { // AM
    tmpS2 = tmpS * ( LFO1_value+(1<<15));
    tmpS2 >>= 16;
    tmpS2 = tmpS2-tmpS;
    tmpS2 *= CV_filter16_out[index_filter_pot8];
    tmpS2 >>= 16;
    tmpS += tmpS2;
  }
  
  if( actualise_LFO2 )  { 
    tmp = tmpS+0x8000; // positive only sur 16 bits
    led4(min(511,tmp>>7));
    audio_outR = (tmpS*45000)^0x80000000;
  }
}
