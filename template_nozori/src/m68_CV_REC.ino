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

//CV REC
// Pot 1 : Clock Speed
// Pot 2 : Mod time
// Pot 3 : CV1
// Pot 4 : MOD CV1
// Pot 5 : CV2
// Pot 6 : Mod CV2
// CV 1 : SYNCRO 
// CV 2 : clock modulation value
// CV 3 : rec CV1
// CV 4 : rec CV2
// IN 1 : CV1 modulation value
// IN 2 : CV2 modulation value
// Selecteur3 : REC 1 / play / REC 2
// OUT 1 : OUT CV1
// OUT 2 : OUT CV2

inline void CV_REC_init_() {
  int i;
  for (i=0; i <= Max_Delay; i++) {
    delay_line.U16[i] = 0x7FFF;
  }
}

inline void CV_REC_loop_() {
  uint32_t toggle, tmp;
  int32_t freq, tmpS;

  filter16_nozori_68
  test_connect_loop_68();
  toggle = get_toggle();
  toggle_global = toggle;

  // CLOCK
  if (CV1_connect < 60) { // syncro sur l'entree
    //freq = CV_filter16_out[index_filter_pot1] / 7282; // from 0 to 8.999
    freq = (CV_filter16_out[index_filter_pot1] + 4095) / 8192; // from 0.5 to 8.5
    //tmp = CV_filter16_out[index_filter_pot2]/9363; // from 0 to 6.999
    tmp = (CV_filter16_out[index_filter_pot2] + 5461)/10923; // from 0.5 to 6.5
    clock_diviseur = tab_diviseur[freq] * tab_diviseur2[tmp];
    clock_multiplieur = tab_multiplieur[freq] * tab_multiplieur2[tmp];
  }
  else { // pas de syncro, on calcul l'increment normallement
  freq = CV_filter16_out[index_filter_pot1] * 1400; 
  freq += 0x02000000;
  macro_FqMod_fine(pot2, CV2)
  macro_fq2increment
  increment1 = min(increment1<<1, 0x7FFFFFFF>>14); // demi periode pour le "pos" 
  LFO1_increment = increment1;
  }
}

inline void CV_REC_audio_() {
  int32_t modulation1, modulation2;
  uint32_t toggle, pos, current_tick, increment1;
  int32_t tmpS;
  int32_t out1, out2;

  toggle = toggle_global;

  nb_tick++;
  if( (last_clock_ == 0) && (CV1_connect < 60) && (CV_filter16_out[index_filter_cv1] > 0xA000) ) { // mode syncro, on a une syncro
    last_clock_ = 1;
    current_tick = nb_tick;
    nb_tick = 0;
    increment1 = 0xFFFFFFFF / current_tick;
    increment1 /= clock_diviseur;
    increment1 *= clock_multiplieur;
    LFO1_increment = min(increment1, 0x7FFFFFFF>>14);
  }
  else if (CV_filter16_out[index_filter_CV1] < 0x9000){
    last_clock_ = 0;
  }

  LFO1_phase += LFO1_increment;
  pos = LFO1_phase >> 18; 

  modulation1 = CV_filter16_out[index_filter_pot3];
  modulation1 -= modulation1>>2;
  modulation1 += 1<<13;
  if (IN1_connect < 60) tmpS = audio_inL^0x80000000; else tmpS = 0;
  tmpS >>= 16;
  tmpS*= CV_filter16_out[index_filter_pot4]>>1;
  tmpS >>= 15;
  modulation1 += tmpS;
  modulation1 = min(0xFFFF, max(0,modulation1));
  
  modulation2 = CV_filter16_out[index_filter_pot5];
  modulation2 -= modulation2>>2;
  modulation2 += 1<<13;
  if (IN2_connect < 60) tmpS = audio_inR^0x80000000; else tmpS = 0;
  tmpS >>= 16;
  tmpS *= CV_filter16_out[index_filter_pot6]>>1;
  tmpS >>= 15;
  modulation2 += tmpS;
  modulation2 = min(0xFFFF, max(0,modulation2));

  //audio_outL = LFO1_phase<<15;
  //audio_outR = LFO1_phase;
  //audio_outL = delay_line.S16[pos]<<16;
  //audio_outR =  delay_line.U16[pos+((Max_Delay+1)>>1)]<<16;

  tmpS = (LFO1_phase>>9) & 0x1FF; // interpolation time
  out1 = delay_line.U16[pos];
  out2 = delay_line.U16[(pos+1) & (Max_Delay>>1)];
  out2 -= out1;
  out2 *= tmpS;
  out1 <<= 16;
  out1 += out2 << 7;
  audio_outL = out1;

  out1 = delay_line.U16[(pos & (Max_Delay>>1)) + ((Max_Delay+1)>>1)];
  out2 = delay_line.U16[((pos+1) & (Max_Delay>>1)) + ((Max_Delay+1)>>1)];
  out2 -= out1;
  out2 *= tmpS;
  out1 <<= 16;
  out1 += out2 << 7;
  audio_outR = out1;

  if ((toggle == 0) || ((CV3_connect < 60) && (CV_filter16_out[index_filter_CV3] > 0xA000)) ) { // rec CV1
    delay_line.U16[pos] = modulation1;
    audio_outL = modulation1<<16;
  }
  if ((toggle == 2) || ((CV4_connect < 60) && (CV_filter16_out[index_filter_CV4] > 0xA000)) ) { // rec CV2
    delay_line.U16[pos+((Max_Delay+1)>>1)] = modulation2; 
    audio_outR = modulation2<<16;
  }
  
  led2(LFO1_phase>>23);
  led4(audio_outL>>23);
}

