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

//CV REC
// Pot 1 : frequency
// Pot 2 : Mod fq
// Pot 3 : audio input gain
// Pot 4 : MOD gain
// Pot 5 : decay
// Pot 6 : low pass
// CV 1 : fq (1V/Oct)
// CV 2 : fq modulation value
// CV 3 : gain modulation value
// CV 4 : noise input gain
// IN 1 : audio in1
// IN 2 : gain
// Selecteur3 : input type : normal / diff / sin(diff)
// OUT 1 : OUT CV1
// OUT 2 : OUT CV2

uint32_t pluck, pluck2, pluck2_save;

inline void KS_init_() {
  uint32_t i;
  filter_audio_out = 0;
  init_chaos();
  for (i=0; i<Max_Delay;i++) {
    delay_line.S16[i] = 0;
  }
  pluck2_save = 0;
  pluck = 0;
  pluck2 = 0;
}

inline void KS_loop_() {
  uint32_t FB;
  int32_t tmpS, CV2_value, CV3_value;
  int32_t freq, gain;
  uint32_t toggle;
  
  filter16_nozori_68
  test_connect_loop_68();
  chaos(15); // for default mod values
  toggle = get_toggle();
  toggle_global = toggle;
  
  if (CV2_connect < 60) CV2_value = CV_filter16_out[index_filter_cv2] - CV2_0V; else CV2_value = chaos_dy >> 16;
  if (CV3_connect < 60) CV3_value = CV_filter16_out[index_filter_cv3] - CV3_0V; else CV3_value = chaos_dx >> 16;
  CV2_value = min(0x7FFF,max(-0x7FFF,CV2_value));
  CV3_value = min(0x7FFF,max(-0x7FFF,CV3_value));
    
  led2((CV2_value+0x7FFF)>>7);
  led4((CV3_value+0x7FFF)>>7);
  
  // delay time
  freq = (0xFFF0-CV_filter16_out[index_filter_pot1])<<11;
  freq += 0x06000000;
  tmpS = CV2_value;
  tmpS *= CV_filter16_out[index_filter_pot2]>>1;
  tmpS >>= 4;
  freq -= tmpS; // freq is used as time...
  
  if (CV1_connect < 60) {
    tmpS = CV_filter16_out[index_filter_CV1];
    tmpS -= CV1_0V;
    tmpS *= CV1_1V;
    freq -= tmpS;
  }
  
  macro_fq2increment 
  delay_time_global = increment1;

  // gain
  gain = CV_filter16_out[index_filter_pot3];
  tmpS = CV3_value;
  tmpS *= CV_filter16_out[index_filter_pot4];
  tmpS >>= 15;
  gain += tmpS; 
  gain = min(0xFFFF, max(0, gain));
  gain *= gain>>1;
  gain_global = gain >> 7; //  24 bits
  
  // decay
  FB = CV_filter16_out[index_filter_pot5];
  FB = min(max(0, FB), 0xFFFF);
  FB = fast_sin(FB<<14);
  FB >>= 8;
  FB_global =  FB; //  24 bits

  // Filter coef
  freq = CV_filter16_out[index_filter_pot6] * 1720;
  freq += 0x07000000; 
  macro_fq2increment_novar
  filter_LOP_global = min(0x00FFFFFF, increment1);

  if ( (CV4_connect < 60) && (CV_filter16_out[index_filter_cv4] > 0xB000) && (pluck == 0) ) {
    pluck = 1;
    pluck2 = 0x7FFFFFFF;
  } else if (CV_filter16_out[index_filter_cv4] < 0xA000) {
    pluck = 0;
    pluck2 = 0;
  }
}

