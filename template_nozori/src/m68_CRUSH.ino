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

// Bit crusher
// Pot 1 : dry / wet
// Pot 2 : wet/dry modulation
// Pot 3 : Downsample
// Pot 4 : DS MOD
// Pot 5 : Bitcrush amplitude
// Pot 6 : Mod
// CV 1 : Bypass
// CV 2 : Mod Wet / Dry
// CV 3 : Mod Downsample
// CV 4 : Mod Bitcrush
// IN 1 : in 1
// IN 2 : in 2
// Selecteur3 : mode : dual / oposit / mono + pan
// OUT 1 : OUT L
// OUT 2 : OUT R

int32_t bitcrush, bitcrush2;
uint32_t save_out1, save_out2;

inline void BIT_Crush_init_() {
  sin1_phase = 0;
  sin2_phase = 0;
  init_chaos();
}

inline void BIT_Crush_loop_() {
  int32_t freq, freq_save, tmpS;
  int32_t bt, bt_save;
  int32_t CV2_value, CV3_value, CV4_value;
  uint32_t toggle;
  int32_t dry1, dry2, dry1_;

  filter16_nozori_68
  test_connect_loop_68();

  toggle = get_toggle();
  toggle_global = toggle;
  chaos(11); // for default mod values
    
  if (CV2_connect < 60) CV2_value = CV_filter16_out[index_filter_cv2] - CV2_0V; else CV2_value = chaos_dx>>16;
  if (CV3_connect < 60) CV3_value = CV_filter16_out[index_filter_cv3] - CV3_0V; else CV3_value = chaos_dy>>16;
  if (CV4_connect < 60) CV4_value = CV_filter16_out[index_filter_cv4] - CV4_0V; else CV4_value = chaos_dz>>16;
  CV2_value = min(0x7FFF,max(-0x7FFF,CV2_value));
  CV3_value = min(0x7FFF,max(-0x7FFF,CV3_value));
  CV4_value = min(0x7FFF,max(-0x7FFF,CV4_value));

  // dry / wet
  dry1_ = (0xFFFF - CV_filter16_out[index_filter_pot1]);
  dry1_ = max(0x00FF, dry1_);
  dry1_ -= 0x00FF;
  
  tmpS = -CV2_value;
  tmpS *= CV_filter16_out[index_filter_pot2];
  tmpS >>= 15;
  dry1 = dry1_ + tmpS;
  dry1_goal = min(max(0, dry1), 0xFFFF)<<8;
  if (toggle == 1) {
    dry2 = dry1_ - tmpS;
    dry2_goal = min(max(0, dry2), 0xFFFF)<<8;
  } else dry2_goal = dry1_goal;

  // frequency control
  freq = (CV_filter16_out[index_filter_pot3]);
  tmpS = CV3_value;
  tmpS *= CV_filter16_out[index_filter_pot4];
  tmpS >>= 16;
  freq_save = freq;
  freq += tmpS;
  freq = 0xFFFF-freq;
  freq <<= 11;
  freq += 0x08000000;
  macro_fq2increment
  increment_1 = increment1*3;

  if (toggle == 1) {
    freq = freq_save - tmpS;
    freq = 0xFFFF-freq;
    freq <<= 11;
    freq += 0x08000000;
    macro_fq2increment_novar
    increment_2 = increment1*3;
  } 
  else {
    increment_2 = increment_1;
  }

  bt = CV4_value >> 1;
  bt *= CV_filter16_out[index_filter_pot6];
  bt >>= 16;
  bt_save = bt;
  bt += CV_filter16_out[index_filter_pot5]>>1;
  bt = min(max(0,bt),0x7FFF);
  //bt *= bt;
  //bt = max(1,bt);
  bitcrush = bt<<8;
  
  if (toggle == 1) {
    bt = -bt_save;
    bt += CV_filter16_out[index_filter_pot5]>>1;
    bt = min(max(0,bt),0x7FFF);
    //bt *= bt;
    //bt = max(1,bt);
  } 
  bitcrush2 = bt<<8;

  led2((CV3_value+0x7FFF)>>7);
  led4((CV4_value+0x7FFF)>>7);
}


inline void BIT_Crush_audio_() {
  int32_t tmpS;
  uint32_t audio_in;
  uint32_t out;
  int32_t dry1_local, dry2_local;
  int32_t bt1_local, bt2_local;

  dry1_local = filter(dry1_goal, dry1_save, 7);
  dry1_save = dry1_local;
  dry2_local = filter(dry2_goal, dry2_save, 7);
  dry2_save = dry2_local;
  
  bt1_local = filter(bitcrush, bt1_save, 7);
  bt1_save = bt1_local;
  bt2_local = filter(bitcrush2, bt2_save, 7);
  bt2_save = bt2_local;
  
  if (IN1_connect < 60) audio_in = audio_inL; else audio_in = 0x80000000;
  sin1_phase += increment_1;
  if (!((CV1_connect < 60) && (CV_filter16_out[index_filter_CV1] > 0xA000))) { // bypass
    if (sin1_phase & 0xC0000000) {
      sin1_phase &= 0x3FFFFFFF;
      tmpS = audio_in^0x80000000;
      tmpS &= ~(bt1_local<<8);
      tmpS += bt1_local<<7; // pseudo hip
      //tmpS /= bitcrush;
      //tmpS *= bitcrush;
      save_out1 = tmpS ^0x80000000;
    } 
    audio_outL = ((dry1_local>>8) * (audio_in>>16)) + ((0xFFFF-(dry1_local>>8)) * (save_out1>>16));
  } 
  else { 
    audio_outL = audio_in; 
  }
  
  if (toggle_global != 2) { 
    if (IN2_connect < 60) audio_in = audio_inR;  // use audio L if right is not connected
    sin2_phase += increment_2;
    if (!((CV1_connect < 60) && (CV_filter16_out[index_filter_CV1] > 0xA800))) { // bypass
      if (sin2_phase & 0xC0000000) {
        sin2_phase &= 0x3FFFFFFF;
        tmpS = audio_in^0x80000000;
        tmpS &= ~(bt2_local<<8);
        tmpS += bt2_local<<7; // pseudo hip
        //tmpS /= bitcrush2;
        //tmpS *= bitcrush2;
        save_out2 = tmpS ^0x80000000;
      }
      audio_outR = ((dry2_local>>8) * (audio_in>>16)) + ((0xFFFF-(dry2_local>>8)) * (save_out2>>16));
    } 
    else { 
      audio_outR = audio_in; 
    }
  } 
  else { // pan mode
    out = audio_outL;
    macro_out_pan
  }
}

