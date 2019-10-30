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

// quad sinus oscillator with linear frequency modulation
// Pot 1 : FQ1
// Pot 2 : Mod1 (or fine if nothing is connected on the modulation)
// Pot 3 : FQ2
// Pot 4 : MOD2
// Pot 5 : FQ3
// Pot 6 : MOD3
// Pot 7 : FQ4
// Pot 8 : MOD4
// IN 1 : 1V/Oct
// IN 2 : mod fq 1
// Selecteur3 : FM algo
// OUT 1 : OUTL
// OUT 2 : OUTR

inline void sin_FM_lin_init_() {
  sin1_phase = 0;
  sin2_phase = 0;
  sin3_phase = 0;
  sin4_phase = 0;
}

inline void sin_FM_lin_loop_() {
  filter16_nozori_84
  test_connect_loop_84();
  int32_t freq, tmpS;
  
  // Oscillateur 4
  freq = (CV_filter16_out[index_filter_pot7])*3500;
  freq += 0x02000000;
  macro_1VOct_IN1
  macro_fq2increment
  increment_4 = increment1; //save

  // Oscillateur 3
  freq = (CV_filter16_out[index_filter_pot5])*3500;
  freq += 0x02000000;
  macro_1VOct_IN1
  macro_fq2increment_novar
  increment_3 = increment1; //save

    // Oscillateur 2
  freq = (CV_filter16_out[index_filter_pot3])*3500;
  freq += 0x02000000;
  macro_1VOct_IN1
  macro_fq2increment_novar
  increment_2 = increment1; //save

  if (IN1_connect < 60) led2(audio_inL >> 23); else set_led2(0);
  if (IN2_connect < 60) led4(audio_inR >> 23); else set_led4(0);
}

