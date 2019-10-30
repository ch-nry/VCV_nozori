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
// Pot 1 : FQ
// Pot 2 : Mod Fq (or fine if nothing is connected on the modulation)
// Pot 3 : gain modulation 1
// Pot 4 : mod gain 1
// Pot 5 : gain modulation 2
// Pot 6 : mod gain 2
// CV 1 : Pitch (1V/Oct)
// CV 2 : Mod Fq (1V/Oct a full)
// CV 3 : gain 1 modulation value
// CV 4 : gain 2 modulation value
// IN 1 : IN1
// IN 2 : IN2
// Selecteur3 : MOD FM / PM / AM
// OUT 1 : OUT
// OUT 2 : OUT


uint32_t mod1_global, mod2_global;

inline void VCO_SIN_MOD_init_() {
  VCO1_phase = 0;
  init_chaos();
}

inline void VCO_SIN_MOD_loop_() {
  filter16_nozori_68
  test_connect_loop_68();
  int32_t tmpS;
  int32_t CV3_value, CV4_value;
  
  chaos(16); // for default mod values
  
  if (CV3_connect < 60) CV3_value = CV_filter16_out[index_filter_cv3] - CV3_0V; else CV3_value = (chaos_dx>>16);
  if (CV4_connect < 60) CV4_value = CV_filter16_out[index_filter_cv4] - CV4_0V; else CV4_value = (chaos_dy>>16);
  CV3_value = min(0x7FFF,max(-0x7FFF,CV3_value));
  CV4_value = min(0x7FFF,max(-0x7FFF,CV4_value));
  mod1_global = CV3_value;
  mod2_global = CV4_value;

  led2((CV3_value+0x7FFF)>>7);
  led4((CV4_value+0x7FFF)>>7);

  macro_fq_in
  macro_1VOct_CV1
  macro_FqMod_fine(pot2,CV2)
  freq_global = freq;
}

inline void VCO_SIN_MOD_audio_() {
  uint32_t tmp, tmp2;
  int32_t freq, tmpS;
  uint32_t out, out2, tlg, PM_phase;
  int32_t mod1, mod2;
  uint32_t clip_audio;
  
  tlg = get_toggle();

  freq = freq_global;
  mod1 = mod1_global;
  mod2 = mod2_global;

  mod1 *= CV_filter16_out[index_filter_pot4];
  mod1 >>=16; // 16 bits
  mod1 += CV_filter16_out[index_filter_pot3];
  mod1 = max(0, mod1);
  mod1 = filter(mod1, mod1_save, 6);
  mod1_save = mod1;
  if (IN1_connect < 60) tmpS = audio_inL^0x80000000; else tmpS = 0;
  tmpS = tmpS>>17;
  mod1 *= tmpS;
    
  mod2 *= CV_filter16_out[index_filter_pot6];
  mod2 >>=16; 
  mod2 += CV_filter16_out[index_filter_pot5];
  mod2 = max(0, mod2);
  mod2 = filter(mod2, mod2_save, 6);
  mod2_save = mod2;
  if (IN2_connect < 60) tmpS = audio_inR^0x80000000; else tmpS = 0;
  tmpS = tmpS>>17;
  mod2 *= tmpS;

  if(tlg == 0) { // FM
    freq += mod1 >> 5;
    freq += mod2 >> 5;
  }
  macro_fq2increment

  VCO1_phase += increment1<<3;  

  PM_phase = 0;
  if(tlg == 1) { // PM
    PM_phase += mod1<<2;
    PM_phase += mod2<<2;
  }
  
  out = fast_sin(VCO1_phase + PM_phase);
  out2 = fast_sin((VCO1_phase<<1) + PM_phase);
  
  if(tlg == 2) { // AM
    tmpS  = mod1_save;
    tmpS = max(0, tmpS);
    tmpS = min( 0xFFFF, tmpS);
    tmp = tmpS;
    clip_audio = max(audio_inL, 0x20000000); // for +/- 5V amplitude modulation
    clip_audio -= 0x20000000;
    clip_audio = min(audio_inL, 0xC0000000);
    clip_audio += clip_audio >>2;    
    if (IN1_connect < 60) tmp2 = tmp * (clip_audio>>16); else tmp2 = 0;
    
    tmpS  = mod2_save;
    tmpS = max(0, tmpS);
    tmpS = min( 0xFFFF, tmpS);
    tmp = tmpS;
    clip_audio = max(audio_inR, 0x20000000);
    clip_audio -= 0x20000000;
    clip_audio = min(audio_inR, 0xC0000000);
    clip_audio += clip_audio >>2;    
    if (IN2_connect < 60) tmp *= clip_audio>>16; else tmp = 0;

    tmp = (tmp >> 1) + (tmp2 >> 1);
    tmp = min(0x7FFFFFFF, tmp);
    tmp <<= 1;
    tmpS = 0xFFFFFFFF - tmp;
    tmpS ^= 0x80000000;
    out ^= 0x80000000;
    _m_s32xs32_s32(out, tmpS, out, tmp);
    out *= 2;
    out -= (int32_t)out>>2;
    out ^= 0x80000000;

    out2 ^= 0x80000000;
    _m_s32xs32_s32(out2, tmpS, out2, tmp);
    out2 *= 2;
    out2 -= (int32_t)out2 >> 2;
    out2 ^= 0x80000000;
  }
  else {
    tmpS = out^0x80000000;
    tmpS -= tmpS>>2;
    out = tmpS^0x80000000;
    tmpS = out2^0x80000000;
    tmpS -= tmpS>>2;
    out2 = tmpS^0x80000000;
  }
  macro_out_stereo
}

