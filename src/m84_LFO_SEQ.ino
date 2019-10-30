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

// LFO Sequencer
// Pot 1 : clock fq
// Pot 2 : clock mod
// Pot 3 : value 1
// Pot 4 : value 2
// Pot 5 : value 3
// Pot 6 : value 4
// Pot 7 : value 5
// Pot 8 : value 6
// IN 1 : syncro
// IN 2 : fq mod
// Selecteur3 : interpolation : none, lin, cubic
// OUT 1 : POS OUT (CLOCK)
// OUT 2 : SEQ OUT

const uint32_t tab_pot[16] = {index_filter_pot3, index_filter_pot4, index_filter_pot5, index_filter_pot6, index_filter_pot7, index_filter_pot8, index_filter_pot3, 0, 0, 0, 0, 0, 0, index_filter_pot6, index_filter_pot7, index_filter_pot8};

inline void LFO_SEQ_init_() {
}

inline void LFO_SEQ_loop_() {
  uint32_t toggle, tmp;
  int32_t tmpS, freq;
  
  filter16_nozori_84
  test_connect_loop_84();
  toggle = get_toggle();
  toggle_global = toggle;

  if (IN1_connect < 60) { // syncro sur l'entree
    //freq = CV_filter16_out[index_filter_pot1] / 7282; // from 0 to 9
    freq = (CV_filter16_out[index_filter_pot1] + 4095) / 8192;
    //tmp = CV_filter16_out[index_filter_pot2]/9363; // from 0 to 7
    tmp = (CV_filter16_out[index_filter_pot2] + 5461)/10923; // from 0.5 to 6.5
    clock_diviseur = tab_diviseur[freq] * tab_diviseur2[tmp];
    clock_multiplieur = tab_multiplieur[freq] * tab_multiplieur2[tmp];    
  }
  else { // pas de syncro, on calcul l'increment normallement
    freq = (CV_filter16_out[index_filter_pot1])<<11;
    freq += 0x03000000;
    macro_FqMod_fine_IN2(pot2)
    macro_fq2increment
    LFO1_increment = increment1<<2; //save
  }
  led2(LFO1_phase>>23);
  led4(audio_outR>>23);
}

inline void LFO_SEQ_audio_() {
  uint32_t current_tick, pos, interpolation, toggle, increment1, tmp2;
  int32_t tmpS, outX;
  
  toggle = toggle_global;
  
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

  LFO1_phase += LFO1_increment;
  pos = LFO1_phase/715827883; //(0xFFFFFFFF/6); 
  interpolation = LFO1_phase*6;

  audio_outL = (LFO1_phase-(LFO1_phase>>2)) + (1<<29);

  switch(toggle) {
  case 0: // saw
    tmpS = CV_filter16_out[tab_pot[pos]] - 0x8000;
    tmpS >>= 1;
    tmp2 = interpolation >> 16;
    tmpS *= tmp2;
    outX = ((0xFFFF - CV_filter16_out[tab_pot[pos]])>>1)  + (tmpS>>15);
    break;
  case 1: // square
    outX = CV_filter16_out[tab_pot[pos]]>>1;
    break;
/*  case 1: // triangle
    tmpS = CV_filter16_out[tab_pot[pos]] - CV_filter16_out[tab_pot[(pos-1) & 0xF]];
    tmpS >>= 1;
    tmp2 = interpolation >> 16;
    tmpS *= tmp2;
    outX = (CV_filter16_out[tab_pot[(pos-1) & 0xF]]>>1) + (tmpS>>16);
    break;*/
  case 2: // curve
    tmp2 = interpolation >> 16;
    tmpS = tabread4(CV_filter16_out[tab_pot[(pos-2)& 0xF]]>>2, CV_filter16_out[tab_pot[(pos-1)& 0xF]]>>2, CV_filter16_out[tab_pot[pos]]>>2, CV_filter16_out[tab_pot[pos+1]]>>2, tmp2);
    outX = tmpS<<1;
    break;
  }
  outX *= 90000; // lower gain since cubic interpolation can be out of range
  outX += (((65535*2) - 90000)/2)<<15;
  audio_outR=outX;  
}


