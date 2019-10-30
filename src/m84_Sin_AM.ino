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

// quad sinus oscillator with amplitude modulation
// Pot 1 : FQ1
// Pot 2 : Mod1 (or fine if nothing is connected on the modulation)
// Pot 3 : FQ2
// Pot 4 : MOD2
// Pot 5 : FQ3
// Pot 6 : MOD3
// Pot 7 : FQ4
// Pot 8 : MOD4
// IN 1 : 1V/Oct
// IN 2 : MOD FQ1
// Selecteur3 : AM algo
// OUT 1 : OUTL
// OUT 2 : OUTR

inline uint32_t modulation_AM(uint32_t in, uint32_t amount) {
  int32_t tmpS, tmpS2;
  tmpS = amount;
  tmpS = 0x7FFFFFFF-tmpS;
  tmpS2 = in ^0x80000000;
  tmpS = (tmpS >> 16) * (tmpS2 >> 16);
  tmpS = min(0x3FFFFFFF, tmpS); 
  tmpS <<= 1;
  return( tmpS^0x80000000 );
}

inline uint32_t modulation_AM_U(uint32_t in, uint32_t amount) {
  uint32_t tmp;
  tmp = 0xFFFFFFFF-amount;
  tmp = (tmp >> 16) * (in >>16);
  return( tmp );
}

inline void sin_AM_init_() {
  sin1_phase = 0;
  sin2_phase = 0;
  sin3_phase = 0;
  sin4_phase = 0;
}

inline void sin_AM_loop_() {
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

  // Oscillateur 1
  freq = (CV_filter16_out[index_filter_pot1])<<11;
  freq += 0x07800000;
  macro_1VOct_IN1
  macro_FqMod_fine_IN2(pot2)
  macro_fq2increment_novar
  increment_1 = increment1; //save

  if (IN1_connect < 60) led2(audio_inL >> 23); else set_led2(0);
  if (IN2_connect < 60) led4(audio_inR >> 23); else set_led4(0);
}

