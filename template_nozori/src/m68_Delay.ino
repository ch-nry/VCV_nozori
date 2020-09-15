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

// sinus oscillator with a waveshapper
// Pot 1 : Delay time
// Pot 2 : Mod time
// Pot 3 : audio IN gain
// Pot 4 : audio in gain modulation
// Pot 5 : FeedBack
// Pot 6 : Mod FB
// CV 1 : syncro
// CV 2 : Delay time
// CV 3 : In
// CV 4 : FB
// IN 1 : In
// IN 2 : Pan
// Selecteur3 : overflow type : clip / fold / wave shape
// OUT 1 : OUT L
// OUT 2 : OUT R

inline void Delay_init_() {
  uint32_t i;
  // Switch to 48KHz
  send_dac(0x08,0b000000001); // sampling control (usb , 250fs,  48K, clock div 2, clk out, active)
  init_chaos();
  for (i=0; i<Max_Delay;i++) {
    delay_line.S16[i] = 0;
  }
}

inline void Delay_loop_() {
  uint32_t tmp, delay_timeU, freq;
  int32_t tmpS, CV2_value, CV3_value, CV4_value, toggle, gain, delay_time, FB;

  filter16_nozori_68
  test_connect_loop_68();
  chaos(15); // for default mod values
  toggle = get_toggle();
  toggle_global = toggle;

  if (CV2_connect < 60) CV2_value = CV_filter16_out[index_filter_cv2] - CV2_0V; else CV2_value = chaos_dx>>16;
  if (CV3_connect < 60) CV3_value = CV_filter16_out[index_filter_cv3] - CV3_0V; else CV3_value = chaos_dy>>16;
  if (CV4_connect < 60) CV4_value = CV_filter16_out[index_filter_cv4] - CV4_0V; else CV4_value = chaos_dz>>16;
  CV2_value = min(0x7FFF,max(-0x7FFF,CV2_value));
  CV3_value = min(0x7FFF,max(-0x7FFF,CV3_value));
  CV4_value = min(0x7FFF,max(-0x7FFF,CV4_value));

  // delay time
  if (CV1_connect  < 60) { // connect mode
    freq = (CV_filter16_out[index_filter_pot1] + 4095) / 8192;
    tmp = (CV_filter16_out[index_filter_pot2] + 5461)/10923; // from 0.5 to 6.5 //  i.e: 0..6 // i.e :7 values
    clock_diviseur = tab_diviseur[freq]  * tab_diviseur2[tmp];;
    clock_multiplieur = tab_multiplieur[freq] * tab_multiplieur2[tmp];;   
  }
  else
  {
    delay_time = CV_filter16_out[index_filter_pot1];
    tmpS = CV2_value>>1;
    tmpS *= CV_filter16_out[index_filter_pot2];
    tmpS >>= 17;
    delay_time += tmpS;
    delay_time = min(max(0, delay_time), 0xFFFF);
    delay_timeU = delay_time;
    delay_timeU *= delay_timeU;
    delay_time_global =  delay_timeU >> 8; // on reste sur 24 bits
  }

  // gain
  gain = CV_filter16_out[index_filter_pot3];
  tmpS = CV3_value>>1;
  tmpS *= CV_filter16_out[index_filter_pot4];
  tmpS >>= 15;
  gain += tmpS;
  gain = min(max(0, gain), 0xFFFF);
  gain_global =  gain << 8; //  24 bits

  // feedback
  FB = CV_filter16_out[index_filter_pot5];
  tmpS = CV4_value>>1;
  tmpS *= CV_filter16_out[index_filter_pot6];
  tmpS >>= 15;
  FB += tmpS;
  FB = min(max(0, FB), 0xFFFF);
  if (toggle == 2) { FB >> 2; }
  FB_global =  FB << 8; //  24 bits

  led2((CV2_value+0x7FFF)>>7);
  led4((CV3_value+0x7FFF)>>7);
}

inline void Delay_audio_() {
  int32_t audio_in, out;
  int32_t audio_out, delay_out, out1, out2;
  uint32_t delay_time, FB, gain, delay_time_LSB, read_point, toggle;

  toggle = toggle_global;

  // syncro
  nb_tick = min(0x0007FFFF, nb_tick+1); // to prvent overflow with multiplier
  if( (last_clock_ == 0) && (CV1_connect < 60) && (CV_filter16_out[index_filter_cv1] > 0xB000) ) { // mode syncro, on a une syncro
    last_clock_ = 1;
    nb_tick /= clock_multiplieur; // reversed because we use the period, not the frequency
    nb_tick *= clock_diviseur;
    delay_time_global = min(Max_Delay, nb_tick)<<9;
    nb_tick = 0;
  }
  else if ( (CV1_connect < 60) && (CV_filter16_out[index_filter_cv1] < 0xA000) ) {
    last_clock_ = 0;
  }
  
  /////////////////////
  if (IN1_connect < 60)
    audio_in = audio_inL ^0x80000000;
  else 
    audio_in = 0;
  audio_in >>= 9;
  filter_audio_in = ((filter_audio_in * 255) + audio_in) >> 8;
  audio_in -= filter_audio_in; // hip on audio in
  
  index_ecriture = (index_ecriture+1) & Max_Delay;
  
  delay_time = delay_time_global;
  delay_time = filter(delay_time, delay_time_save, 8);
  delay_time_save = delay_time;

  gain = filter(gain_global, gain_save, 8);
  gain_save = gain;

  FB = filter(FB_global, FB_save, 8);
  FB_save = FB;

  FB = fast_sin(FB<<6); // sinusoidal curve 
  FB = max(0x7FFFFFFF, FB) - 0x7FFFFFFF;
  FB <<= 1;
  FB = min(0xFF000000, FB);
  FB += FB>>8;
  if(toggle == 2) FB >>= 1;
  
  delay_time_LSB = delay_time & 0x1FF; // on garde les 9 bits de poinds faible pour interpoler
  delay_time >>= 9; // on les suprime pour ne garder que l'index sur 15 bit (taille du buffer)
  read_point = (index_ecriture - delay_time) & Max_Delay;
  out1 = delay_line.S16[read_point];
  out2 = delay_line.S16[(read_point-1) & Max_Delay];
  out2 -= out1;
  out2 *= delay_time_LSB;
  out1 += out2 >> 9;

  delay_out = out1 * (FB>>16);
  delay_out >>= 16;

  audio_in >>= 8;
  audio_in *= (gain>>8);
  audio_in >>= 15;
  
  audio_out = audio_in-delay_out;

  switch(toggle) { //clip type
    case 0 :
      audio_out = max(-0x7FFF, min(0x7FFF, audio_out));
    break;
    case 1 : 
      if(audio_out > 0x7FFF)  audio_out = 0xFFFE - audio_out;
      if(audio_out < -0x7FFF) audio_out = -0xFFFE - audio_out;
    break;
    case 2 :
      audio_out = ((fast_sin(audio_out<<15)^0x80000000)>>16);
    break;
  }
  out = (audio_out<<16)^0x80000000;
  macro_out_pan

  audio_out = audio_in-delay_out;
  switch(toggle) { //clip type
    case 0 :
      audio_out = max(-0x7FFF, min(0x7FFF, audio_out));
    break;
    case 1 : 
      if(audio_out > 0x7FFF)  audio_out = 0xFFFE - audio_out;
      if(audio_out < -0x7FFF) audio_out = -0xFFFE - audio_out;
    break;
    case 2 :
      audio_out = ((fast_sin(audio_out<<16)^0x80000000)>>16);
    break;
  }
  delay_line.S16[index_ecriture] = audio_out;
}

