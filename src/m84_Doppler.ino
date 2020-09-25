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

// Pot 1 : distance 1
// Pot 2 : distance 2
// Pot 3 : doepler 1 -> 2
// Pot 4 : doepler 2 -> 1
// Pot 5 : doepler 1 -> 1
// Pot 6 : doepler 2 -> 2
// Pot 7 : filter 1
// Pot 8 : filter 2
// IN 1 : audio in 1
// IN 2 : audio in 2
// Selecteur3 : LOP filter on doppler, out, both
// OUT 1 : out L
// OUT 2 : out R


uint32_t time1_save, time2_save, mod3_save, mod4_save, filter1L_save, filter2L_save, filter1R_save, filter2R_save;
//uint32_t mod1_save, mod2_save;

//#define delay_line_size 0x3FFF // half so we can use 2 of them
int32_t filter_low_1L, filter_low_2L, filter_low_1R, filter_low_2R;
uint32_t low_gainL,low_gainR;

inline void Doppler_init_() {
  uint32_t i;
  set_led2(0);
  set_led4(0);

  time1_save = 0;
  time2_save = 0;
  mod3_save = 0;
  mod4_save = 0;
  filter1L_save = 0;
  filter2L_save = 0;
  filter1R_save = 0;
  filter2R_save = 0;

  filter_low_1L = 0;
  filter_low_2L = 0;
  filter_low_1R = 0;
  filter_low_2R = 0;
  low_gainL = 0;
  low_gainR = 0;

  for (i=0; i<Max_Delay;i++) {
    delay_line.S16[i] = 0;
  }
}

inline void Doppler_loop_() {
  int32_t freq;
  uint32_t freq_MSB, freq_LSB;
  uint32_t increment1, increment2;

  filter16_nozori_84  // macro that get CV and potentiometter value
  test_connect_loop_84(); // macro that test if a jack is plugged or not  
  toggle_global = get_toggle();;
  
  freq = (CV_filter16_out[index_filter_pot7]*3) << 10;
  freq += 1 << (16+10);
  freq_MSB = freq >> 18; /* keep the 1st 10 bits */
  freq_LSB = freq & 0x3FFFF; /* other 18  bits */
  increment1 = table_CV2increment[freq_MSB];
  increment2 = table_CV2increment[freq_MSB+1];
  increment2 -= increment1;
  increment1 += ((increment2>>8)*(freq_LSB>>2))>>8;
  increment1 -= 60;     
  low_gainL = increment1 * 12; // 31 bit max

  freq = (CV_filter16_out[index_filter_pot8]*3) << 10;
  freq += 1 << (16+10);
  freq_MSB = freq >> 18; /* keep the 1st 10 bits */
  freq_LSB = freq & 0x3FFFF; /* other 18  bits */
  increment1 = table_CV2increment[freq_MSB];
  increment2 = table_CV2increment[freq_MSB+1];
  increment2 -= increment1;
  increment1 += ((increment2>>8)*(freq_LSB>>2))>>8;
  increment1 -= 60;     
  low_gainR = increment1 * 12; // 31 bit max  
}

