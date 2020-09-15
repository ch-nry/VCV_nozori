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

//Seq 4
// Pot 1 : Value 1
// Pot 2 : Time 1
// Pot 3 : Value 2
// Pot 4 : Time 2
// Pot 5 : Value 3
// Pot 6 : Time 3
// Pot 7 : Value 4
// Pot 8 : Time 4
// IN 1 : clock
// IN 2 : Effect
// Selecteur3 : effect type (skip / long gate / glide)
// OUT 1 : Gate
// OUT 2 : Value 

uint32_t time_value[4], out_value[4];
uint32_t clock_div, current_pos;
bool last_gate, new_clock;
uint32_t value_out, gate_out;
uint32_t interpol_increment, interpol_A, interpol_B;
bool do_interpol;

inline void SEQ4_init_() {
  last_gate = false;
  clock_div = 1;
  current_pos = 0;
  new_clock = false;

  value_out = 0;
  gate_out = 0;
  interpol_increment = 0;
  interpol_A = 0;
  interpol_B = 0;
  do_interpol = false;
}

inline void SEQ4_loop_() {
  uint32_t toggle;

  filter16_nozori_84
  test_connect_loop_84();
  toggle = get_toggle();
  toggle_global = toggle;

  //time_value[0] = CV_filter16_out[index_filter_pot2] / 7282; // from 0 to 8
  //time_value[1] = CV_filter16_out[index_filter_pot4] / 7282; 
  //time_value[2] = CV_filter16_out[index_filter_pot6] / 7282; 
  //time_value[3] = CV_filter16_out[index_filter_pot8] / 7282; 
  time_value[0] = (CV_filter16_out[index_filter_pot2] + 4095) / 8192;
  time_value[1] = (CV_filter16_out[index_filter_pot4] + 4095) / 8192;
  time_value[2] = (CV_filter16_out[index_filter_pot6] + 4095) / 8192;
  time_value[3] = (CV_filter16_out[index_filter_pot8] + 4095) / 8192;

  out_value[0] = CV_filter16_out[index_filter_pot1];
  out_value[1] = CV_filter16_out[index_filter_pot3];
  out_value[2] = CV_filter16_out[index_filter_pot5];
  out_value[3] = CV_filter16_out[index_filter_pot7];
}

inline void SEQ4_audio_() {
  uint32_t toggle, tmp;
  int32_t tmpS;
  nb_tick++;

  toggle = toggle_global;
  if ( (IN1_connect < 60) && (audio_inL > 0xB0000000) && (last_gate== false) ) {
    last_gate = true;
    new_clock = true;
    last_clock_ = nb_tick;
    nb_tick = 0;
    if ( (toggle==1) && (IN2_connect < 60) && (audio_inR > 0xB0000000) ) {
      gate_out = 0xE0000000;
    }
  } 
  else if ((IN1_connect < 60) && (audio_inL < 0xA0000000)) {
    last_gate = false; 
    gate_out = OUT1_0V;
    //if (!( (toggle==1) && (IN2_connect < 60) && (audio_inR > 0xB0000000) && (clock_div != 0) )) { // !long gate
    //  gate_out = OUT1_0V;
    //} 
  }
  
  if ( (new_clock) && (clock_div > 0) ) {
    clock_div--;
    new_clock = false;
  }
  if ( (new_clock) && (clock_div == 0) ) {
    current_pos = (current_pos+1) & 0b11;
    clock_div = time_value[current_pos];  
    if ((clock_div > 0)) { 
      clock_div--;
      new_clock = false;
      if  (!((toggle==0) && (IN2_connect < 60) && (audio_inR > 0xB0000000))) { // if we are not in skip mode
        gate_out = 0xE0000000;
        if ((toggle==2) && (IN2_connect < 60) && (audio_inR > 0xB0000000)) { // glide mode
          do_interpol = true;
          interpol_B = out_value[current_pos];
          interpol_A = audio_outR>>16;
          interpol_increment = 0xFFFFFFFF / last_clock_;
          interpol_increment /= clock_div+1;
          interpol_pos = 0;
        } else {
          do_interpol = false;
          value_out = out_value[current_pos];
        }
      }
    }
  }
  
  if(do_interpol) {
    interpol_pos += interpol_increment;
    if (interpol_pos < interpol_increment) { // overflow
      interpol_pos = 0xFFFFFFFF;
    }
    value_out = MIX16U(interpol_A>>1, interpol_B>>1, interpol_pos>>16)<<1;
  }

  audio_outL = gate_out;
  tmp = value_out<<16; 
  tmpS = tmp^0x80000000;
  tmpS -= tmpS >> 2;
  tmp = tmpS^0x80000000;
  audio_outR = tmp; 
  
  led2(min(gate_out-OUT1_0V,0x1FF));
  led4(value_out>>7);
}

