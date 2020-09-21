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

// dual pitch shifter
// Pot 1 : FQ 1
// Pot 2 : FQ 2
// Pot 3 : FQ modulation 1
// Pot 4 : FQ modulation 2
// Pot 5 : size
// Pot 6 : FQ modulation 3
// CV 1 : Pitch 1 (1V/Oct)
// CV 2 : Mod Fq 1
// CV 3 : pod Fq 2
// CV 4 : Mod Fq 3
// IN 1 : in 1
// IN 2 : in 2
// Selecteur3 : mode : stereo / oposit / mono + pan
// OUT 1 : OUT L
// OUT 2 : OUT R

// stereo mod : FQ1 / CV1 / CV2*pot3 / CV4*pot6 ///// FQ2 / CV1 / CV3*pot4 / CV4*pot6
// oposit mod : FQ1 / CV1 / CV2*pot3 / CV4*pot6 ///// FQ2 / CV1 / CV3*pot4 / -CV4*pot6
// Mono mod : Q1 / CV1 / CV2*pot3 / CV3*pot4 / -CV4*pot6

uint32_t delay_line_size_global;

inline void PShift_init_() {
  uint32_t i;
  // Switch to 48KHz
  send_dac(0x08,0b000000001); // sampling control (usb , 250fs,  48K, clock div 2, clk out, active)

  start_new_grain = false;
  grain1_used = false;
  grain2_used = false; 
  init_chaos();
  for (i=0; i<Max_Delay;i++) {
    delay_line.S16[i] = 0;
  }
}