inline void Doppler_audio_() {  
  uint32_t delay_time1, delay_time2;
  int32_t out1, out2;
  uint32_t delay_time, delay_time_LSB, read_point;
  uint32_t time1, time2, mod1, mod2, mod3, mod4;
  int32_t tmpS, tmpS2;
  uint32_t inL, inR;
  uint32_t toggle;

  sin1_phase += 2926394; // C2
  sin2_phase += 4921317; // A2
  
  if (IN1_connect < 60) { inL = audio_inL^0x80000000; } else { tmpS = fast_sin(sin1_phase)^0x80000000; tmpS -= tmpS>>1; inL = tmpS; }
  if (IN2_connect < 60) { inR = audio_inR^0x80000000; } else { tmpS = fast_sin(sin2_phase)^0x80000000; tmpS -= tmpS>>1; inR = tmpS; }
  toggle = toggle_global;

  index_ecriture = (index_ecriture+1) & 0x3FFF;
  delay_line.S16[index_ecriture] = inL>>16;
  delay_line.S16[index_ecriture + 0x3FFF + 1] = inR>>16;

  time1 = filter(CV_filter16_out[index_filter_pot1] << 12, time1_save, 11); // 28 bit
  time1_save = time1;
  time2 = filter(CV_filter16_out[index_filter_pot2] << 12, time2_save, 11);
  time2_save = time2;
  mod1 = filter(CV_filter16_out[index_filter_pot3] << 12, mod1_save, 11); // 28 bit
  mod1_save = mod1;
  mod2 = filter(CV_filter16_out[index_filter_pot4] << 12, mod2_save, 11); // 28 bit
  mod2_save = mod2;
  mod3 = filter(CV_filter16_out[index_filter_pot5] << 12, mod3_save, 11); // 28 bit
  mod3_save = mod3;
  mod4 = filter(CV_filter16_out[index_filter_pot6] << 12, mod4_save, 11); // 28 bit
  mod4_save = mod4;

  mod1 >>= 12;
  mod1 *= mod1;
  mod2 >>= 12;
  mod2 *= mod2;
  mod3 >>= 12;
  mod3 *= mod3;
  mod4 >>= 12;
  mod4 *= mod4;

  tmpS = inR;
  tmpS >>= 16;
  tmpS *= mod1>>18;
  tmpS >>= 8;
  tmpS2 = inL;
  tmpS2 >>= 16;
  tmpS2 *= mod3>>18;
  tmpS2 >>= 8;
  tmpS += tmpS2;
  //tmpS >>= 1;
  tmpS += time1>>5; // Taile max = 14+9 = 23 bit
  tmpS = max(0, tmpS);
  delay_time1 = tmpS;
  delay_time = min(delay_time1, 0x7FFE00); // Taile max = 14+9 = 23 bit  
  if (!(toggle & 0b1)) { // 0 ou 2 
    tmpS = delay_time<<8;
    tmpS -= filter_low_2L; // 31 bits + sign
    _m_s32xs32_s32(tmpS, low_gainL, tmpS, tmpS2);
    filter_low_2L += tmpS;
    delay_time = (filter_low_2L>>8);
  }
  delay_time_LSB = delay_time & 0x1FF; // on garde les 9 bits de poids faible pour interpoler
  delay_time = delay_time >> 9; // on les suprime pour ne garder que l'index sur 14 bit (taille du buffer)
  read_point = (index_ecriture - delay_time) & 0x3FFF;
  out1 = delay_line.S16[read_point];
  out2 = delay_line.S16[(read_point-1) & 0x3FFF];
  out2 -= out1;
  out2 *= delay_time_LSB;
  out1 += out2 >> 9;
  if(toggle > 0) { 
    tmpS = out1<<15;
    tmpS -= filter_low_1L; // 31 bits + sign
    _m_s32xs32_s32(tmpS, low_gainL, tmpS, tmpS2);
    filter_low_1L += tmpS;
    audio_outL = (filter_low_1L<<1)^0x80000000;
  }
  else {
    audio_outL = (out1<<16)^0x80000000;
  }

  tmpS = inL;
  tmpS >>= 16;
  tmpS *= mod2>>18;
  tmpS >>= 8; 
  tmpS2 = inR;
  tmpS2 >>= 16;
  tmpS2 *= mod4>>18;
  tmpS2 >>= 8;
  tmpS += tmpS2;
  //tmpS >>= 1;
  tmpS += time2>>5;
  tmpS = max(0, tmpS);
  delay_time2 = tmpS;
  delay_time = min(delay_time2, 0x7FFE00); // Taile max = 14+9 = 23 bit
  if (!(toggle & 0b1)) { // 0 ou 2 
    tmpS = delay_time<<8;
    tmpS -= filter_low_2R; // 31 bits + sign
    _m_s32xs32_s32(tmpS, low_gainR, tmpS, tmpS2);
    filter_low_2R += tmpS;
    delay_time = (filter_low_2R>>8);
  }
  delay_time_LSB = delay_time & 0x1FF; // on garde les 9 bits de poids faible pour interpoler
  delay_time = delay_time >> 9; // on les suprime pour ne garder que l'index sur 15 bit (taille du buffer)
  read_point = (index_ecriture - delay_time) & 0x3FFF;
  out1 = delay_line.S16[read_point + 0x3FFF + 1];
  out2 = delay_line.S16[((read_point-1) & 0x3FFF) + 0x3FFF + 1];
  out2 -= out1;
  out2 *= delay_time_LSB;
  out1 += out2 >> 9;
  if(toggle > 0) { 
    tmpS = out1<<15;
    tmpS -= filter_low_1R; // 31 bits + sign
    _m_s32xs32_s32(tmpS, low_gainR, tmpS, tmpS2);
    filter_low_1R += tmpS;
    audio_outR = (filter_low_1R<<1)^0x80000000;
  }
  else {
    audio_outR = (out1<<16)^0x80000000;
  }
}

