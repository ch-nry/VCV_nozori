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

// noise generator
// Pot 1 : digital noise frequeny
// Pot 2 : noise frequency modulation
// Pot 3 : high pass filter cutoff
// Pot 4 : high pass filter cutoff modulation
// Pot 5 : Low pass filter cutoff
// Pot 6 : Low pass filter cutoff modulation
// CV 1 : Trig
// CV 2 : Noise frequency modulation CV
// CV 3 : high pass frequency modulation
// CV 4 : low pass frequency modulation
// IN 1 : gain
// IN 2 : pan
// Selecteur3 : noise color (white, pink, red)
// OUT 1 : noise Left
// OUT 2 : Noise Right



int32_t noiseL, noiseR, noiseL_filter_low, noiseR_filter_low, noiseL_filter_high, noiseR_filter_high;
int32_t filterL1, filterL2, filterL3, filterL4;
int32_t filterR1, filterR2, filterR3, filterR4;

inline void noise_init_() {
  VCO1_phase = 0;
  init_chaos();
  last_CV1 = 0;

  noiseL = 0;
  noiseR = 0;
  noiseL_filter_low = 0;
  noiseR_filter_low = 0;
  noiseL_filter_high = 0;
  noiseR_filter_high = 0;
  filterL1 = 0;
  filterL2 = 0;
  filterL3 = 0;
  filterL4 = 0;
  filterR1 = 0;
  filterR2 = 0;
  filterR3 = 0;
  filterR4 = 0;
}

inline void noise_loop_() {
  int32_t CV3_value, CV4_value;
  int32_t tmpS, pot3_tmp, pot5_tmp;
  int32_t freq;
  
  filter16_nozori_68  
  test_connect_loop_68();
  chaos(16); // for default mod values
  
  if (CV3_connect < 60) CV3_value = CV_filter16_out[index_filter_cv3] - CV3_0V; else CV3_value = (chaos_dx>>16);
  if (CV4_connect < 60) CV4_value = CV_filter16_out[index_filter_cv4] - CV4_0V; else CV4_value = (chaos_dy>>16);
  CV3_value = min(0x7FFF,max(-0x7FFF,CV3_value));
  CV4_value = min(0x7FFF,max(-0x7FFF,CV4_value));

  led2((CV3_value+0x7FFF)>>7);
  led4((CV4_value+0x7FFF)>>7);

  // fq
  //macro_fq_in
  //freq = (CV_filter16_out[index_filter_pot1])<<11;
  //freq += 0x06000000;
  freq = (CV_filter16_out[index_filter_pot1])<<12;
  macro_FqMod_fine(pot2,CV2)
  macro_fq2increment
  increment_1 = increment1;
  
  //filter
  pot3_tmp = CV_filter16_out[index_filter_pot3];
  tmpS = CV3_value;
  tmpS *= CV_filter16_out[index_filter_pot4];
  tmpS >>= 16;
  pot3_tmp += tmpS;
  pot3_tmp = min(0xFFFF, max(0,pot3_tmp));
  //high_gain = pot3_tmp;

  pot5_tmp = CV_filter16_out[index_filter_pot5];
  tmpS = CV4_value;
  tmpS *= CV_filter16_out[index_filter_pot6];
  tmpS >>= 16;
  pot5_tmp += tmpS;
  pot5_tmp = min(0xFFFF, max(0,pot5_tmp));
  //low_gain = pot5_tmp;

  //tmp = pot3_tmp; // passage en unsigned
  //tmp *= tmp; // TODO : better curve???
  //tmp >>= 17;
  //high_gain = max(10,tmp)-10;

  //freq = _min(0xFA00000, freq); /*20KHz max*/
  freq = pot3_tmp << 12;
  freq_MSB = freq >> 18; /* keep the 1st 10 bits */
  freq_LSB = freq & 0x3FFFF; /* other 18  bits */
  increment1 = table_CV2increment[freq_MSB];
  increment2 = table_CV2increment[freq_MSB+1];
  increment2 -= increment1;
  increment1 += ((increment2>>8)*(freq_LSB>>2))>>8;
  increment1 -= 60;     
  high_gain = increment1 * 5; // 31 bit max

  //tmp= pot5_tmp; // passage en unsigned
  //tmp *= tmp;
  //tmp >>= 17;
  //low_gain = max(10,tmp)-10;
  
  freq = pot5_tmp << 12;
  freq_MSB = freq >> 18; /* keep the 1st 10 bits */
  freq_LSB = freq & 0x3FFFF; /* other 18  bits */
  increment1 = table_CV2increment[freq_MSB];
  increment2 = table_CV2increment[freq_MSB+1];
  increment2 -= increment1;
  increment1 += ((increment2>>8)*(freq_LSB>>2))>>8;
  increment1 -= 60;     
  low_gain = increment1 * 12; // 31 bit max
}

