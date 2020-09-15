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

// VCO LOOP : random value to generate a slowlly evolving LFO, or VCO
// Pot 1 : fq
// Pot 2 : Mod fq
// Pot 3 : nb step (1 .. 32)
// Pot 4 : nb step modulation 
// Pot 5 : mutate speed
// Pot 6 : mutation speed modulation
// CV 1 : fq (1V/Oct)
// CV 2 : fq modulation
// CV 3 : step modulation value
// CV 4 : Mutation speed modulation value
// IN 1 : gain
// IN 2 : pan
// Selecteur3 : interpolation : none / linear / bubic
// OUT 1 : OUT X
// OUT 2 : OUT Y
// LED 1 : CV2
// LED 2 : CV4

uint32_t current_step_nb, pos_old;

//  can be optimised to switch to 96KHz  ???

inline void VCO_LOOP_init_() {
  uint32_t i;
  send_dac(0x08,0b000000001); // sampling control (usb , 250fs,  48K, clock div 2, clk out, active)
  for (i=0; i<32; i++) { thomas[0][i] = random32(); }
  for (i=0; i<32; i++) { thomas[1][i] = random32(); }
  for (i=0; i<32; i++) { thomas[2][i] = random32(); }
  rnd_all_save = 0;
  init_chaos();
  current_step_nb = 1;
  pos_old = 0;
  freq_save = 0x1000;
}

inline void VCO_LOOP_loop_() {
  int32_t tmpS, CV3_value, CV4_value;
  uint32_t tmp, toggle;
  int32_t chaosX, chaosY, chaosZ;
  int32_t chaosdX, chaosdY, chaosdZ;
  uint32_t variation_speed;
  int32_t pot3_tmp, pot5_tmp;
  
  filter16_nozori_68
  test_connect_loop_68();
  toggle = get_toggle();
  toggle_global = toggle;
  chaos(15); // for default mod values

  if (CV3_connect < 60) CV3_value = CV_filter16_out[index_filter_cv3] - CV3_0V; else CV3_value = chaos_dy>>16;
  if (CV4_connect < 60) CV4_value = CV_filter16_out[index_filter_cv4] - CV4_0V; else CV4_value = chaos_dz>>16;
  CV3_value = min(0x7FFF,max(-0x7FFF,CV3_value));
  CV4_value = min(0x7FFF,max(-0x7FFF,CV4_value));

  // frequency control
  macro_fq_in
  freq += 48 << 18;
  macro_1VOct_CV1
  macro_FqMod_fine(pot2,CV2)
  macro_fq2increment
  increment_0 = increment1*3;
  
  // STEP
  pot3_tmp = CV_filter16_out[index_filter_pot3];
  tmpS = CV3_value;
  tmpS *= CV_filter16_out[index_filter_pot4];
  tmpS >>= 16;
  pot3_tmp += tmpS;
  pot3_tmp = min(0xFFFF, max(0,pot3_tmp));
  tmp = pot3_tmp >> 11; // 0..32
  nb_step = tmp+3;

  // variation speed
  pot5_tmp = CV_filter16_out[index_filter_pot5];
  tmpS = CV4_value;
  tmpS *= CV_filter16_out[index_filter_pot6];
  tmpS >>= 16;
  pot5_tmp += tmpS;
  pot5_tmp = min(0xFFFF, max(0,pot5_tmp));
  pot5_tmp *= pot5_tmp>>1;
  variation_speed = pot5_tmp >> 15;

  // chaos
  loop_index = (loop_index+1) % 35;
  chaosX = thomas[0][loop_index];
  chaosY = thomas[1][loop_index];
  chaosZ = thomas[2][loop_index];
  chaosdX = fast_sin(chaosY)^0x80000000;
  chaosdY = fast_sin(chaosZ)^0x80000000;
  chaosdZ = fast_sin(chaosX)^0x80000000;
  chaosdX >>= 17;
  chaosdX *= variation_speed;
  chaosdY >>= 17;
  chaosdY *= variation_speed;
  chaosdZ >>= 17;
  chaosdZ *= variation_speed;
  thomas[0][loop_index] += chaosdX>>5;
  thomas[1][loop_index] += chaosdY>>5;
  thomas[2][loop_index] += chaosdZ>>5;
  
  // LEDS
  led2((CV3_value+0x7FFF)>>7);
  led4((CV4_value+0x7FFF)>>7);
}

