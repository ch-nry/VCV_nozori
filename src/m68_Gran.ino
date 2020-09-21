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

// granulator
// Pot 1 : Grain size
// Pot 2 : mod (Rnd)
// Pot 3 : Detune
// Pot 4 : MOD (Rnd)
// Pot 5 : Pos 
// Pot 6 : Mod (Rnd)
// CV 1 : Trig 1 grain (continous trig if unplug)
// CV 2 : Mod Size
// CV 3 : Mod Detune
// CV 4 : Mod Pos
// IN 1 : audio in1
// IN 2 : Pan
// Selecteur3 : mode : normal / hold / backward
// OUT 1 : OUT L
// OUT 2 : OUT R

inline void Gran_init_() {
  uint32_t i;
  // Switch to 48KHz
  send_dac(0x08,0b000000001); // sampling control (usb , 250fs,  48K, clock div 2, clk out, active)
  start_new_grain_old = false;
  start_new_grain = false;
  grain1_used = false;
  grain2_used = false;  
  init_chaos();
  for (i=0; i<Max_Delay;i++) {
    delay_line.S16[i] = 0;
  }
}

inline void Gran_loop_() { 
  int32_t offset_local, speed_local, size_local;
  int32_t tmpS, CV2_value, CV3_value, CV4_value;
  uint32_t freq;
  uint32_t offset_min, offset_max;
  uint32_t toggle;
  
  filter16_nozori_68
  test_connect_loop_68();
  chaos(10); // for default mod values
  toggle = get_toggle();
  toggle_global = toggle;

  if (CV2_connect < 60) CV2_value = CV_filter16_out[index_filter_cv2] - CV2_0V; else CV2_value = chaos_dx>>16;
  if (CV3_connect < 60) CV3_value = CV_filter16_out[index_filter_cv3] - CV3_0V; else CV3_value = chaos_dy>>16;
  if (CV4_connect < 60) CV4_value = CV_filter16_out[index_filter_cv4] - CV4_0V; else CV4_value = chaos_dz>>16;
  CV2_value = min(0x7FFF,max(-0x7FFF,CV2_value));
  CV3_value = min(0x7FFF,max(-0x7FFF,CV3_value));
  CV4_value = min(0x7FFF,max(-0x7FFF,CV4_value));
  
  // grain size
  size_local = CV_filter16_out[index_filter_pot1];
  tmpS = CV2_value>>1;
  tmpS *= CV_filter16_out[index_filter_pot2];
  tmpS >>= 16;
  size_local += tmpS;
  size_local = min(max(0, size_local), 0xFFFF);

  // grain size de 10 a 100ms // 480 a 4096 echantillons // freq2increment[140] a freq2increment[288]
  freq = ((288-140) * size_local ) << 2; // 18 bit de LSB sur interpolation - 16 de la taille de speed local
  freq += 140 << 18;
  macro_fq2increment // 18 bit de LSB
  size_local = increment1;
    
  // speed (12bit de LSB)
  speed_local = CV_filter16_out[index_filter_pot3];
  tmpS = CV3_value>>1;
  tmpS *= CV_filter16_out[index_filter_pot4];
  tmpS >>= 16;
  speed_local += tmpS;
  speed_local = min(max(0, speed_local), 0xFFFF);
  // standard speed = 0x1000
  // scaling :
  // 0x0000 -> 0x1000 / 4 = 0x0400 = 1024    = freq2increment[193]
  // 0x7FFF -> 0x1000               = 4096
  // 0xFFFF -> 0x1000 * 4 = 0x4000 = 16000  = freq2increment[384]
  freq = ((384-193) * speed_local ) <<(18-16); // 18 bit de LSB sur interpolation - 16 de la taille de speed local
  freq += 193 <<18;
  macro_fq2increment_novar // 18 bit de LSB
  speed_local = increment1;

  // grain offset position
  // 12 bit de LSB
  offset_local = CV_filter16_out[index_filter_pot5];
  tmpS = CV4_value;
  tmpS *= CV_filter16_out[index_filter_pot6];
  tmpS >>= 16;
  offset_local += tmpS;
  offset_local = min(max(0, offset_local), 0xFFFF);
  offset_local >>= 1;
  switch (toggle) {
  case 0 : // lecture normal
    offset_local >>= 1;
    offset_local -= offset_local >> 4;
    offset_min = (size_local * max(0,speed_local-0x1000))>>12;
    offset_max = Max_Delay - ((size_local*3)/4);
    offset_local = min(offset_max, offset_local);
    offset_local = max(offset_min, offset_local);
    // grain de 100ms (4800 samples), lut 4 fois plus vite : on ratrape 3*4800, il faut dc que l'offfset soit au minimum de 14400
    // grain de 100ms lut 4 fois plus doucement, on perd 3/4 de 4800, soit 3600 point. il faut dc que l'offset max soit de taille_max-3600
  break;
  case 1 : // pas d'ecriture, il faut juste eviter de croiser le debut du buffer 
    offset_local >>= 1;
    offset_min = ((size_local * speed_local)>>12); 
    offset_max = Max_Delay;
    offset_local = min(offset_max, offset_local);
    offset_local = max(offset_min, offset_local);
    // offset min doit etre de la taille du grain
    // offset max doit etre : taille max 
  break;
  case 2 :
    offset_local >>= 1;
    offset_min = size_local + ((size_local * speed_local)>>12); 
    offset_max = Max_Delay ;
    offset_local = min(offset_max, offset_local);
    offset_local = max(offset_min, offset_local); 
  break;
  }
  
  if ( !start_new_grain_old && (CV1_connect < 60) && (REG_CV1 > 0xA00) ) { // on a un front montant
    start_new_grain_old = true;
    start_new_grain = true;
  }
  else  if ( start_new_grain_old && (CV1_connect < 60) && (REG_CV1 < 0x900) ) { // front descendant
    start_new_grain_old = false;
  }
  if ( (!(CV1_connect < 60)) && (!grain1_used) ) start_new_grain = true;
  else if ( (!(CV1_connect < 60)) && (grain1_used) && (!grain2_used) && ((grain1_pos >> 11) > grain1_size) ) start_new_grain = true;

  if ((start_new_grain) && (!grain1_used) ) {
    start_new_grain = false;
    grain1_size = size_local;
    grain1_speed = speed_local;
    grain1_offset = ((index_ecriture - offset_local) & Max_Delay)<<12;
    grain1_pos = 0;
    grain1_used = true;
  } 
  else 
  if ((start_new_grain) && (!grain2_used) ) { // il faut demarer un nvx grain, le 2est dispo
    start_new_grain = false;
    grain2_size = size_local;
    grain2_speed = speed_local;
    grain2_offset = ((index_ecriture - offset_local) & Max_Delay)<<12;
    grain2_pos = 0;
    grain2_used = true;
  }
  led2((CV2_value+0x7FFF)>>7);
  led4((CV3_value+0x7FFF)>>7);
}