inline void PShift_loop_() { 
  int32_t offset_local1, offset_local2, speed_local, speed_local2, size_local;
  int32_t tmpS, CV1_value, CV2_value, CV3_value, CV4_value;
  uint32_t freq;
  uint32_t toggle, delay_line_size;
  
  filter16_nozori_68
  test_connect_loop_68();
  toggle = get_toggle();
  toggle_global = toggle;
  chaos(15); // for default mod values
    
  if (CV1_connect < 60) CV1_value = CV_filter16_out[index_filter_cv1] - CV1_0V; else CV1_value = 0;
  if (CV2_connect < 60) CV2_value = CV_filter16_out[index_filter_cv2] - CV2_0V; else CV2_value = chaos_dx>>16;
  if (CV3_connect < 60) CV3_value = CV_filter16_out[index_filter_cv3] - CV3_0V; else CV3_value = chaos_dy>>16;
  if (CV4_connect < 60) CV4_value = CV_filter16_out[index_filter_cv4] - CV4_0V; else CV4_value = chaos_dz>>16;
  CV2_value = min(0x7FFF,max(-0x7FFF,CV2_value));
  CV3_value = min(0x7FFF,max(-0x7FFF,CV3_value));
  CV4_value = min(0x7FFF,max(-0x7FFF,CV4_value));

  led2((CV2_value+0x7FFF)>>7);
  led4((CV3_value+0x7FFF)>>7);
  
  // grain size
  size_local = CV_filter16_out[index_filter_pot5];
  // grain size de 480 a 4096 echantillons // freq2increment[140] a freq2increment[288]
  freq = ((288-140) * size_local ) << 2; // 18 bit de LSB sur interpolation - 16 de la taille de speed local
  freq += 140 << 18;
  macro_fq2increment // 18 bit de LSB
  size_local = increment1;

  switch(toggle) {
  case 0: // Stereo
    delay_line_size = 0x3FFF;
    speed_local = CV_filter16_out[index_filter_pot1];
    tmpS = (CV1_value * (0xFFF0/4)) / (int32_t)CV1_1Vminus0V;
    speed_local += tmpS;
    tmpS = CV2_value>>1;
    tmpS *= CV_filter16_out[index_filter_pot3];
    tmpS >>= 16;
    speed_local += tmpS;
    tmpS = CV4_value>>1;
    tmpS *= CV_filter16_out[index_filter_pot6];
    tmpS >>= 16;
    speed_local += tmpS;
    speed_local = min(max(0, speed_local), 0xFFFF);
    // standard speed = 0x1000
    // scaling :
    // 0x0000 -> 0x1000 / 4 = 0x0400 = 1024    = freq2increment[192] 
    // 0x7FFF -> 0x1000               = 4096
    // 0xFFFF -> 0x1000 * 4 = 0x4000 = 16000  = freq2increment[384]
    freq = ((384-193) * speed_local ) <<(18-16); // 18 bit de LSB sur interpolation - 16 de la taille de speed local
    freq += 193 <<18;
    macro_fq2increment_novar // 18 bit de LSB
    speed_local = increment1;

    speed_local2 = CV_filter16_out[index_filter_pot2];
    tmpS = (CV1_value * (0xFFF0/4)) / (int32_t)CV1_1Vminus0V;
    speed_local2 += tmpS;
    tmpS = CV3_value>>1;
    tmpS *= CV_filter16_out[index_filter_pot4];
    tmpS >>= 16;
    speed_local2 += tmpS;
    tmpS = CV4_value>>1;
    tmpS *= CV_filter16_out[index_filter_pot6];
    tmpS >>= 16;
    speed_local2 += tmpS;
    speed_local2 = min(max(0, speed_local2), 0xFFFF);
    freq = ((384-193) * speed_local2 ) <<(18-16); // 18 bit de LSB sur interpolation - 16 de la taille de speed local
    freq += 193 <<18;
    macro_fq2increment_novar // 18 bit de LSB
    speed_local2 = increment1;
  break;
  case 1: // Stereo with oposite modulation
    delay_line_size = 0x3FFF;
    speed_local = CV_filter16_out[index_filter_pot1];
    tmpS = (CV1_value * (0xFFF0/4)) / (int32_t)CV1_1Vminus0V;
    speed_local += tmpS;
    tmpS = CV2_value>>1;
    tmpS *= CV_filter16_out[index_filter_pot3];
    tmpS >>= 16;
    speed_local += tmpS;
    tmpS = CV4_value>>1;
    tmpS *= CV_filter16_out[index_filter_pot6];
    tmpS >>= 16;
    speed_local += tmpS;
    speed_local = min(max(0, speed_local), 0xFFFF);
    // standard speed = 0x1000
    // scaling :
    // 0x0000 -> 0x1000 / 4 = 0x0400 = 1024    = freq2increment[192] 
    // 0x7FFF -> 0x1000               = 4096
    // 0xFFFF -> 0x1000 * 4 = 0x4000 = 16000  = freq2increment[384]
    freq = ((384-193) * speed_local ) <<(18-16); // 18 bit de LSB sur interpolation - 16 de la taille de speed local
    freq += 193 <<18;
    macro_fq2increment_novar // 18 bit de LSB
    speed_local = increment1;

    speed_local2 = CV_filter16_out[index_filter_pot2];
    tmpS = (CV1_value * (0xFFF0/4)) / (int32_t)CV1_1Vminus0V;
    speed_local2 += tmpS;
    tmpS = CV3_value>>1;
    tmpS *= CV_filter16_out[index_filter_pot4];
    tmpS >>= 16;
    speed_local2 += tmpS;
    tmpS = CV4_value>>1;
    tmpS *= CV_filter16_out[index_filter_pot6];
    tmpS >>= 16;
    speed_local2 -= tmpS;
    speed_local2 = min(max(0, speed_local2), 0xFFFF);
    freq = ((384-193) * speed_local2 ) <<(18-16); // 18 bit de LSB sur interpolation - 16 de la taille de speed local
    freq += 193 <<18;
    macro_fq2increment_novar // 18 bit de LSB
    speed_local2 = increment1;
  break;
  case 2 : // MONO : the delay line is used for 1 single data, so it is longer, the pitch shifter can go lower and higher
    delay_line_size = 0x7FFF;
    // speed (12bit de LSB)
    speed_local = CV_filter16_out[index_filter_pot1];         // master
    tmpS = CV_filter16_out[index_filter_pot2] - 0x7FF0;       // fine
    tmpS >>= 5;
    speed_local += tmpS;
    tmpS = (CV1_value * (0xFFF0/4)) / (int32_t)CV1_1Vminus0V; // CV1 en 1V/Oct
    speed_local += tmpS;
    tmpS = CV2_value>>1;                                      // CV2 * pot3
    tmpS *= CV_filter16_out[index_filter_pot3];
    tmpS >>= 16;
    speed_local += tmpS;
    tmpS = CV3_value>>1;                                      // CV3 * pot4
    tmpS *= CV_filter16_out[index_filter_pot4];
    tmpS >>= 16;
    speed_local += tmpS;
    tmpS = CV4_value>>1;                                      // CV4 * pot6
    tmpS *= CV_filter16_out[index_filter_pot6];
    tmpS >>= 16;
    speed_local -= tmpS;
    speed_local = min(max(0, speed_local), 0xFFFF);
    // standard speed = 0x1000
    // scaling :
    // 0x0000 -> 0x1000 / 8 = 0x0200 = 512    = freq2increment[145]
    // 0x7FFF -> 0x1000              = 4096
    // 0xFFFF -> 0x1000 * 8 = 0x8000 = 32000  = freq2increment[433]
    freq = ((433-145) * speed_local ) <<(18-16); // 18 bit de LSB sur interpolation - 16 de la taille de speed local
    freq += 145 <<18;
    macro_fq2increment_novar // 18 bit de LSB
    speed_local = increment1;
  break;
  }
  delay_line_size_global = delay_line_size;
  
  // grain offset position
  // 12 bit de LSB
  offset_local1 = (size_local * max(0,speed_local-0x1000))>>12;
  offset_local2 = (size_local * max(0,speed_local2-0x1000))>>12;
  
  if ( (grain1_used) && (!grain2_used) && ((grain1_pos >> 11) > grain1_size) ) start_new_grain = true;
  if ( (!grain1_used) && (grain2_used) && ((grain2_pos >> 11) > grain2_size) ) start_new_grain = true;
  if ( (!grain1_used) && (!grain2_used) ) start_new_grain = true;

  if ((start_new_grain) && (!grain1_used) ) {
    start_new_grain = false;
    grain1_size = size_local;
    grain1_speed = speed_local;
    grain1_speed2 = speed_local2;
    grain1_offset  = ((index_ecriture - offset_local1) & delay_line_size)<<12;
    grain1_offset2 = ((index_ecriture - offset_local2) & delay_line_size)<<12;
    grain1_pos = 0;
    grain1_used = true;
  }
  
  if ((start_new_grain) && (!grain2_used) ) { // il faut demarer un nvx grain, le 2est dispo
    start_new_grain = false;
    grain2_size = size_local;
    grain2_speed = speed_local;
    grain2_speed2 = speed_local2;
    grain2_offset  = ((index_ecriture - offset_local1) & delay_line_size)<<12;
    grain2_offset2 = ((index_ecriture - offset_local2) & delay_line_size)<<12;
    grain2_pos = 0;
    grain2_used = true;
  }
}

