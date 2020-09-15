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

// parametric oscilator : diferents points can be manually adjusted
// Pot 1 : FQ
// Pot 2 : Mod Fq (or fine if nothing is connected on the modulation)
// Pot 3 : A1
// Pot 4 : A2
// Pot 5 : A3
// Pot 6 : A4
// Pot 7 : A5
// Pot 8 : A6
// IN 1 : 1V/Oct
// IN 2 : MOD FQ
// Selecteur3 : FQ range
// OUT 1 : OUT
// OUT 2 : OUT

uint32_t save_increment;

inline void OSC_Param_init_() {
  VCO1_phase = 0;
  save_increment = 0;
}

inline void OSC_Param_loop_() {
  int32_t tmpS;

  filter16_nozori_84
  test_connect_loop_84();

  // Fq principale
  macro_fq_in_tlg
  macro_1VOct_IN1 
  macro_FqMod_fine_IN2(pot2);
  macro_fq2increment
  save_increment = increment1<<2;

  if (IN1_connect < 60) led2(audio_inL >> 23); else set_led2(0);
  if (IN2_connect < 60) led4(audio_inR >> 23); else set_led4(0);
}

inline void OSC_Param_audio_() {
  uint32_t pos, out, interpolation;
  uint32_t value1, value2, value3, value4;

  // Oscillateur principal
  VCO1_phase += save_increment; 

  pos = VCO1_phase / 0x2AAAAAAA;
  interpolation = VCO1_phase*6;

/*
  switch (pos) { // no interpolation
    case 0 :
      out = CV_filter16_out[index_filter_pot3] << 16;
    break;
    case 1 :
      out = CV_filter16_out[index_filter_pot4] << 16;
    break;
    case 2 : 
      out = CV_filter16_out[index_filter_pot5] << 16;
    break;
    case 3 :
      out = CV_filter16_out[index_filter_pot6] << 16;
    break;
    case 4 :
      out = CV_filter16_out[index_filter_pot7] << 16;
    break;
    case 5 : 
      out = CV_filter16_out[index_filter_pot8] << 16;
    break;
  }
*/

/*
  switch (pos) { // linear interpolation
    case 0 :
      value1 = CV_filter16_out[index_filter_pot3];
      value2 = CV_filter16_out[index_filter_pot4];
      out = MIX16U(value1, value2, interpolation>>16);
    break;
    case 1 :
      value1 = CV_filter16_out[index_filter_pot4];
      value2 = CV_filter16_out[index_filter_pot5];
      out = MIX16U(value1, value2, interpolation>>16);
    break;
    case 2 : 
      value1 = CV_filter16_out[index_filter_pot5];
      value2 = CV_filter16_out[index_filter_pot6];
      out = MIX16U(value1, value2, interpolation>>16);
    break;
    case 3 :
      value1 = CV_filter16_out[index_filter_pot6];
      value2 = CV_filter16_out[index_filter_pot7];
      out = MIX16U(value1, value2, interpolation>>16);
    break;
    case 4 :
      value1 = CV_filter16_out[index_filter_pot7];
      value2 = CV_filter16_out[index_filter_pot8];
      out = MIX16U(value1, value2, interpolation>>16);
    break;
    case 5 : 
      value1 = CV_filter16_out[index_filter_pot8];
      value2 = CV_filter16_out[index_filter_pot3];
      out = MIX16U(value1, value2, interpolation>>16);
    break;
  }
  out <<= 16;
*/

  switch (pos) { // cubic interpolation
    case 0 :
      value1 = CV_filter16_out[index_filter_pot3] >> 2;
      value2 = CV_filter16_out[index_filter_pot4] >> 2;
      value3 = CV_filter16_out[index_filter_pot5] >> 2;
      value4 = CV_filter16_out[index_filter_pot6] >> 2;
      out = tabread4(value1, value2, value3, value4, interpolation>>16);
    break;
    case 1 :
      value1 = CV_filter16_out[index_filter_pot4] >> 2;
      value2 = CV_filter16_out[index_filter_pot5] >> 2;
      value3 = CV_filter16_out[index_filter_pot6] >> 2;
      value4 = CV_filter16_out[index_filter_pot7] >> 2;
      out = tabread4(value1, value2, value3, value4, interpolation>>16);
    break;
    case 2 : 
      value1 = CV_filter16_out[index_filter_pot5] >> 2;
      value2 = CV_filter16_out[index_filter_pot6] >> 2;
      value3 = CV_filter16_out[index_filter_pot7] >> 2;
      value4 = CV_filter16_out[index_filter_pot8] >> 2;
      out = tabread4(value1, value2, value3, value4, interpolation>>16);
    break;
    case 3 :
      value1 = CV_filter16_out[index_filter_pot6] >> 2;
      value2 = CV_filter16_out[index_filter_pot7] >> 2;
      value3 = CV_filter16_out[index_filter_pot8] >> 2;
      value4 = CV_filter16_out[index_filter_pot3] >> 2;
      out = tabread4(value1, value2, value3, value4, interpolation>>16);
    break;
    case 4 :
      value1 = CV_filter16_out[index_filter_pot7] >> 2;
      value2 = CV_filter16_out[index_filter_pot8] >> 2;
      value3 = CV_filter16_out[index_filter_pot3] >> 2;
      value4 = CV_filter16_out[index_filter_pot4] >> 2;
      out = tabread4(value1, value2, value3, value4, interpolation>>16);
    break;
    case 5 : 
      value1 = CV_filter16_out[index_filter_pot8] >> 2;
      value2 = CV_filter16_out[index_filter_pot3] >> 2;
      value3 = CV_filter16_out[index_filter_pot4] >> 2;
      value4 = CV_filter16_out[index_filter_pot5] >> 2;
      out = tabread4(value1, value2, value3, value4, interpolation>>16);
    break;
  }
  out = 0x34000000 + (out * 165000);
  audio_outL = out;


// out R
/////////////////////////////////////////////////////////

  pos = (VCO1_phase<<1) / 0x2AAAAAAA;
  interpolation = VCO1_phase*12;

  switch (pos) { // cubic interpolation
    case 0 :
      value1 = CV_filter16_out[index_filter_pot3] >> 2;
      value2 = CV_filter16_out[index_filter_pot4] >> 2;
      value3 = CV_filter16_out[index_filter_pot5] >> 2;
      value4 = CV_filter16_out[index_filter_pot6] >> 2;
      out = tabread4(value1, value2, value3, value4, interpolation>>16);
    break;
    case 1 :
      value1 = CV_filter16_out[index_filter_pot4] >> 2;
      value2 = CV_filter16_out[index_filter_pot5] >> 2;
      value3 = CV_filter16_out[index_filter_pot6] >> 2;
      value4 = CV_filter16_out[index_filter_pot7] >> 2;
      out = tabread4(value1, value2, value3, value4, interpolation>>16);
    break;
    case 2 : 
      value1 = CV_filter16_out[index_filter_pot5] >> 2;
      value2 = CV_filter16_out[index_filter_pot6] >> 2;
      value3 = CV_filter16_out[index_filter_pot7] >> 2;
      value4 = CV_filter16_out[index_filter_pot8] >> 2;
      out = tabread4(value1, value2, value3, value4, interpolation>>16);
    break;
    case 3 :
      value1 = CV_filter16_out[index_filter_pot6] >> 2;
      value2 = CV_filter16_out[index_filter_pot7] >> 2;
      value3 = CV_filter16_out[index_filter_pot8] >> 2;
      value4 = CV_filter16_out[index_filter_pot3] >> 2;
      out = tabread4(value1, value2, value3, value4, interpolation>>16);
    break;
    case 4 :
      value1 = CV_filter16_out[index_filter_pot7] >> 2;
      value2 = CV_filter16_out[index_filter_pot8] >> 2;
      value3 = CV_filter16_out[index_filter_pot3] >> 2;
      value4 = CV_filter16_out[index_filter_pot4] >> 2;
      out = tabread4(value1, value2, value3, value4, interpolation>>16);
    break;
    case 5 : 
      value1 = CV_filter16_out[index_filter_pot8] >> 2;
      value2 = CV_filter16_out[index_filter_pot3] >> 2;
      value3 = CV_filter16_out[index_filter_pot4] >> 2;
      value4 = CV_filter16_out[index_filter_pot5] >> 2;
      out = tabread4(value1, value2, value3, value4, interpolation>>16);
    break;
  }
  out = 0x34000000 + (out * 165000);
  audio_outR = out;
}

