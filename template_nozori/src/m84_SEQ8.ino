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

// 8 Steps sequencer
// Pot 1 : step 1
// Pot 2 : step 2
// Pot 3 : step 3
// Pot 4 : step 4
// Pot 5 : step 5
// Pot 6 : step 6
// Pot 7 : step 7
// Pot 8 : step 8
// IN 1 : clock
// IN 2 : Effect
// Selecteur3 : effect mode (back / stop / glide)
// OUT 1 : POS OUT
// OUT 2 : SEQ OUT

uint32_t pos, clock_time, last_clock_time, last_goal, current_goal, goal;
bool last_clock;

inline void SEQ8_init_() {
  last_clock = false;
  pos = 0;
  last_clock_time = 1;
  clock_time = 0;
  last_goal = 0;
  goal = 0;
  current_goal = 0;
}

inline void SEQ8_loop_() {
  filter16_nozori_84
  test_connect_loop_84();
}

inline void SEQ8_audio_() {
  uint32_t toggle, inR, inL, tmp, skip;
  int32_t tmpS;
  
  toggle = get_toggle();
  clock_time++;
  interpol_pos += (0x7FFFFFFF / last_clock_time);
  interpol_pos = min(0x7FFFFFFF, interpol_pos);
  
  if (IN1_connect < 60) inL = audio_inL; else inL = 0;
  if (IN2_connect < 60) inR = audio_inR; else inR = 0;
  
  if ( (!last_clock) && (inL > 0xB0000000) ) { //front montant 
    if (inR > 0xB0000000) {
      switch (toggle) {
        case 0:
          pos = (pos-1) & 0b111;
          interpol_pos = 0x7FFFFFFF;
          skip = 0;
        break;
        case 1:
          interpol_pos = 0x7FFFFFFF;
          skip = 1;
          pos = (pos+1) & 0b111;
        break;
        case 2:
          skip = 0;          
          pos = (pos+1) & 0b111;
          interpol_pos = 0;
        break;
      }
    }
    else pos = (pos+1) & 0b111;

    if ( skip == 0) {
      switch (pos) {
        case 0 : goal = CV_filter16_out[index_filter_pot1]; break;
        case 1 : goal = CV_filter16_out[index_filter_pot2]; break;
        case 2 : goal = CV_filter16_out[index_filter_pot3]; break;
        case 3 : goal = CV_filter16_out[index_filter_pot4]; break;
        case 4 : goal = CV_filter16_out[index_filter_pot5]; break;
        case 5 : goal = CV_filter16_out[index_filter_pot6]; break;
        case 6 : goal = CV_filter16_out[index_filter_pot7]; break;
        case 7 : goal = CV_filter16_out[index_filter_pot8]; break;
      }
    }
    last_clock = true;
    last_clock_time = clock_time;
    clock_time = 0;
    last_goal = current_goal;
  }
  if ( (last_clock) && (audio_inL < 0xA0000000) ) {
    last_clock = false;
  }

  audio_outL = 677483648+(pos * 420000000);// pour etre proprement repartit en 32 bit (pos est de 0 a 7) 
  
  current_goal = MIX16U(last_goal>>1, goal>>1, interpol_pos>>15)<<1;  
  tmp = current_goal<<16; 
  tmpS = tmp^0x80000000;
  tmpS -= tmpS >>2;
  tmp = tmpS ^0x80000000;
  audio_outR = tmp;
}


