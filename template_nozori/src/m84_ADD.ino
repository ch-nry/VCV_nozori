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

// sinus oscillator with a diferents harmonics
// Pot 1 : FQ
// Pot 2 : Mod Fq (or fine if nothing is connected on the modulation)
// Pot 3 : FQ H1
// Pot 4 : GAIN H1
// Pot 5 : FQ H2
// Pot 6 : MOD H2
// Pot 7 : FQ H3
// Pot 8 : MOD H3
// IN 1 : 1V/Oct
// IN 2 : MOD FQ
// Selecteur3 : FQ range
// OUT 1 : OUT
// OUT 2 : OUT without fundamental

uint32_t VCO_Add1_phase, VCO_Add2_phase, VCO_Add3_phase, VCO_Add4_phase, VCO_Add5_phase, VCO_Add6_phase;

inline void VCO_Add_init_() {
  VCO1_phase = 0;
  VCO_Add1_phase = 0;
  VCO_Add2_phase = 0;
  VCO_Add3_phase = 0;
  VCO_Add4_phase = 0;
  VCO_Add5_phase = 0;
  VCO_Add6_phase = 0;
}

inline void VCO_Add_loop_() {
  int32_t tmpS;
  
  filter16_nozori_84
  test_connect_loop_84();

  // Fq principale
  macro_fq_in
  macro_1VOct_IN1
  macro_FqMod_fine_IN2(pot2);
  
  // Calcule des increments
  macro_fq2increment
  increment_0 = increment1;
  switch(get_toggle()) {
    case 0 : // Harmonics
      increment_1 = min(0x19999999, increment1 * 2); // 20KHz max
      increment_2 = min(0x19999999, increment1 * 3);
      increment_3 = min(0x19999999, increment1 * 4);
      increment_4 = min(0x19999999, increment1 * 5);
      increment_5 = min(0x19999999, increment1 * 6);
      increment_6 = min(0x19999999, increment1 * 7);
    break;
    case 1 : // major
      increment_1 = (increment1 / 8) * 9;     // 2nd major, +2
      increment_2 = (increment1 / 4) * 5;     // 3rd major, +4
      increment_3 = (increment1 / 3) * 4;     // 4th, +5
      increment_4 = (increment1 / 2) * 3;     // 5th, +7
      increment_5 = (increment1 / 3) * 5;     // 6th major, +9
      increment_6 = (increment1 / 8) * 15;    // 7th major, +11
    break;
    case 2 : // minor
      increment_1 = (increment1 / 8) * 9;     // 2nd major, +2
      increment_2 = (increment1 / 5) * 6;     // 3rd minor, +3
      increment_3 = (increment1 / 3) * 4; // 4th, +5
      increment_4 = (increment1 / 2) * 3; // 5th, +7
      increment_5 = (increment1 / 5) * 8; // 6 minor, +8
      increment_6 = (increment1 / 9) * 16; // 7 minor, +10   
    break;
  }
  if (IN1_connect < 60) led2(audio_inL >> 23); else set_led2(0);
  if (IN2_connect < 60) led4(audio_inR >> 23); else set_led4(0);
}

inline void VCO_Add_audio_() {
  int32_t outS, tmpS, outS2;
  uint32_t out, out2;

  // Oscillateur principal
  VCO1_phase += increment_0<<3; 
  tmpS = fast_sin(VCO1_phase)^0x80000000;
  outS = tmpS>>3;
  outS2=0;

  // Harmonique 1
  VCO_Add1_phase += increment_1 <<3;    
  tmpS = fast_sin(VCO_Add1_phase)^0x80000000;
  tmpS >>= 17;
  tmpS *= CV_filter16_out[index_filter_pot3];
  outS2 += tmpS>>2;

  // Harmonique 2
  VCO_Add2_phase += increment_2<<3;    
  tmpS = fast_sin(VCO_Add2_phase)^0x80000000;
  tmpS >>= 17;
  tmpS *= CV_filter16_out[index_filter_pot4];
  outS2 += tmpS>>2;

  // Harmonique 3
  VCO_Add3_phase += increment_3<<3;    
  tmpS = fast_sin(VCO_Add3_phase)^0x80000000;
  tmpS >>= 17;
  tmpS *= CV_filter16_out[index_filter_pot5];
  outS2 += tmpS>>2;

   // Harmonique 4
  VCO_Add4_phase += increment_4 <<3;    
  tmpS = fast_sin(VCO_Add4_phase)^0x80000000;
  tmpS >>= 17;
  tmpS *= CV_filter16_out[index_filter_pot6];
  outS2 += tmpS>>2;

  // Harmonique 5
  VCO_Add5_phase += increment_5<<3;    
  tmpS = fast_sin(VCO_Add5_phase)^0x80000000;
  tmpS >>= 17;
  tmpS *= CV_filter16_out[index_filter_pot7];
  outS2 += tmpS>>2;

  // Harmonique 6
  VCO_Add6_phase += increment_6<<3;    
  tmpS = fast_sin(VCO_Add6_phase)^0x80000000;
  tmpS >>= 17;
  tmpS *= CV_filter16_out[index_filter_pot8];
  outS2 += tmpS>>2;

  outS += outS2; 

  outS += outS >> 3;
  outS2 += outS2 >> 2;
  
  out = outS ^0x80000000;
  out2 = outS2 ^0x80000000;
  audio_outL = out;
  audio_outR = out2;
}