inline void noise_audio_() {
  int32_t tmpS;
  int32_t out, out2;

  if (CV1_connect < 60) {
    if ( (last_CV1 == 0) && (CV_filter16_out[index_filter_CV1] > 0xB000) )
    {
       last_CV1 = 1;
       switch(get_toggle()) { // noise type
          case 0: // white
            tmpS = random32();
            noiseL = tmpS/3;// 30 bits + sign
            tmpS = random32();
            noiseR = tmpS/3;// 30 bits + sign
          break;
          case 1: // pink
            tmpS = random32();
            tmpS >>= 1; // limation de l'amplitude pour etre sur de ne pas saturer
            filterL1 += (tmpS/128) - (filterL1>>9);
            filterL2 += (tmpS/40)  - (filterL2>>6);
            filterL3 += (tmpS/12)  - (filterL3>>3);
            filterL4 =  (tmpS/10);
            tmpS = filterL1 + filterL2 + filterL3 + filterL4;          
            saturate_S31(tmpS, tmpS);
            noiseL =  tmpS;           
  
            tmpS = random32();
            tmpS >>= 1; // limation de l'amplitude pour etre sur de ne pas saturer
            filterR1 += (tmpS/128) - (filterR1>>9);
            filterR2 += (tmpS/40)  - (filterR2>>6);
            filterR3 += (tmpS/12)  - (filterR3>>3);
            filterR4 =  (tmpS/10);
            tmpS = filterR1 + filterR2 + filterR3 + filterR4;          
            saturate_S31(tmpS, tmpS);
            noiseR =  tmpS;                   
          break;
          case 2: // red
            tmpS = random32();
            filterL1 += (tmpS>>5) - (filterL1>>9);
            saturate_S31(filterL1, filterL1);
            noiseL =  filterL1;
            
            tmpS = random32();
            //tmpS >>= 1; // limation de l'amplitude pour etre sur de ne pas saturer
            filterR1 += (tmpS>>5) - (filterR1>>9);
            saturate_S31(filterR1, filterR1);
            noiseR =  filterR1;
          break;
        }
    }
    else if ( (last_CV1 == 1) && (CV_filter16_out[index_filter_CV1] < 0xA000)) {
       last_CV1 = 0;
    }
  }
  else {
    VCO1_phase += increment_1<<4;  
    if (VCO1_phase & 0xC0000000) {
      VCO1_phase &= 0x3FFFFFFF;
      
      switch(get_toggle()) { // noise type
        case 0: // white
          tmpS = random32();
          noiseL = tmpS/3;// 30bits + sign
          tmpS = random32();
          noiseR = tmpS/3;// 30bits + sign
        break;
        case 1: // pink
          tmpS = random32();
          tmpS >>= 1; // limation de l'amplitude pour etre sur de ne pas saturer
          filterL1 += (tmpS/128) - (filterL1>>9);
          filterL2 += (tmpS/40)  - (filterL2>>6);
          filterL3 += (tmpS/12)  - (filterL3>>3);
          filterL4 =  (tmpS/10);
          tmpS = filterL1 + filterL2 + filterL3 + filterL4;          
          saturate_S31(tmpS, tmpS);
          noiseL =  tmpS;           

          tmpS = random32();
          tmpS >>= 1; // limation de l'amplitude pour etre sur de ne pas saturer
          filterR1 += (tmpS/128) - (filterR1>>9);
          filterR2 += (tmpS/40)  - (filterR2>>6);
          filterR3 += (tmpS/12)  - (filterR3>>3);
          filterR4 =  (tmpS/10);
          tmpS = filterR1 + filterR2 + filterR3 + filterR4;          
          saturate_S31(tmpS, tmpS);
          noiseR =  tmpS;                   
        break;
        case 2: // red
          tmpS = random32();
          filterL1 += (tmpS>>5) - (filterL1>>9);
          saturate_S31(filterL1, filterL1);
          noiseL =  filterL1;
          
          tmpS = random32();
          //tmpS >>= 1; // limation de l'amplitude pour etre sur de ne pas saturer
          filterR1 += (tmpS>>5) - (filterR1>>9);
          saturate_S31(filterR1, filterR1);
          noiseR =  filterR1;
        break;
      }
    }
  }

  //left
  tmpS = noiseL - noiseL_filter_high; // 31 bits + sign
   _m_s32xs32_s32(tmpS, high_gain, tmpS, tmpS2);
  noiseL_filter_high += tmpS;
  tmpS = noiseL - noiseL_filter_high; // hip
  
  tmpS -= noiseL_filter_low; //31 bit + sign
  _m_s32xs32_s32(tmpS, low_gain, tmpS, tmpS2);
  noiseL_filter_low += tmpS<<1;

  tmpS = noiseL_filter_low>>2;
  tmpS *= 3;
  saturate_S31(tmpS, tmpS);
  tmpS <<= 1;
  out = tmpS ^0x80000000;
  
  //right
  tmpS = noiseR - noiseR_filter_high; // 31 bits + sign
   _m_s32xs32_s32(tmpS, high_gain, tmpS, tmpS2);
  noiseR_filter_high += tmpS;
  tmpS = noiseR - noiseR_filter_high; // hip
  
  tmpS -= noiseR_filter_low; //31 bit + sign
  _m_s32xs32_s32(tmpS, low_gain, tmpS, tmpS2);
  noiseR_filter_low += tmpS<<1;

  tmpS = noiseR_filter_low>>2;
  tmpS *= 3;
  saturate_S31(tmpS, tmpS);
  tmpS <<= 1;
  out2 = tmpS ^0x80000000;
  
  macro_out_gain_pan_stereo  
}

