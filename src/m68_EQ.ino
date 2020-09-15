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

// you do what you want to test anything
// Or use this module as a template to create an other module.
// rename the 3 functions and declare it in the nozori.h module list

// EQ MIXER
// MIX BASS/ MEDIUM / HIGH of a source

// Pot 1 : High level (from -inf to +6dB)
// Pot 2 : High Level modulation
// Pot 3 : Medium level (from -inf to +6dB)
// Pot 4 : Medium Level modulation
// Pot 5 : Low level (from -inf to +6dB)
// Pot 6 : Low Level modulation
// CV 1 : bypass
// CV 2 : Bass modulation CV
// CV 3 : Medium Modulation CV
// CV 4 : High modulation CV
// IN 1 : audio in L
// IN 2 : audio in R
// Selecteur3 :
// OUT 1 : audio out L
// OUT 2 : audio out R


int32_t inL_filter_low1, inL_filter_low2, inL_filter_low3, inL_filter_low4, inL_filter_high1, inL_filter_high2, inL_filter_high3, inL_filter_high4;
int32_t inR_filter_low1, inR_filter_low2, inR_filter_low3, inR_filter_low4, inR_filter_high1, inR_filter_high2, inR_filter_high3, inR_filter_high4;

inline void EQ_init_() {
    init_chaos();
    last_CV1 = 0;

  inL_filter_low1 = 0;
  inL_filter_low2 = 0;
  inL_filter_low3 = 0;
  inL_filter_low4 = 0;
  inL_filter_high1 = 0;
  inL_filter_high2 = 0;
  inL_filter_high3 = 0;
  inL_filter_high4 = 0;
  inR_filter_low1 = 0;
  inR_filter_low2 = 0;
  inR_filter_low3 = 0;
  inR_filter_low4 = 0;
  inR_filter_high1 = 0;
  inR_filter_high2 = 0;
  inR_filter_high3 = 0;
  inR_filter_high4 = 0;
}

inline void EQ_loop_() {
  int32_t CV2_value, CV3_value, CV4_value;
  int32_t tmpS, pot1_tmp, pot3_tmp, pot5_tmp;
  uint32_t tmp;

  filter16_nozori_68  
  test_connect_loop_68();
  chaos(16); // for default mod values
  toggle_global = get_toggle();
  
  if (CV2_connect < 60) CV2_value = CV_filter16_out[index_filter_cv2] - CV2_0V; else CV2_value = (chaos_dx>>16);
  if (CV3_connect < 60) CV3_value = CV_filter16_out[index_filter_cv3] - CV3_0V; else CV3_value = (chaos_dy>>16);
  if (CV4_connect < 60) CV4_value = CV_filter16_out[index_filter_cv4] - CV4_0V; else CV4_value = (chaos_dz>>16);
  CV2_value = min(0x7FFF,max(-0x7FFF,CV2_value));
  CV3_value = min(0x7FFF,max(-0x7FFF,CV3_value));
  CV4_value = min(0x7FFF,max(-0x7FFF,CV4_value));

  led2((CV2_value+0x7FFF)>>7);
  led4((CV3_value+0x7FFF)>>7);
  
  //filter high
  pot1_tmp = CV_filter16_out[index_filter_pot1];
  tmpS = CV2_value;
  tmpS *= CV_filter16_out[index_filter_pot2];
  tmpS >>= 16;
  pot1_tmp += tmpS;
  tmp = min(0xFFFF, max(0,pot1_tmp));
  tmp = tmp * tmp;
  tmp >>= 17;
  high_gain = tmp;
  
  //filter medium
  pot3_tmp = CV_filter16_out[index_filter_pot3];
  tmpS = CV3_value;
  tmpS *= CV_filter16_out[index_filter_pot4];
  tmpS >>= 16;
  pot3_tmp += tmpS;
  tmp = min(0xFFFF, max(0,pot3_tmp));
  tmp = tmp * tmp;
  tmp >>= 17;
  medium_gain = tmp;

  // filter low
  pot5_tmp = CV_filter16_out[index_filter_pot5];
  tmpS = CV4_value;
  tmpS *= CV_filter16_out[index_filter_pot6];
  tmpS >>= 16;
  pot5_tmp += tmpS;
  tmp = min(0xFFFF, max(0,pot5_tmp));
  tmp = tmp * tmp;
  tmp >>= 17;
  low_gain = tmp;
}

