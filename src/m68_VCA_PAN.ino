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


// dual panoramique + gain 
// Pot 1 : Gain 1 (exp)
// Pot 2 : gain 2
// Pot 3 : Pan 1
// Pot 4 : pan 2
// Pot 5 : Pan 1 Mod 
// Pot 6 : Pan 2 Mod
// CV 1 : Gain 1 modulation value
// CV 2 : Pan 1 modulation value
// CV 3 : Gain 2 modulation value
// CV 4 : Pan 2 modulation value
// IN 1 : audio 1
// IN 2 : audio 2
// Selecteur3 : gain curve : (linear, exponential (from 0 to 5V), exponential from -5 to +5V)
// OUT 1 : out L
// OUT 2 : out R

volatile int32_t Gain1_goal, Gain2_goal, Pan1_goal, Pan2_goal, CVgain1_goal, CVgain2_goal;
volatile int32_t Gain1_save, Gain2_save, Pan1_save, Pan2_save, CVgain1_save, CVgain2_save;
 
inline void Pan_init_() {
  init_chaos();
  Gain1_save = 0;
  Gain2_save = 0;
  Pan1_save = 0;
  Pan2_save = 0;
  CVgain1_save = 0;
  CVgain2_save = 0;
}

inline void Pan_loop_() {
  uint32_t Gain1_local, Gain2_local;
  int32_t Pan1_local, Pan2_local, CVgain1_local, CVgain2_local;
  int32_t tmpS, CV1_value, CV2_value, CV3_value, CV4_value;
  uint32_t toggle;

  filter16_nozori_68
  test_connect_loop_68();
  toggle = get_toggle();
  chaos(15); // for default mod values
  
  if (CV1_connect < 60) CV1_value = CV_filter16_out[index_filter_cv1] - CV1_0V; else CV1_value = 0;
  if (CV2_connect < 60) CV2_value = CV_filter16_out[index_filter_cv2] - CV2_0V; else CV2_value = chaos_dx>>16;
  if (CV3_connect < 60) CV3_value = CV_filter16_out[index_filter_cv3] - CV3_0V; else CV3_value = 0;
  if (CV4_connect < 60) CV4_value = CV_filter16_out[index_filter_cv4] - CV4_0V; else CV4_value = chaos_dy>>16;
  CV1_value = min(0x7FFF,max(-0x7FFF,CV1_value));
  CV2_value = min(0x7FFF,max(-0x7FFF,CV2_value));
  CV3_value = min(0x7FFF,max(-0x7FFF,CV3_value));
  CV4_value = min(0x7FFF,max(-0x7FFF,CV4_value));
  
  Gain1_local = CV_filter16_out[index_filter_pot1];
  Gain1_goal = Gain1_local<<8;
  Gain2_local = CV_filter16_out[index_filter_pot2];
  Gain2_goal = Gain2_local<<8;

  CVgain1_local = min(max(-0x7FFF, CV1_value), 0x7FFF);
  CVgain1_local = CVgain1_local<<8;
  CVgain2_local = min(max(-0x7FFF, CV3_value), 0x7FFF);
  CVgain2_local = CVgain2_local<<8;

  // CV Gain
  switch (toggle) {
    case 0 : // linear, positive and negative
      CVgain1_local <<= 6; // passage sur 30 bit
      CVgain2_local <<= 6;
    break;
    case 1 : // exp from 0 to +5
      CVgain1_local = max(0, CVgain1_local);
      //CVgain1_local = min(0x007FFFFF, CVgain1_local);
      CVgain1_local <<= 7;
      _m_u32xu32_u32(CVgain1_local, CVgain1_local, CVgain1_local, tmp);
      CVgain1_local <<= 1;
      
       CVgain2_local = max(0, CVgain2_local);
      //CVgain2_local = min(0x007FFFFF, CVgain2_local);
      CVgain2_local <<= 7;
      _m_u32xu32_u32(CVgain2_local, CVgain2_local, CVgain2_local, tmp);
      CVgain2_local <<= 1;
    break;
    case 2 : // exp from -5 to +5
      CVgain1_local = CVgain1_local + 0x007FFF00;
      CVgain1_local = max(0, CVgain1_local);  
      CVgain1_local = min(0x00FFFFFF, CVgain1_local);
      CVgain1_local <<= 6;
      _m_u32xu32_u32(CVgain1_local, CVgain1_local, CVgain1_local, tmp);
      CVgain1_local <<= 1;
      
      CVgain2_local = CVgain2_local + 0x007FFF00;
      CVgain2_local = max(0, CVgain2_local);  
      CVgain2_local = min(0x00FFFFFF, CVgain2_local);
      CVgain2_local <<= 6;
      _m_u32xu32_u32(CVgain2_local, CVgain2_local, CVgain2_local, tmp);
      CVgain2_local <<= 1;
    break;
  }
  CVgain1_goal = CVgain1_local;
  CVgain2_goal = CVgain2_local;
  
  Pan1_local = CV_filter16_out[index_filter_pot3]-0x7FFF;
  tmpS = CV2_value>>1;
  tmpS *= CV_filter16_out[index_filter_pot5];
  tmpS >>= 15;
  Pan1_local += tmpS;
  Pan1_local = min(max(-0x7FFF, Pan1_local), 0x7FFF);
  Pan1_goal = Pan1_local<<8;

  Pan2_local = CV_filter16_out[index_filter_pot4]-0x7FFF;
  tmpS = CV4_value>>1;
  tmpS *= CV_filter16_out[index_filter_pot6];
  tmpS >>= 15;
  Pan2_local += tmpS;
  Pan2_local = min(max(-0x7FFF, Pan2_local), 0x7FFF);
  Pan2_goal = Pan2_local<<8;

  led2((CV2_value+0x7FFF)>>7);
  led4((CV4_value+0x7FFF)>>7);
}