inline void Gran_audio_() {
  int32_t tmpS;
  uint32_t pos_relative_MSB;
  uint32_t delay_time_LSB, delay_time_MSB, read_point, out, envelope;
  int32_t  out1, out2, outS;
  uint32_t toggle;

  toggle = toggle_global;
  
  // write audio in  to the buffer
  if (toggle == 0) { // ecriture normal ds le buffer
    index_ecriture = (index_ecriture+1) & Max_Delay;
  } 
  else if (toggle == 2) { // ecriture inversé ds le buffer
    index_ecriture = (index_ecriture-1) & Max_Delay;
  }
  
  tmpS = (IN1_connect < 60)? audio_inL^0x80000000 : 0; 
  tmpS >>= 16;
  delay_line.S16[index_ecriture] = tmpS;
  outS = 0;
  
  if (grain1_used) {
    // position relative dans le grain, pour le calcul de l'envelope
    // ==  grain1_pos / grain1_size
    // grain1_pos a un offset de 12 bit, pour un max sur 14 bit soit un total de 26 bit, on peux dc le decaler au max de 6 bit
    // grain1_size a un max de 12 bit
    pos_relative_MSB = (grain1_pos << 6) / grain1_size; // 12 + 14 + 6 -12 = 20 bit de precision sur la position
    envelope = fast_sin(pos_relative_MSB<<13); // envelope du gain
    envelope = (envelope < 0x7FFFF801)? 0: envelope - 0x7FFFF801;
    envelope >>= 15;
    envelope *= envelope; // envelope = cos² 
    envelope >>= 16;

    delay_time_LSB = grain1_offset & 0xFFF; // on garde les 12 bits de poids faible pour interpoler
    delay_time_MSB = grain1_offset >> 12; // on les suprime pour ne garder que l'index sur 15 bit (taille du buffer)
    read_point = delay_time_MSB & Max_Delay;
    out1 = delay_line.S16[read_point];
    out2 = delay_line.S16[(read_point+1) & Max_Delay];
    out2 -= out1;
    out2 *= delay_time_LSB;
    out1 += out2 >> 12;
    out1 *= envelope;
    outS += out1>>1;

    grain1_pos += 0x1000;
    grain1_offset += grain1_speed; 
    
    if ((grain1_pos >> 12) > grain1_size) grain1_used = false; // on arrive a la fin du grain
  }

  if (grain2_used) {
    pos_relative_MSB = (grain2_pos << 6) / grain2_size; 
    envelope = fast_sin(pos_relative_MSB<<13); // envelope du gain
    envelope = (envelope < 0x7FFFF801)? 0: envelope - 0x7FFFF801;
    envelope >>= 15;
    envelope *= envelope; // envelope = cos² 
    envelope >>= 16;
    
    delay_time_LSB = grain2_offset & 0xFFF; // on garde les 12 bits de poinds faible pour interpoler
    delay_time_MSB = grain2_offset >> 12; // on les suprime pour ne garder que l'index sur 15 bit (taille du buffer)
    read_point = delay_time_MSB & Max_Delay;
    out1 = delay_line.S16[read_point];
    out2 = delay_line.S16[(read_point+1) & Max_Delay];
    out2 -= out1;
    out2 *= delay_time_LSB;
    out1 += out2 >> 12;
    out1 *= envelope;
    outS += out1>>1;

    grain2_pos += 0x1000;
    grain2_offset += grain2_speed; 
    
    if ((grain2_pos >> 12) > grain2_size) grain2_used = false; // on arrive a la fin du grain
  }
  //  outS += outS>>1; // clip if to much gain
  out = outS^0x80000000;
  macro_out_pan
}

