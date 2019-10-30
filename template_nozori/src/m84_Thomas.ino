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

//Thomas chaotic attractor used as LFO or audio source
// Pot 1 : caracteristic frequency of attractor 1
// Pot 2 : caracteristic frequency of attractor 2
// Pot 3 : Mod frequency
// Pot 4 : Mod frequency
// Pot 5 : B : damping parametter of the attractor 1
// Pot 6 : B : damping parametter of the attractor 2
// Pot 7 : Gain of attract6r 1
// Pot 8 : Gain of attractor 2
// IN 1 : speed 1
// IN 2 : speed 2
// Selecteur3 : Fq range : VCO / clock / lfo
// OUT 1 : OUT X
// OUT 2 : OUT Y

int32_t thomasXa, thomasYa, thomasZa, global_speed;
int32_t thomasXb, thomasYb, thomasZb;
uint32_t f1, f2, b1, b2;

inline void THOMAS_init_() {
  //send_dac(0x08,0b000000001); // sampling control (usb , 250fs,  48K, clock div 2, clk out, active)
  thomasXa = random32()>>2; // X
  thomasYa = random32()>>2; // Y
  thomasZa = random32()>>2; // Z
  thomasXb = random32()>>2; // X
  thomasYb = random32()>>2; // Y
  thomasZb = random32()>>2; // Z
}

inline void THOMAS_loop_() {
  uint32_t toggle;
  int32_t tmpS;
  int32_t pot1_tmp, pot5_tmp;
  int32_t freq;
  
  filter16_nozori_84
  test_connect_loop_84();
  toggle = get_toggle();

  pot1_tmp = CV_filter16_out[index_filter_pot1];
  freq = pot1_tmp * (420*4); // max (avec modulation) : 470 << 18
  macro_FqMod_fine_IN1(pot3)
  macro_fq2increment
  f1 = min(0xFFFF,increment1);

  pot5_tmp = CV_filter16_out[index_filter_pot2];
  freq = pot5_tmp * (420*4); // max (avec modulation) : 470 << 18
  macro_FqMod_fine_IN2(pot4)
  macro_fq2increment_novar
  f2 = min(0xFFFF,increment1);

  b1 = (655360 + 15*(0xFFFF-CV_filter16_out[index_filter_pot5]))>>5; // sur 15 bits
  b2 = (655360 + 15*(0xFFFF-CV_filter16_out[index_filter_pot6]))>>5; // sur 15 bits

  switch(toggle) {
    case 0: global_speed = 2;
    break;
    case 1: global_speed = 6;
    break;
    case 2: global_speed = 13;
    break;    
  }
  led2(audio_outL>>23);
  led4(audio_outR>>23);  
}
 
inline void THOMAS_audio_() {
  int32_t dx, dy, dz, tmpS;
  uint32_t gain1, gain2, tmp;
  int32_t chaos_X_local,chaos_Y_local, chaos_Z_local;
  uint32_t f1_local, f2_local, b1_local, b2_local;
  uint32_t global_speed_local;

  global_speed_local = global_speed;
  f1_local = f1;
  f2_local = f2;
  b1_local = b1;
  b2_local = b2;

  // chaos1 ////////////////////////////////////////////////////
  // chaos_n : 1 sur 1<<30
  //1st thomas attrator
  chaos_X_local = thomasXa;
  chaos_Y_local = thomasYa;
  chaos_Z_local = thomasZa;

  tmp = chaos_Y_local;
  dx = fast_sin(tmp<<2)^0x80000000; // 2^16/2pi
  dx >>= 1; // de -1 a 1
  tmpS = chaos_X_local >> 15;
  tmpS *= b1_local;
  dx -= tmpS;
  dx >>= 16;  // dt
  dx *= f1_local;
  dx >>= global_speed_local;
  
  tmp = chaos_Z_local;
  dy = fast_sin(tmp<<2)^0x80000000; // 2^16/2pi
  dy >>= 1; // de -1 a 1
  tmpS = chaos_Y_local >> 15;
  tmpS *= b1_local;
  dy -= tmpS;
  dy >>= 16;
  dy *= f1_local;
  dy >>= global_speed_local;
  
  tmp = chaos_X_local;
  dz = fast_sin(tmp<<2)^0x80000000; // 2^16/2pi
  dz >>= 1; // de -1 a 1
  tmpS = chaos_Z_local >> 15;
  tmpS *= b1_local;
  dz -= tmpS;
  dz >>= 16;
  dz *= f1_local;
  dz >>= global_speed_local;
  
  thomasXa = chaos_X_local + dx;
  thomasYa = chaos_Y_local + dy;
  thomasZa = chaos_Z_local + dz;

  // chaos2 ////////////////////////////////////////////////////
  // chaos_n : 1 sur 1<<30
  chaos_X_local = thomasXb;
  chaos_Y_local = thomasYb;
  chaos_Z_local = thomasZb;

  tmp = chaos_Y_local;
  dx = fast_sin(tmp<<2)^0x80000000; // 2^16/2pi
  dx >>= 1; // de -1 a 1
  tmpS = chaos_X_local >> 15;
  tmpS *= b2_local;
  dx -= tmpS;
  dx >>= 16;  // dt
  dx *= f2_local;
  dx >>= global_speed_local;
  
  tmp = chaos_Z_local;
  dy = fast_sin(tmp<<2)^0x80000000; // 2^16/2pi
  dy >>= 1; // de -1 a 1
  tmpS = chaos_Y_local >> 15;
  tmpS *= b2_local;
  dy -= tmpS;
  dy >>= 16;
  dy *= f2_local;
  dy >>= global_speed_local;
  
  tmp = chaos_X_local;
  dz = fast_sin(tmp<<2)^0x80000000; // 2^16/2pi
  dz >>= 1; // de -1 a 1
  tmpS = chaos_Z_local >> 15;
  tmpS *= b2_local;
  dz -= tmpS;
  dz >>= 16;
  dz *= f2_local;
  dz >>= global_speed_local;
  
  thomasXb = chaos_X_local + dx;
  thomasYb = chaos_Y_local + dy;
  thomasZb = chaos_Z_local + dz;

  gain1 = CV_filter16_out[index_filter_pot7]>>1;
  gain2 = CV_filter16_out[index_filter_pot8]>>1;
  
  tmpS = (thomasXa>>15) * gain1;
  tmpS += (thomasYb>>15) * gain2;
  tmpS = min(max(-0x3FFFFFFF, tmpS),  0x3FFFFFFF);
  tmpS <<= 1;
  audio_outL = tmpS^0x80000000;
  
  tmpS = (thomasYa>>15) * gain1;
  tmpS += (thomasZb>>15) * gain2;
  tmpS = min(max(-0x3FFFFFFF, tmpS),  0x3FFFFFFF);
  tmpS <<= 1;
  audio_outR = tmpS^0x80000000;
}

