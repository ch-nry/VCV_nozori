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

// sinus oscillator with a diferents harmonics (additiv synthesys)
// Pot 1 : FQ
// Pot 2 : Mod Fq (or fine if nothing is connected on the modulation)
// Pot 3 : FQ H1 (relative from 0 ~ +1 Oct)
// Pot 4 : GAIN H1
// Pot 5 : FQ H2 (relative from 0 ~ +2 Oct)
// Pot 6 : GAIN H2
// Pot 7 : FQ H3 (relative from 0 ~ +2 Oct)
// Pot 8 : GAIN H3
// IN 1 : 1V/Oct
// IN 2 : MOD FQ
// Selecteur3 : FQ range
// OUT 1 : OUT
// OUT 2 : OUT (without fundamental)

//uint32_t VCO_H1_phase, VCO_H2_phase, VCO_H3_phase;

inline void VCO_Harmo_init_() {
  VCO1_phase = 0;
  VCO_H1_phase = 0;
  VCO_H2_phase = 0;
  VCO_H3_phase = 0;
}

inline void VCO_Harmo_loop_() {
  int32_t tmpS;

  filter16_nozori_84
  test_connect_loop_84();

  // Fq principale
  macro_fq_in_tlg
  macro_1VOct_IN1
  macro_FqMod_fine_IN2(pot2);
  freq_save = freq;

  if (IN1_connect < 60) led2(audio_inL >> 23); else set_led2(0);
  if (IN2_connect < 60) led4(audio_inR >> 23); else set_led4(0);
}

inline void VCO_Harmo_audio_() {
  int32_t tmpS;
  int32_t outS, outS2;
  uint32_t out, out2;
  int32_t freq;

  freq = freq_save;
  // Oscillateur principal
  macro_fq2increment
  VCO1_phase += increment1<<3; 
  tmpS = fast_sin(VCO1_phase)^0x80000000;
  outS = tmpS>>2;
  outS2 = 0;
  
  // Harmonique 1
  freq = freq_save;
  tmpS = min(4086<<4, CV_filter16_out[index_filter_pot3]);
  tmpS = (tmpS * 2048)/2043;
  freq += tmpS * (48<<2);
  
  freq = _min(0xFA00000, freq); /*20KHz max*/
  freq = _max(0, freq);
  freq_MSB = freq >> 18; /* keep the 1st 10 bits */
  freq_LSB = freq & 0x3FFFF; /* other 18  bits */
  increment1 = table_CV2increment[freq_MSB];
  increment2 = table_CV2increment[freq_MSB+1];
  increment2 -= increment1;
  increment1 += ((increment2>>8)*(freq_LSB>>2))>>8;

  VCO_H1_phase += increment1<<3;    
  tmpS = fast_sin(VCO_H1_phase)^0x80000000;
  tmpS >>= 17;
  tmpS *= CV_filter16_out[index_filter_pot4];
  outS2 += tmpS>>1;

  // Harmonique 2
  freq = freq_save;
  tmpS = min(4086<<4, CV_filter16_out[index_filter_pot5]);
  tmpS = (tmpS * 2048)/2043;
  freq += tmpS *(48<<3);
  
  freq = _min(0xFA00000, freq); /*20KHz max*/
  freq = _max(0, freq);
  freq_MSB = freq >> 18; /* keep the 1st 10 bits */
  freq_LSB = freq & 0x3FFFF; /* other 18  bits */
  increment1 = table_CV2increment[freq_MSB];
  increment2 = table_CV2increment[freq_MSB+1];
  increment2 -= increment1;
  increment1 += ((increment2>>8)*(freq_LSB>>2))>>8;

  VCO_H2_phase += increment1<<3;    
  tmpS = fast_sin(VCO_H2_phase)^0x80000000;
  tmpS >>= 17;
  tmpS *= CV_filter16_out[index_filter_pot6];
  outS2 += tmpS>>1;

  // Harmonique 3
  freq = freq_save;
  tmpS = min(4086<<4, CV_filter16_out[index_filter_pot7]);
  tmpS = (tmpS * 2048)/2043;
  freq += tmpS *(48<<3);
  
  freq = _min(0xFA00000, freq); /*20KHz max*/
  freq = _max(0, freq);
  freq_MSB = freq >> 18; /* keep the 1st 10 bits */
  freq_LSB = freq & 0x3FFFF; /* other 18  bits */
  increment1 = table_CV2increment[freq_MSB];
  increment2 = table_CV2increment[freq_MSB+1];
  increment2 -= increment1;
  increment1 += ((increment2>>8)*(freq_LSB>>2))>>8;

  VCO_H3_phase += increment1<<3;    
  tmpS = fast_sin(VCO_H3_phase)^0x80000000;
  tmpS >>= 17;
  tmpS *= CV_filter16_out[index_filter_pot8];
  outS2 += tmpS>>1;

  outS += outS2;
  
  out = outS ^0x80000000;
  out2 = outS2 ^0x80000000;
  macro_out_stereo
}