inline void PShift_audio_() {
  int32_t tmpS, inL;
  uint32_t pos_relative_MSB;
  uint32_t delay_time_LSB, delay_time_MSB, read_point, out, envelope, envelope1, envelope2;
  int32_t  out1, out2, outS;
  uint32_t toggle, delay_line_size;

  toggle = toggle_global;
  delay_line_size = delay_line_size_global;
  
  // write audio in  to the buffer
  if (IN1_connect < 60) {
     tmpS = audio_inL^0x80000000;
  } else {
     tmpS = 0;
  }
  inL = tmpS;
  tmpS >>= 16;
  index_ecriture = (index_ecriture+1) & delay_line_size;
  delay_line.S16[index_ecriture] = tmpS;
  outS = 0;
  
  if (grain1_used) {
    // position relative dans le grain, pour la calcul de l'envelope
    // ==  grain1_pos / grain1_size
    // grain1_pos a un offset de 12 bit, pour un max sur 14 bit soit un total de 26 bit, on peux dc le decaler au max de 6 bit
    // grain1_size a un max de 12 bit
    pos_relative_MSB = (grain1_pos << 6) / grain1_size; // 12 + 14 + 6 -12 = 20 bit de precision sur la position
    envelope = fast_sin(pos_relative_MSB<<13); // envelope du gain
    envelope = (envelope < 0x7FFFF801)? 0: envelope - 0x7FFFF801;
    envelope >>= 15;
    envelope *= envelope; // envelope = cos² 
    envelope >>= 16;
    envelope1 = envelope;
    
    delay_time_LSB = grain1_offset & 0xFFF; // on garde les 12 bits de poinds faible pour interpoler
    delay_time_MSB = grain1_offset >> 12; // on les suprime pour ne garder que l'index sur 15 bit (taille du buffer)
    read_point = delay_time_MSB & delay_line_size;
    out1 = delay_line.S16[read_point];
    out2 = delay_line.S16[(read_point+1) & delay_line_size];
    out2 -= out1;
    out2 *= delay_time_LSB;
    out1 += out2 >> 12;
    out1 *= envelope;
    outS += out1>>1;

    grain1_offset += grain1_speed; 
  }

  if (grain2_used) {
    pos_relative_MSB = (grain2_pos << 6) / grain2_size; 
    envelope = fast_sin(pos_relative_MSB<<13); // envelope du gain
    envelope = (envelope < 0x7FFFF801)? 0: envelope - 0x7FFFF801;
    envelope >>= 15;
    envelope *= envelope; // envelope = cos² 
    envelope >>= 16;
    envelope2 = envelope;

    delay_time_LSB = grain2_offset & 0xFFF; // on garde les 12 bits de poinds faible pour interpoler
    delay_time_MSB = grain2_offset >> 12; // on les suprime pour ne garder que l'index sur 15 bit (taille du buffer)
    read_point = delay_time_MSB & delay_line_size;
    out1 = delay_line.S16[read_point];
    out2 = delay_line.S16[(read_point+1) & delay_line_size];
    out2 -= out1;
    out2 *= delay_time_LSB;
    out1 += out2 >> 12;
    out1 *= envelope;
    outS += out1>>1;

    grain2_offset += grain2_speed; 
  }
  outS <<= 1;
  audio_outL = outS^0x80000000;
  
  switch (toggle) {
    case 0:
    case 1:
      // write audio in  to the buffer
      if (IN2_connect < 60) {
         tmpS = audio_inR^0x80000000;
      } else {
         tmpS = inL;
      }
      tmpS >>= 16;
      delay_line.S16[index_ecriture + delay_line_size + 1] = tmpS;
      outS = 0;
      if (grain1_used) {
        envelope = envelope1;
        delay_time_LSB = grain1_offset2 & 0xFFF; // on garde les 12 bits de poinds faible pour interpoler
        delay_time_MSB = grain1_offset2 >> 12; // on les suprime pour ne garder que l'index sur 15 bit (taille du buffer)
        read_point = delay_time_MSB & delay_line_size;
        read_point += delay_line_size + 1;
        out1 = delay_line.S16[read_point];
        read_point = (delay_time_MSB + 1) & delay_line_size;
        read_point += delay_line_size + 1;
        out2 = delay_line.S16[read_point];
        out2 -= out1;
        out2 *= delay_time_LSB;
        out1 += out2 >> 12;
        out1 *= envelope;
        outS += out1>>1;
    
        grain1_offset2 += grain1_speed2; 
      }
    
      if (grain2_used) {
        envelope = envelope2;

        delay_time_LSB = grain2_offset2 & 0xFFF; // on garde les 12 bits de poinds faible pour interpoler
        delay_time_MSB = grain2_offset2 >> 12; // on les suprime pour ne garder que l'index sur 15 bit (taille du buffer)
        read_point = delay_time_MSB & delay_line_size;
        read_point += delay_line_size + 1;
        out1 = delay_line.S16[read_point];
        read_point = (delay_time_MSB + 1) & delay_line_size;
        read_point += delay_line_size + 1;
        out2 = delay_line.S16[read_point];
        out2 -= out1;
        out2 *= delay_time_LSB;
        out1 += out2 >> 12;
        out1 *= envelope;
        outS += out1>>1;
    
        grain2_offset2 += grain2_speed2; 
      }  
      outS <<= 1;
      audio_outR = outS^0x80000000;
    break;
    case 2:
      out = outS^0x80000000;
      macro_out_pan
    break;
  }

  grain1_pos += 0x1000;
  if ((grain1_pos >> 12) > grain1_size) {
    grain1_used = false; // on arrive a la fin du grain
    grain1_pos = grain1_size<<12; // pour eviter des overflow eventuel ; ne devrait pas servir car le grain doit jouer en permanence
  }
  grain2_pos += 0x1000; 
  if ((grain2_pos >> 12) > grain2_size) {
    grain2_used = false; // on arrive a la fin du grain
    grain2_pos = grain2_size<<12; // pour eviter des overflow eventuel ; ne devrait pas servir car le grain doit jouer en permanence
  }
}

