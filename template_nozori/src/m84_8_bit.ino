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

// 8 bit oscillator with various mix between diferent frequency
// Pot 1 : FQ 1
// Pot 2 : Mod Fq (or fine if nothing is connected on the modulation)
// Pot 3 : FQ 2
// Pot 4 : MOD FQ 2
// Pot 5 : FQ 3
// Pot 6 : MOD FQ 3
// Pot 7 : FQ 4
// Pot 8 : MOD FQ 4
// IN 1 : 1V/Oct
// IN 2 : MOD FQ
// Selecteur3 : EFFECT TYPE
// OUT 1 : OUT
// OUT 2 : OUT

inline void VCO_8bit_init_() {
  VCO1_phase = 0;
  VCO_H1_phase = 0;
  VCO_H2_phase = 0;
  VCO_H3_phase = 0;
}

inline void VCO_8bit_loop_() {
  int32_t tmpS;

  filter16_nozori_84
  test_connect_loop_84();
  toggle_global = get_toggle();

  // Fq principale
  macro_fq_in_tlg
  macro_1VOct_IN1
  macro_FqMod_fine_IN2(pot2);
  macro_fq2increment
  increment_0 = increment1;

  // Fq harmo 1
  macro_fq_in_tlg_pot(pot3)
  macro_1VOct_IN1
  macro_FqMod_fine_IN2(pot4);
  macro_fq2increment_novar
  increment_1 = increment1;

  // Fq harmo 2
  macro_fq_in_tlg_pot(pot5)
  macro_1VOct_IN1
  macro_FqMod_fine_IN2(pot6);
  macro_fq2increment_novar
  increment_2 = increment1;

  // Fq harmo 3
  macro_fq_in_tlg_pot(pot7)
  macro_1VOct_IN1
  macro_FqMod_fine_IN2(pot8);
  macro_fq2increment_novar
  increment_3 = increment1;

  if (IN1_connect < 60) led2(audio_inL >> 23); else set_led2(0);
  if (IN2_connect < 60) led4(audio_inR >> 23); else set_led4(0);
}

inline void VCO_8bit_audio_() {
  uint32_t out, out2;
  uint32_t toggle;

  toggle = toggle_global;
  
  switch(toggle) {
    case 0 : 
      VCO1_phase += increment_0;
      VCO_H1_phase += increment_1;
      VCO_H2_phase += increment_2;
      VCO_H3_phase += increment_3;
  
      out  = (VCO1_phase + VCO_H1_phase) xor (VCO_H2_phase + VCO_H3_phase);
      out2 = (VCO1_phase + VCO_H3_phase) xor (VCO_H2_phase + VCO_H1_phase);
      break;
    case 1 : 
      VCO1_phase += increment_0<<3;
      VCO_H1_phase += increment_1<<3;
      VCO_H2_phase += increment_2<<3;
      VCO_H3_phase += increment_3<<3;
  
      out  = VCO1_phase xor VCO_H1_phase;// xor VCO_H2_phase;
      out2 = VCO_H3_phase xor VCO_H2_phase;// xor VCO_H3_phase;
      break;
    case 2 : 
      VCO1_phase += increment_0<<5;
      VCO_H1_phase += increment_1<<5;
      VCO_H2_phase += increment_2<<5;
      VCO_H3_phase += increment_3<<5;
  
      out  = (VCO1_phase & VCO_H1_phase) ;
      out2 = (VCO_H2_phase & VCO_H3_phase);
      break;
  }
  out &= 0xFF000000;
  out2 &= 0xFF000000;

  macro_out_stereo
}