inline void sin_FM_lin_audio_() {
  int32_t tmpS, modulation1, freq;
  uint32_t out, out2;
  uint32_t freq_MSB, freq_LSB;
  uint32_t increment1, increment2;

  switch (get_toggle()) {
  case 0 :
    // Oscillateur 4
    //freq = (CV_filter16_out[index_filter_pot7])<<11;
    //freq += 0x04000000;
    //macro_fq2increment_novar
    increment1 = increment_4;
    sin4_phase += increment1<<3; 
    tmpS = fast_sin(sin4_phase)^0x80000000;
    tmpS = ((tmpS>>16) * CV_filter16_out[index_filter_pot8]);
    modulation1 = tmpS >> 4;
    
    // Oscillateur 3
    //freq = (CV_filter16_out[index_filter_pot5])<<11;
    //freq += 0x04000000;
    //macro_fq2increment_novar
    increment1 = increment_3;
    sin3_phase += increment1<<3; 
    tmpS = fast_sin(sin3_phase)^0x80000000;
    tmpS = ((tmpS>>16) * CV_filter16_out[index_filter_pot6]);
    modulation1 += tmpS >> 4;
    
    // Oscillateur 2
    //freq = (CV_filter16_out[index_filter_pot3])<<11;
    //freq += 0x04000000;
    //macro_fq2increment_novar
    increment1 = increment_2;
    sin2_phase += increment1<<3; 
    tmpS = fast_sin(sin2_phase)^0x80000000;
    tmpS = ((tmpS>>16) * CV_filter16_out[index_filter_pot4]);
    modulation1 += tmpS >> 4;

    // Oscillateur 1
    freq = (CV_filter16_out[index_filter_pot1])<<11;
    freq += 0x07000000;
    macro_1VOct_IN1
    macro_FqMod_fine_IN2(pot2)
    macro_fq2increment_novar
    sin1_phase += increment1<<3; 
    sin1_phase += modulation1;
    out = fast_sin(sin1_phase);
    out2 = fast_sin(sin1_phase<<1);
  break;
  case 1 :
    // Oscillateur 4
    //freq = (CV_filter16_out[index_filter_pot7])<<11;
    //freq += 0x04000000;
    //macro_fq2increment_novar
    increment1 = increment_4;
    sin4_phase += increment1<<3; 
    tmpS = fast_sin(sin4_phase)^0x80000000;
    tmpS = ((tmpS>>16) * CV_filter16_out[index_filter_pot8]);
    modulation1 = tmpS >> 4;
    
    // Oscillateur 3
    //freq = (CV_filter16_out[index_filter_pot5])<<11;
    //freq += 0x04000000;
    //freq += modulation1;
    //macro_fq2increment_novar
    increment1 = increment_3;
    sin3_phase += increment1<<3; 
    sin3_phase += modulation1;
    tmpS = fast_sin(sin3_phase)^0x80000000;
    tmpS = ((tmpS>>16) * CV_filter16_out[index_filter_pot6]);
    modulation1 = tmpS >> 4;
    
    // Oscillateur 2
    //freq = (CV_filter16_out[index_filter_pot3])<<11;
    //freq += 0x04000000;
    //macro_fq2increment_novar
    increment1 = increment_2;
    sin2_phase += increment1<<3; 
    tmpS = fast_sin(sin2_phase)^0x80000000;
    tmpS = ((tmpS>>16) * CV_filter16_out[index_filter_pot4]);
    modulation1 += tmpS >> 4;

    // Oscillateur 1
    freq = (CV_filter16_out[index_filter_pot1])<<11;
    freq += 0x07000000;
    macro_FqMod_fine_IN2(pot2)
    macro_1VOct_IN1
    macro_fq2increment_novar
    sin1_phase += increment1<<3; 
    sin1_phase += modulation1; 
    out = fast_sin(sin1_phase);
    out2 = fast_sin(sin1_phase<<1);
  break;
  case 2 :
    // Oscillateur 4
    //freq = (CV_filter16_out[index_filter_pot7])<<11;
    //freq += 0x04000000;
    //macro_fq2increment_novar
    increment1 = increment_4;
    sin4_phase += increment1<<3; 
    tmpS = fast_sin(sin4_phase)^0x80000000;
    tmpS = ((tmpS>>16) * CV_filter16_out[index_filter_pot8]);
    modulation1 = tmpS >> 4;
    
    // Oscillateur 3
    //freq = (CV_filter16_out[index_filter_pot5])<<11;
    //freq += 0x04000000;
    //macro_fq2increment_novar
    increment1 = increment_3;
    sin3_phase += increment1<<3; 
    tmpS = fast_sin(sin3_phase)^0x80000000;
    tmpS = ((tmpS>>16) * CV_filter16_out[index_filter_pot6]);
    modulation1 += tmpS >> 4;
    
    // Oscillateur 2
    //freq = (CV_filter16_out[index_filter_pot3])<<11;
    //freq += 0x04000000;
    //freq += modulation1;
    //macro_fq2increment_novar
    increment1 = increment_2;
    sin2_phase += increment1<<3;
    sin2_phase += modulation1;
    tmpS = fast_sin(sin2_phase)^0x80000000;
    tmpS = ((tmpS>>16) * CV_filter16_out[index_filter_pot4]);
    modulation1 = tmpS >> 4;

    // Oscillateur 1
    freq = (CV_filter16_out[index_filter_pot1])<<11;
    freq += 0x07000000;
    macro_FqMod_fine_IN2(pot2)
    macro_1VOct_IN1
    macro_fq2increment_novar
    sin1_phase += increment1<<3; 
    sin1_phase += modulation1;
    out = fast_sin(sin1_phase);
    out2 = fast_sin(sin1_phase<<1);
  break;    
  }
  out -= out >> 2; // reduire un peu le gain pour rester ds -5/+5V
  out += 0x80000000 >> 2; // pour symetriser

  out2 -= out2 >> 2; // reduire un peu le gain pour rester ds -5/+5V
  out2 += 0x80000000 >> 2; // pour symetriser

  macro_out_stereo
}