inline void Pan_audio_() {
  int32_t inL, inR, outL, outR, tmpS;
  uint32_t Gain1_local, Gain2_local;
  int32_t Pan1_local, Pan2_local, CVgain1_local, CVgain2_local;
  uint32_t panL, panR;
  
  if (IN1_connect < 60) inL = (audio_inL^0x80000000); else inL = 0;
  if (IN2_connect < 60) inR = (audio_inR^0x80000000); else inR = 0;

  //audio filter on various parametters
  Gain1_local = filter(Gain1_goal, Gain1_save, 7);
  Gain1_save = Gain1_local;
  Gain2_local = filter(Gain2_goal, Gain2_save, 7);
  Gain2_save = Gain2_local;
  Pan1_local = filter(Pan1_goal, Pan1_save, 7);
  Pan1_save = Pan1_local;
  Pan2_local = filter(Pan2_goal, Pan2_save, 7);
  Pan2_save = Pan2_local;
  CVgain1_local = filter(CVgain1_goal, CVgain1_save, 7);
  CVgain1_save = CVgain1_local;
  CVgain2_local = filter(CVgain2_goal, CVgain2_save, 7);
  CVgain2_save = CVgain2_local;

  //1st gain
  Gain1_local <<= 8;
  _m_u32xu32_u32(Gain1_local, Gain1_local, Gain1_local, tmp);
  Gain1_local >>= 1;
  _m_s32xs32_s32(Gain1_local, inL, inL, tmp);
  inL <<= 1;
  Gain2_local <<= 8;
  _m_u32xu32_u32(Gain2_local, Gain2_local, Gain2_local, tmp);
  Gain2_local >>= 1;
  _m_s32xs32_s32(Gain2_local, inR, inR, tmp);
  inR <<= 1;

  if (CV1_connect < 60) {
    _m_s32xs32_s32(CVgain1_local, inL, inL, tmp);
    inL <<= 3;
  }
  if (CV3_connect < 60) {
    _m_s32xs32_s32(CVgain2_local, inR, inR, tmp);
    inR <<= 3;
  }
  
  // Pan 1
  panL = Pan1_local+0x00800000;
  panL = fast_sin((panL<<6) + 0x40000000);
  panL = max(0x7FFFFFFF, panL);
  panL -= 0x7FFFFFFF;
  
  panR = Pan1_local+0x00800000;
  panR = fast_sin(panR<<6);
  panR = max(0x7FFFFFFF, panR);
  panR -= 0x7FFFFFFF;
  
  _m_s32xs32_s32(panL, inL, outL, tmp);
  _m_s32xs32_s32(panR, inL, outR, tmp);

  // Pan 2
  panL = Pan2_local+0x00800000;
  panL = fast_sin((panL<<6) + 0x40000000);
  panL = max(0x7FFFFFFF, panL);
  panL -= 0x7FFFFFFF;
  
  panR = Pan2_local+0x00800000;
  panR = fast_sin(panR<<6);
  panR = max(0x7FFFFFFF, panR);
  panR -= 0x7FFFFFFF;
  
  _m_s32xs32_s32(panL, inR, tmpS, tmp);
  outL += tmpS;
  _m_s32xs32_s32(panR, inR, tmpS, tmp);
  outR += tmpS;

  //outL <<= 1;
  //outR <<= 1;
  // Fixed gain with clip
  outL=min(max(-0x0FFFFFFF, outL), 0x0FFFFFFF);
  outR=min(max(-0x0FFFFFFF, outR), 0x0FFFFFFF);
  outL <<= 3;
  outR <<= 3;
  
  audio_outL = outL^0x80000000;
  audio_outR = outR^0x80000000;
}