inline void sin_AM_audio_() {
  uint32_t tmp, out, out2, modulation1, modulation2, modulation3;

  switch (get_toggle()) {
  case 0 : // 4 > (3 > (2 > 1))
    // Oscillateur 4
    sin4_phase += increment_4 <<3; 
    tmp = fast_sin(sin4_phase);
    tmp = ((tmp>>16) * CV_filter16_out[index_filter_pot8]);
    modulation1 = tmp;
    
    // Oscillateur 3
    sin3_phase += increment_3 <<3; 
    tmp = fast_sin(sin3_phase);
    tmp = ((tmp>>16) * CV_filter16_out[index_filter_pot6]);
    modulation2 = tmp;
    
    // Oscillateur 2
    sin2_phase += increment_2 <<3; 
    tmp = fast_sin(sin2_phase);
    tmp = ((tmp>>16) * CV_filter16_out[index_filter_pot4]);
    modulation3 = tmp;

    // Oscillateur 1
    sin1_phase += increment_1<<3; 
   
    out = fast_sin(sin1_phase);
    out = modulation_AM(out, modulation1);
    out = modulation_AM(out, modulation2);
    out = modulation_AM(out, modulation3);
    out2 = fast_sin(sin1_phase<<1);
    out2 = modulation_AM(out2, modulation1);
    out2 = modulation_AM(out2, modulation2);
    out2 = modulation_AM(out2, modulation3);  
    break;
    
  case 1 : // 4 > 3; 3 > (2 > 1)
    // Oscillateur 4
    sin4_phase += increment_4 <<3; 
    tmp = fast_sin(sin4_phase);
    tmp = ((tmp>>16) * CV_filter16_out[index_filter_pot8]);
    modulation1 = tmp;
    
    // Oscillateur 3
    sin3_phase += increment_3 <<3; 
    tmp = fast_sin(sin3_phase);
    tmp = modulation_AM_U(fast_sin(sin3_phase), modulation1);
    tmp = ((tmp>>16) * CV_filter16_out[index_filter_pot6]);
    modulation2 = tmp;
    
    // Oscillateur 2
    sin2_phase += increment_2 <<3; 
    tmp = fast_sin(sin2_phase);
    tmp = ((tmp>>16) * CV_filter16_out[index_filter_pot4]);
    modulation3 = tmp;

    // Oscillateur 1
    sin1_phase += increment_1<<3; 
    out = fast_sin(sin1_phase);
    out = modulation_AM(out, modulation2);
    out = modulation_AM(out, modulation3);
    out2 = fast_sin(sin1_phase<<1);
    out2 = modulation_AM(out2, modulation2);
    out2 = modulation_AM(out2, modulation3);
  break;
  case 2 : // 2+3+4 > 1
    // Oscillateur 4
    sin4_phase += increment_4 <<3; 
    tmp = fast_sin(sin4_phase);
    tmp = ((tmp>>16) * CV_filter16_out[index_filter_pot8]);
    modulation1 = tmp >> 2;
    
    // Oscillateur 3
    sin3_phase += increment_3 <<3; 
    tmp = fast_sin(sin3_phase);
    tmp = ((tmp>>16) * CV_filter16_out[index_filter_pot6]);
    modulation1 += tmp >> 2;
    
    // Oscillateur 2
    sin2_phase += increment_2 <<3; 
    tmp = fast_sin(sin2_phase);
    tmp = ((tmp>>16) * CV_filter16_out[index_filter_pot4]);
    modulation1 += tmp >> 2;

    // Oscillateur 1
    sin1_phase += increment_1<<3; 
    // out = fast_sin(sin1_phase);
    modulation1 = min(modulation1, 0x3FFFFFFF);
    modulation1 <<=2;
    out = modulation_AM(fast_sin(sin1_phase), modulation1);
    out2 = modulation_AM(fast_sin(sin1_phase<<1), modulation1);
  break;
  /*
  case 1 : //4 > 3; 3+2 > 1
    // Oscillateur 4
    sin4_phase += increment_4 <<3; 
    tmp = fast_sin(sin4_phase);
    tmp = ((tmp>>16) * CV_filter16_out[index_filter_pot8]);
    modulation1 = tmp;
    
    // Oscillateur 3
    sin3_phase += increment_3 <<3; 
    //tmp = fast_sin(sin3_phase);
    tmp = modulation_AM_U(fast_sin(sin3_phase), modulation1);
    tmp = ((tmp>>16) * CV_filter16_out[index_filter_pot6]);
    modulation1 = tmp >> 2;
    
    // Oscillateur 2
    sin2_phase += increment_2 <<3; 
    tmp = fast_sin(sin2_phase);
    tmp = ((tmp>>16) * CV_filter16_out[index_filter_pot4]);
    modulation1 += tmp >> 2;

    // Oscillateur 1
    freq = (CV_filter16_out[index_filter_pot1])<<11;
    freq += 0x07800000;
    macro_FqMod_fine_IN1(pot2)
    macro_fq2increment_novar
    sin1_phase += increment1<<3; 
    //out = fast_sin(sin1_phase);
    modulation1 = min(modulation1, 0x3FFFFFFF);
    modulation1 <<=2;
    out = modulation_AM(fast_sin(sin1_phase), modulation1);
    //out = fast_sin(sin1_phase);
  break;
  */
  /*
  case 2 : // 3+4 > 2 > 1
    // Oscillateur 4
    sin4_phase += increment_4 <<3; 
    tmp = fast_sin(sin4_phase);
    tmp = ((tmp>>16) * CV_filter16_out[index_filter_pot8]);
    modulation1 = tmp >> 2;
    
    // Oscillateur 3
    sin3_phase += increment_3<<3; 
    tmp = fast_sin(sin3_phase);
    tmp = ((tmp>>16) * CV_filter16_out[index_filter_pot6]);
    modulation1 += tmp >> 2;
    
    // Oscillateur 2
    sin2_phase += increment_2 <<3;
    modulation1 = min(modulation1, 0x3FFFFFFF);
    modulation1 <<=2;
    tmp = modulation_AM_U(fast_sin(sin2_phase), modulation1);
    //tmp = fast_sin(sin2_phase);
    tmp = ((tmp>>16) * CV_filter16_out[index_filter_pot4]);
    modulation1 = tmp;

    // Oscillateur 1
    freq = (CV_filter16_out[index_filter_pot1])<<11;
    freq += 0x07800000;
    macro_FqMod_fine_IN1(pot2)
    macro_fq2increment_novar
    sin1_phase += increment1<<3; 
    //out = fast_sin(sin1_phase);
    out = modulation_AM(fast_sin(sin1_phase), modulation1);
  break;
  */ 
  }  
  out -= out >> 2;
  out += 1<<29;
  out2 -= out2 >> 2;
  out2 += 1<<29;
  
  macro_out_stereo
}

