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

// sinus oscillator with a diferents harmonics (additiv synthesys)
// Pot 1 : FQ
// Pot 2 : Mod Fq (or fine if nothing is connected on the modulation)
// Pot 3 : FQ H1
// Pot 4 : GAIN H1
// Pot 5 : FQ H2
// Pot 6 : GAIN H2
// Pot 7 : FQ H3
// Pot 8 : GAIN H3
// IN 1 : 1V/Oct
// IN 2 : MOD FQ
// Selecteur3 : waveform (sin / saw / square)
// OUT 1 : OUT
// OUT 2 : OUT (without fundamental) 

//uint32_t VCO_H1_phase, VCO_H2_phase, VCO_H3_phase;

inline void VCO_Tropical_init_() {
  VCO1_phase = 0;
  VCO_H1_phase = 0;
  VCO_H2_phase = 0;
  VCO_H3_phase = 0;
}

inline void VCO_Tropical_loop_() {
  int32_t tmpS;

  filter16_nozori_84
  test_connect_loop_84();

  // Fq principale
  macro_fq_in
  macro_1VOct_IN1
  macro_FqMod_fine_IN2(pot2);
  macro_fq2increment
  increment_0 = increment1;

  // Fq harmo 1
  //macro_fq_in_novar
  freq = (CV_filter16_out[index_filter_pot3])*3800;
  macro_1VOct_IN1
  macro_FqMod_fine_IN2(pot2);
  macro_fq2increment_novar
  increment_1 = increment1;

  // Fq harmo 2
  //macro_fq_in_novar
  freq = (CV_filter16_out[index_filter_pot5])*3800;
  macro_1VOct_IN1
  macro_FqMod_fine_IN2(pot2);
  macro_fq2increment_novar
  increment_2 = increment1;

  // Fq harmo 3
  //macro_fq_in_novar
  freq = (CV_filter16_out[index_filter_pot7])*3800;
  macro_1VOct_IN1
  macro_FqMod_fine_IN2(pot2);
  macro_fq2increment_novar
  increment_3 = increment1;

  if (IN1_connect < 60) led2(audio_inL >> 23); else set_led2(0);
  if (IN2_connect < 60) led4(audio_inR >> 23); else set_led4(0);
//  led2(0);
//  led4(0);
}

inline void VCO_Tropical_audio_() {
  uint32_t tmp, tmp2;
  uint32_t out, out2;

  VCO1_phase   += increment_0<<3;
  VCO_H1_phase += increment_1<<3;
  VCO_H2_phase += increment_2<<3;
  VCO_H3_phase += increment_3<<3;
    
  out = fast_sin(VCO1_phase);
  out2 = fast_sin(VCO1_phase<<1);

  switch(get_toggle()) {
    case 0: // sin
      tmp = fast_sin(VCO_H1_phase);
      tmp2 = CV_filter16_out[index_filter_pot4]<<16;
      tmp2 = 0xFFFFFFFF - tmp2;
      tmp = max(tmp,tmp2)-tmp2;
      out = max(out,tmp);
      out2 = max(out2,tmp);
    
      tmp = fast_sin(VCO_H2_phase);
      tmp2 = CV_filter16_out[index_filter_pot6]<<16;
      tmp2 = 0xFFFFFFFF - tmp2;
      tmp = max(tmp,tmp2)-tmp2;
      out = max(out,tmp);
      out2 = max(out2,tmp);
    
      tmp = fast_sin(VCO_H3_phase);
      tmp2 = CV_filter16_out[index_filter_pot8]<<16;
      tmp2 = 0xFFFFFFFF - tmp2;
      tmp = max(tmp,tmp2)-tmp2;
      out = max(out,tmp);
      out2 = max(out2,tmp);
    break;
    case 1: // saw
      tmp = VCO_H1_phase;
      tmp2 = CV_filter16_out[index_filter_pot4]<<16;
      tmp2 = 0xFFFFFFFF - tmp2;
      tmp = max(tmp,tmp2)-tmp2;
      out = max(out,tmp);
      out2 = max(out2,tmp);
    
      tmp = VCO_H2_phase;
      tmp2 = CV_filter16_out[index_filter_pot6]<<16;
      tmp2 = 0xFFFFFFFF - tmp2;
      tmp = max(tmp,tmp2)-tmp2;
      out = max(out,tmp);
      out2 = max(out2,tmp);
    
      tmp = VCO_H3_phase;
      tmp2 = CV_filter16_out[index_filter_pot8]<<16;
      tmp2 = 0xFFFFFFFF - tmp2;
      tmp = max(tmp,tmp2)-tmp2;
      out = max(out,tmp);
      out2 = max(out2,tmp);
    break;
    case 2: // square
      tmp = VCO_H1_phase<0x80000000?0:0xFFFFFFFF;
      tmp2 = CV_filter16_out[index_filter_pot4]<<16;
      tmp2 = 0xFFFFFFFF - tmp2;
      tmp = max(tmp,tmp2)-tmp2;
      out = max(out,tmp);
      out2 = max(out2,tmp);
    
      tmp = VCO_H2_phase<0x80000000?0:0xFFFFFFFF;
      tmp2 = CV_filter16_out[index_filter_pot6]<<16;
      tmp2 = 0xFFFFFFFF - tmp2;
      tmp = max(tmp,tmp2)-tmp2;
      out = max(out,tmp);
      out2 = max(out2,tmp);
    
      tmp = VCO_H3_phase<0x80000000?0:0xFFFFFFFF;
      tmp2 = CV_filter16_out[index_filter_pot8]<<16;
      tmp2 = 0xFFFFFFFF - tmp2;
      tmp = max(tmp,tmp2)-tmp2;
      out = max(out,tmp);
      out2 = max(out2,tmp);
    break;    
  }
  macro_out_stereo
}

