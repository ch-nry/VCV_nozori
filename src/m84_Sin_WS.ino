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

//Peter de Jong chaotic attractor used as a waveshaper, for 2 included sinusoid
// To be used as LFO or audio source
// Pot 1 : caracteristic frequency 1
// Pot 2 : Mod frequency 1
// Pot 3 : caracteristic frequency 2
// Pot 4 : Mod frequency 2
// Pot 5 : A
// Pot 6 : B
// Pot 7 : C
// Pot 8 : D
// IN 1 : FQ 1 mod
// IN 2 : FQ 2 mod
// Selecteur3 : Fq range : VCO / clock / lfo
// OUT 1 : OUT X
// OUT 2 : OUT Y

inline void SIN_WS_init_() {
}

inline void SIN_WS_loop_() {
  int32_t tmpS;
  uint32_t toggle;
  int32_t freq;

  filter16_nozori_84
  test_connect_loop_84();
  toggle = get_toggle();

  // Oscillateur 1
  switch (toggle) {
  case 0 : /* High : full audio range*/
    freq = (CV_filter16_out[index_filter_pot1])<<11;
    freq += 0x07000000;
    if (IN1_connect < 60) led2(audio_inL >> 23); else set_led2(0);
  break;
  case 1 : /* medium (for external control using 1V/Oct) */
    freq = (CV_filter16_out[index_filter_pot1])<<11;
    freq += 0x07000000;
    if (IN1_connect < 60) led2(audio_inL >> 23); else set_led2(0);
  break;
  case 2 :  /* low (LFO) */
    freq = (CV_filter16_out[index_filter_pot1])<<11;
    freq += 0x01000000;
    led2(audio_outL>>23);
  break;
  }
  macro_FqMod_fine_IN1(pot2)
  macro_fq2increment
  increment_1 = increment1; 

  // Oscillateur 2
  switch (toggle) {
  case 0 : /* High : full audio range*/
    freq = (CV_filter16_out[index_filter_pot3])<<11;
    freq += 0x07000000;
    if (IN2_connect < 60) led4(audio_inR >> 23); else set_led4(0);
  break;
  case 1 : /* medium (for external control using 1V/Oct) */
    freq = (CV_filter16_out[index_filter_pot3])<<11;
    freq += 0x01000000;
    if (IN2_connect < 60) led4(audio_inR >> 23); else set_led4(0);
  break;
  case 2 :  /* low (LFO) */
    freq = (CV_filter16_out[index_filter_pot3])<<11;
    freq += 0x01000000;
    led4(audio_outR>>23);  
  break;
  }
  macro_FqMod_fine_IN2(pot4)
  macro_fq2increment_novar
  increment_2 = increment1;
}

inline void SIN_WS_audio_() {
  uint32_t sin1, sin2;
  uint32_t A, B, C, D;
  uint32_t potA, potB, potC, potD;

  sin1_phase += increment_1;
  sin2_phase += increment_2;
  sin1 = fast_sin(sin1_phase);
  sin2 = fast_sin(sin2_phase);

  potA = filter(CV_filter16_out[index_filter_pot7]<<14, potA_save, 9);
  potA_save = potA;
  potB = filter(CV_filter16_out[index_filter_pot5]<<14, potB_save, 9);
  potB_save = potB;
  potC = filter(CV_filter16_out[index_filter_pot6]<<14, potC_save, 9);
  potC_save = potC;
  potD = filter(CV_filter16_out[index_filter_pot8]<<14, potD_save, 9);
  potD_save = potD;
  potA >>= 14;
  potB >>= 14;
  potC >>= 14;
  potD >>= 14;
  
  A = sin2>>16;
  A *= potA;
  B = sin1>>16;
  B *= potB;
  C = sin1>>16;
  C *= potC;
  D = sin2>>16;
  D *= potD;
  sin1 = (fast_sin(A)>>2) - (fast_sin(B+0xA0000000)>>2);
  sin2 = (fast_sin(C)>>2) - (fast_sin(D+0xA0000000)>>2);
  sin1 ^= 0x80000000;
  sin2 ^= 0x80000000;

  
  A = sin2>>12;
  A *= potA;
  B = sin1>>12;
  B *= potB;
  C = sin1>>12;
  C *= potC;
  D = sin2>>12;
  D *= potD;
  sin1 = (fast_sin(A)>>2) - (fast_sin(B+0xA0000000)>>2);
  sin2 = (fast_sin(C)>>2) - (fast_sin(D+0xA0000000)>>2);
  sin1 ^= 0x80000000;
  sin2 ^= 0x80000000;

  /* // pas assez de puissance pour un 3eme
  A = sin2>>12;
  A *= potA;
  B = sin1>>12;
  B *= potB;
  C = sin1>>12;
  C *= potC;
  D = sin2>>12;
  D *= potD;
  sin1 = (fast_sin(A)>>2) - (fast_sin(B+0xA0000000)>>2);
  sin2 = (fast_sin(C)>>2) - (fast_sin(D+0xA0000000)>>2);
  sin1 ^= 0x80000000;
  sin2 ^= 0x80000000;
  */
  
  audio_outL=sin1;
  audio_outR=sin2;
}