inline void VCO_LOOP_audio_() {
  uint32_t current_step, outX, outY, toggle;
  int32_t tmpS;
  uint32_t pos, interpol;
  uint32_t out, out2;
  
  VCO1_phase += increment_0;
  toggle = toggle_global;
  
  // step
  if (VCO1_phase < increment_0) { // debut de phase, pour ne changer de nb de step seulement au debut de phae
    current_step_nb =  nb_step;
  }
  current_step = current_step_nb;
  pos = VCO1_phase / (0xFFFFFFFF/current_step);
  interpol = (VCO1_phase * current_step) >> 16;
  if (pos > current_step) { // rounding error
    pos = current_step;
    interpol = 0xFFFF;
  }
  
  if (pos != pos_old) { 
    pos_old = pos;
    previous_interpol_valueX = last_interpol_valueX;
    previous_interpol_valueY = last_interpol_valueY;
    last_interpol_valueX = current_interpol_valueX;
    last_interpol_valueY = current_interpol_valueY;   
    current_interpol_valueX =  current_interpol_valueX2;
    current_interpol_valueY =  current_interpol_valueY2;
    current_interpol_valueX2 = abs((int32_t)thomas[0][pos]);
    current_interpol_valueY2 = abs((int32_t)thomas[1][pos]);
  }
  
  switch(toggle) { 
  case 0: // saw
    tmpS = current_interpol_valueX - 0x40000000;
    tmpS = tmpS >> 15;
    tmpS *= interpol;
    outX = 0x80000000 - current_interpol_valueX + tmpS;
    tmpS = current_interpol_valueY - 0x40000000;
    tmpS = tmpS >> 16;
    tmpS *= interpol;
    outY = 0x40000000 + tmpS;
    outX -= outX>>2;
    outX += 1<<28;
    outY -= outY>>2;
    outY += 1<<28;
    break;
  /*case 0: // saw
    tmpS = current_interpol_valueX - 0x40000000;
    tmpS = tmpS >> 16;
    tmpS *= interpol;
    outX = 0x40000000 + tmpS;
    tmpS = current_interpol_valueY - 0x40000000;
    tmpS = tmpS >> 16;
    tmpS *= interpol;
    outY = 0x40000000 + tmpS;
    outX -= outX>>2;
    outX += 1<<28;
    outY -= outY>>2;
    outY += 1<<28;
    break; */
  case 1: // no interpolation (square)
    outX = current_interpol_valueX;
    outY = current_interpol_valueY;
    outX -= outX>>2;
    outX += 1<<28;
    outY -= outY>>2;
    outY += 1<<28;
    break;
/*  case 1: // linear interpolation
    tmpS = current_interpol_valueX - last_interpol_valueX;
    tmpS = tmpS >> 16;
    tmpS *= interpol;
    outX = last_interpol_valueX + tmpS;
    tmpS = current_interpol_valueY - last_interpol_valueY;
    tmpS = tmpS >> 16;
    tmpS *= interpol;
    outY = last_interpol_valueY + tmpS;
    outX -= outX>>2;
    outX += 1<<28;
    outY -= outY>>2;
    outY += 1<<28;
    break;
*/
  case 2: // cubic interpolation
    outX = tabread4(previous_interpol_valueX>>17, last_interpol_valueX>>17, current_interpol_valueX>>17, current_interpol_valueX2>>17, interpol);
    outX *= 90000;
    outX += (((65535*2) - 90000)/2)<<14;
    outY = tabread4(previous_interpol_valueY>>17, last_interpol_valueY>>17, current_interpol_valueY>>17, current_interpol_valueY2>>17, interpol);    
    outY *= 90000;
    outY += (((65535*2) - 90000)/2)<<14;
    break;   
  }
  // Les gains on ete fait plus clean sur RND_LOOP (changer si necessaire...)
  out  = outX<<1;
  out2 = outY<<1;
  macro_out_gain_pan_stereo2
}