inline void KS_audio_() {
  int32_t audio_in, tmpS, outS, out2S;
  int32_t audio_out, delay_out, out1, out2, feedback_out;
  uint32_t delay_time, FB, gain, delay_time_LSB, read_point, filter_LOP;
  uint32_t pluck_filter;
  uint32_t toggle, out;

  toggle = toggle_global;
  
  if (IN1_connect < 60) 
    audio_in = audio_inL ^0x80000000;
  else {
    if (CV4_connect < 60) {
      audio_in = 0;
    }
    else {
      audio_in = random32();
    }
  }
  
  index_ecriture = (index_ecriture+1) & Max_Delay;
  
  //delay_time = delay_time_global;
  delay_time = filter(delay_time_global, delay_time_save, 6);
  delay_time_save = delay_time;

  //gain = gain_global;
  gain = filter(gain_global, gain_save, 6);
  gain_save = gain;

  //FB = FB_global;
  FB = filter(FB_global, FB_save, 6);
  FB_save = FB;

  //Filter
  filter_LOP = filter(filter_LOP_global, filter_LOP_save, 6);
  filter_LOP_save = filter_LOP;

  // delay loop
  delay_time_LSB = delay_time & 0xFFF; // on garde les 12 bits de poinds faible pour interpoler
  delay_time >>= 12; // on les suprime pour ne garder que l'index sur 12 bit (4096 point, soit env 10Hz a 48KHz) 
  read_point = (index_ecriture - delay_time) & Max_Delay;
  out1 = delay_line.S16[read_point];
  out2 = delay_line.S16[(read_point-1) & Max_Delay];
  out2 -= out1;
  out2 *= delay_time_LSB;
  out1 += out2 >> 12;

  //input
  audio_in >>= 16;

  //pluck
  pluck_filter = filter(pluck2, pluck2_save, 3);
  pluck2_save = pluck_filter;
  if (pluck_filter > 0x70000000) pluck2 = 0;
  audio_in += pluck_filter>>16;


  switch (toggle) { // effect
    case 0:
      feedback_out = max(0, min(0x7FFF, delay_out - audio_in));
      feedback_out = (-fast_sin(feedback_out<<16))>>16;
      feedback_out *= (gain>>8);
      feedback_out >>= 16;
      delay_out = out1 * (FB>>8);
      delay_out >>= 16;
      feedback_out = max(-0x7FFF, min(0x7FFF, feedback_out - delay_out));
    break;
    case 1:
      feedback_out = max(0, min(0x7FFF, delay_out - audio_in ));
      feedback_out *= (gain>>8);
      feedback_out >>= 16;
      delay_out = out1 * (FB>>8);
      delay_out >>= 16;
      feedback_out = max(-0x7FFF, min(0x7FFF, feedback_out - delay_out));
    break;
    case 2:
      feedback_out =  audio_in;
      feedback_out *= (gain>>8);
      feedback_out >>= 16;
      delay_out = out1 * (FB>>8);
      delay_out >>= 16;
      feedback_out = max(-0x7FFF, min(0x7FFF, feedback_out - delay_out));
    break;
  }
  
  //audio_out = audio_in - delay_out;
  //audio_out = max(-0x7FFF, min(0x7FFF, audio_out));
  audio_out = feedback_out;

  // filtre LOP
  //audio_outL = (audio_out<<16)^0x80000000;
  //tmpS = audio_out;
  tmpS = feedback_out;
  tmpS -= filter_audio_out;
  tmpS *= filter_LOP>>9;
  tmpS >>= 15;
  filter_audio_out += tmpS;

  delay_line.S16[index_ecriture] = filter_audio_out;

  outS = (filter_audio_out<<16);
  out = outS^0x80000000;

/*
 switch (toggle) { // effect
    case 0:
    break;
    case 1:
      out = fast_sin((out>>1)+0x40000000);
    break;
    case 2:
      out = fast_sin(out<<1);
    break;
  }
*/

  //out -= out>>2;
  //out += 0x20000000; // limiter le gain


  // delay loop
  delay_time = delay_time_save >> 1; // on divise le delay par 2 pour la 2eme sortie en oposition de phase
  delay_time_LSB = delay_time & 0xFFF; // on garde les 12 bits de poinds faible pour interpoler
  delay_time >>= 12; // on les suprime pour ne garder que l'index sur 12 bit (4096 point, soit env 10Hz a 48KHz) 
  read_point = (index_ecriture - delay_time) & Max_Delay;
  out1 = delay_line.S16[read_point];
  out2 = delay_line.S16[(read_point-1) & Max_Delay];
  out2 -= out1;
  out2 *= delay_time_LSB;
  out1 += out2 >> 12;

  
  out2S = out1<<16;
  out2 = out2S ^0x80000000;
  
  macro_out_pan_stereo
}