inline void EQ_audio_() {
  int32_t tmpS, inL, inR, in;
  uint32_t out, out2;
  int32_t outS, out2S;
  int32_t low, medium, high;
  uint32_t toggle;

  toggle = toggle_global;

  if (!( (CV1_connect < 60) && (CV_filter16_out[index_filter_CV1] > 0xA800) )) {    

    if (IN1_connect < 60) in = audio_inL^0x80000000; else in = 0;
    inL = in >> 16;
  
    if (IN2_connect < 60) inR = audio_inR^0x80000000; else inR = in;
    inR = inR >> 16;
    
    tmpS = inL - inL_filter_low1;
    inL_filter_low1 += tmpS/100;
    tmpS = inL_filter_low1 - inL_filter_low2;
    inL_filter_low2 += tmpS/100;
    tmpS = inL_filter_low2 - inL_filter_low3;
    inL_filter_low3 += tmpS/100;
    //tmpS = inL_filter_low3 - inL_filter_low4;
    //inL_filter_low4 += tmpS/100;
  
    switch(toggle) {
      case 0: 
        low = inL_filter_low3;
        break;
      case 1:
        low = inL_filter_low2;
        break;
      case 2:
        low = inL_filter_low1;
        break;
    }  
    medium = inL - low;
  
    tmpS = medium - inL_filter_high1;
    inL_filter_high1 += tmpS/5;
    tmpS = inL_filter_high1 - inL_filter_high2;
    inL_filter_high2 += tmpS/5;
    tmpS = inL_filter_high2 - inL_filter_high3;
    inL_filter_high3 += tmpS/5;
    //tmpS = inL_filter_high3 - inL_filter_high4;
    //inL_filter_high4 += tmpS/5;
  
    switch(toggle) {
      case 0: 
        tmpS = inL_filter_high3;
        break;
      case 1:
        tmpS = inL_filter_high2;
        break;
      case 2:
        tmpS = inL_filter_high1;
        break;
    }
    high = medium - tmpS;
    medium = tmpS;
  
    tmpS = (low * low_gain);
    tmpS += (medium * medium_gain);
    tmpS += (high * high_gain);
    saturate_S30(tmpS, outS);
    out = (outS<<2)^0x80000000;

  //////////////////////////////////////////////////////
    tmpS = inR - inR_filter_low1;
    inR_filter_low1 += tmpS/100;
    tmpS = inR_filter_low1 - inR_filter_low2;
    inR_filter_low2 += tmpS/100;
    tmpS = inR_filter_low2 - inR_filter_low3;
    inR_filter_low3 += tmpS/100;
    //tmpS = inR_filter_low3 - inR_filter_low4;
    //inR_filter_low4 += tmpS/100;
  
    switch(toggle) {
      case 0: // 12dB
        low = inR_filter_low3;
        break;
      case 1:
        low = inR_filter_low2;
        break;
      case 2:
        low = inR_filter_low1;
        break;
    }  
    medium = inR - low;
  
    tmpS = medium - inR_filter_high1;
    inR_filter_high1 += tmpS/5;
    tmpS = inR_filter_high1 - inR_filter_high2;
    inR_filter_high2 += tmpS/5;
    tmpS = inR_filter_high2 - inR_filter_high3;
    inR_filter_high3 += tmpS/5;
    //tmpS = inR_filter_high3 - inR_filter_high4;
    //inR_filter_high4 += tmpS/5;
  
    switch(toggle) {
      case 0: // 12dB
        tmpS = inR_filter_high3;
        break;
      case 1:
        tmpS = inR_filter_high2;
        break;
      case 2:
        tmpS = inR_filter_high1;
        break;
    }
    high = medium - tmpS;
    medium = tmpS;
  
    tmpS = (low * low_gain);
    tmpS += (medium * medium_gain);
    tmpS += (high * high_gain);
    saturate_S30(tmpS, out2S);
    out2 = (out2S<<2)^0x80000000;

    macro_out_stereo
  } 
  else { // Bypass
    if (IN1_connect < 60) audio_outL = audio_inL; else audio_outL = 0;  
    if (IN2_connect < 60) audio_outR = audio_inR; else audio_outR = 0;
  }
}

