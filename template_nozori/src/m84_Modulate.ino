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

// DIGITAL Modulation of 2 audio sources
// Pot 1 : XOR
// Pot 2 : AND
// Pot 3 : OR
// Pot 4 : MIN
// Pot 5 : MAX
// Pot 6 : ring
// Pot 7 : AM (uint32*int32) // (int32*uint32)
// Pot 8 : abs(2-1) // abs (1-2)
// IN 1 : audio 1
// IN 2 : audio 2
// Selecteur3 : MONO / SPLIT / STEREO
// OUT 1 : OUT 1
// OUT 2 : OUT 2

uint32_t gain1_save, gain2_save, gain3_save, gain4_save, gain5_save, gain6_save, gain7_save, gain8_save;

inline void modulation_init_() {
  gain1_save = 0;
  gain2_save = 0;
  gain3_save = 0;
  gain4_save = 0;
  gain5_save = 0;
  gain6_save = 0;
  gain7_save = 0;
  gain8_save = 0;
}

inline void modulation_loop_() {
  uint32_t toggle_value;
  
  filter16_nozori_84
  test_connect_loop_84();
  toggle_value = get_toggle();
  toggle_global = toggle_value;
  
  // TODO : led
}


inline void modulation_audio_() {
  int32_t modulation1a, modulation2a, modulation3a, modulation4a, modulation5a;
  int32_t modulation1b, modulation2b, modulation3b, modulation4b, modulation5b;
  int32_t modulation6a, modulation6b, modulation7a, modulation7b, modulation8a, modulation8b;
  int32_t modulation, modulationL, modulationR;
  int32_t inL, inR;
  uint32_t tmp, gain1, gain2, gain3, gain4, gain5, gain6, gain7, gain8;
  int32_t tmpS;
  
  if (IN1_connect < 60) { inL = audio_inL^0x80000000; } else {inL = fast_sin(sin1_phase)^0x80000000; inL -= inL>>1;}
  if (IN2_connect < 60) { inR = audio_inR^0x80000000; } else {inR = fast_sin(sin2_phase)^0x80000000; inR -= inR>>1;}

  sin1_phase += 200000;
  sin2_phase += 456789;
  /*
  modulation1 = inL & inR;
  modulation2 = inL | inR;
  modulation3 = min(inL, inR);
  modulation4 = max(inL, inR);
  modulation5 = ((inL>>16)*(inR>>16))<<1;
  modulation6 = (inL>>16)*((inR>>16)+0x7FFF);
//  modulation6b = (inR>>16)*((inL>>16)+0x7FFF);
  modulation7a = -abs(inL);
  modulation7b = abs(inR);
  modulation8a = abs(inL-inR);
  modulation8b = -abs(inR-inL);
*/
  modulationL = 0;
  modulationR = 0;
  modulation = 0;

  gain1 = filter(CV_filter16_out[index_filter_pot1]<<8, gain1_save, 6);
  gain1_save = gain1;
  gain2 = filter(CV_filter16_out[index_filter_pot2]<<8, gain2_save, 6);
  gain2_save = gain2;
  gain3 = filter(CV_filter16_out[index_filter_pot3]<<8, gain3_save, 6);
  gain3_save = gain3;
  gain4 = filter(CV_filter16_out[index_filter_pot4]<<8, gain4_save, 6);
  gain4_save = gain4;
  gain5 = filter(CV_filter16_out[index_filter_pot5]<<8, gain5_save, 6);
  gain5_save = gain5;
  gain6 = filter(CV_filter16_out[index_filter_pot6]<<8, gain6_save, 6);
  gain6_save = gain6;
  gain7 = filter(CV_filter16_out[index_filter_pot7]<<8, gain7_save, 6);
  gain7_save = gain7;
  gain8 = filter(CV_filter16_out[index_filter_pot8]<<8, gain8_save, 6);
  gain8_save = gain8;

  switch (toggle_global) {
    case 0: // 8 effect diferents en mono
      modulation1a = max( -0x2AAAAAAA, min(0x2AAAAAAA, (inL>>16)*(inR>>16))) * 3;
      modulation2a = abs((inL>>1) - (inR>>1)) - (gain2 << 5);
      modulation3a = (inL>>16)*((inR>>16)+0x7FFF);
      modulation4a = (inR>>16)*((inL>>16)+0x7FFF);
      modulation5a = min(inL, inR);
      modulation6a = max(inL, inR);
      modulation7a = inL & inR;
      modulation8a = inL | inR;
  
      modulation += (modulation1a >> 15) * (gain1 >> 12); // 16 bit * 12 bits 
      modulation += (modulation2a >> 14) * (gain2 >> 12); // 16 bit * 12 bits 
      modulation += (modulation3a >> 15) * (gain3 >> 12); // 16 bit * 12 bits 
      modulation += (modulation4a >> 15) * (gain4 >> 12); // 16 bit * 12 bits 
      modulation += (modulation5a >> 15) * (gain5 >> 12); // 16 bit * 12 bits 
      modulation += (modulation6a >> 15) * (gain6 >> 12); // 16 bit * 12 bits 
      modulation += (modulation7a >> 15) * (gain7 >> 12); // 16 bit * 12 bits 
      modulation += (modulation8a >> 15) * (gain8 >> 12); // 16 bit * 12 bits 

      modulationL = modulation;
      modulationR = modulation;
    break;
    case 1: // 4 effet separé  
      modulation1a = (inL>>16)*((inR>>16)+0x7FFF);
      modulation1b = (inR>>16)*((inL>>16)+0x7FFF);
      modulation2a = abs((inL>>1) - (inR>>1)) - (gain3 << 5);
      modulation2b = abs((inL>>1) - (inR>>1)) - (gain4 << 5);
      modulation3a = min(inL, inR);
      modulation3b = max(inL, inR);
      modulation4a = inL & inR;
      modulation4b = inL | inR;
  
      modulationL += (modulation1a >> 15) * (gain1 >> 12); // 16 bit * 12 bits 
      modulationR += (modulation1b >> 15) * (gain2 >> 12); // 16 bit * 12 bits 
      modulationL += (modulation2a >> 14) * (gain3 >> 12); // 16 bit * 12 bits 
      modulationR += (modulation2b >> 14) * (gain4 >> 12); // 16 bit * 12 bits 
      modulationL += (modulation3a >> 15) * (gain5 >> 12); // 16 bit * 12 bits 
      modulationR += (modulation3b >> 15) * (gain6 >> 12); // 16 bit * 12 bits               
      modulationL += (modulation4a >> 15) * (gain7 >> 12); // 16 bit * 12 bits 
      modulationR += (modulation4b >> 15) * (gain8 >> 12); // 16 bit * 12 bits 
    break;
    case 2: // 8 effect stereo
      modulation1a = inL;
      modulation1b = inR;
      tmp = gain2;
      tmp <<= 8;
      tmpS = tmp ^0x80000000;
      modulation2a = min(inL, tmpS);
      modulation2b = min(inR, tmpS);
      modulation3a = ((inL>>16)*(inR>>16))<<1;
      modulation3b= modulation3a;
      modulation4a = ((inL>>16)*((inR>>16)+0x7FFF));
      modulation4b = ((inR>>16)*((inL>>16)+0x7FFF));
      modulation5a = min(inL, inR);
      modulation5b = max(inL, inR);
      modulation6a = (abs(inL) - (gain6 << 5)); 
      modulation6b = (abs(inR) - (gain6 << 5)); 
      modulation7a = abs((inL>>1) - (inR>>1)) - (gain7 << 5);
      modulation7b = abs((inL>>1) - (inR>>1)) - (gain7 << 5);
      modulation8a = inL & inR;
      modulation8b = inL | inR;
  
      modulationL += (modulation1a >> 15) * (gain1 >> 12); // 16 bit * 12 bits 
      modulationR += (modulation1b >> 15) * (gain1 >> 12); // 16 bit * 12 bits 
      modulationL += (modulation2a >> 15) * (gain2 >> 12); // 16 bit * 12 bits 
      modulationR += (modulation2b >> 15) * (gain2 >> 12); // 16 bit * 12 bits 
      modulationL += (modulation3a >> 15) * (gain3 >> 12); // 16 bit * 12 bits 
      modulationR += (modulation3b >> 15) * (gain3 >> 12); // 16 bit * 12 bits 
      modulationL += (modulation4a >> 15) * (gain4 >> 12); // 16 bit * 12 bits 
      modulationR += (modulation4b >> 15) * (gain4 >> 12); // 16 bit * 12 bits 
      modulationL += (modulation5a >> 15) * (gain5 >> 12); // 16 bit * 12 bits       
      modulationR += (modulation5b >> 15) * (gain5 >> 12); // 16 bit * 12 bits 
      modulationL += (modulation6a >> 14) * (gain6 >> 12); // 16 bit * 12 bits 
      modulationR += (modulation6b >> 14) * (gain6 >> 12); // 16 bit * 12 bits 
      modulationL += (modulation7a >> 14) * (gain7 >> 12); // 16 bit * 12 bits 
      modulationR += (modulation7b >> 14) * (gain7 >> 12); // 16 bit * 12 bits 
      modulationL += (modulation8a >> 15) * (gain8 >> 12); // 16 bit * 12 bits 
      modulationR += (modulation8b >> 15) * (gain8 >> 12); // 16 bit * 12 bits 
    break;    
  }

  modulationL = max(-0x0FFFFFFF, min(0x0FFFFFFF, modulationL));
  modulationR = max(-0x0FFFFFFF, min(0x0FFFFFFF, modulationR));

  modulationL <<= 3;
  modulationR <<= 3;
  
  audio_outL = modulationL ^ 0x80000000;
  audio_outR = modulationR ^ 0x80000000;
}

