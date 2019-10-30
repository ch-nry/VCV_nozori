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

// RND LOOP : random value to generate a slowlly evolving LFO, or VCO
// Pot 1 : clock fq
// Pot 2 : Mod clock
// Pot 3 : nb step (1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 24, 32)
// Pot 4 : nb step modulation 
// Pot 5 : mutate speed
// Pot 6 : mutation speed modulation
// CV 1 : clock syncro
// CV 2 : clock modulation value
// CV 3 : step modulation value
// CV 4 : Mutation speed modulation value
// IN 1 : change all step
// IN 2 : change next step
// Selecteur3 : interpolation : none / linear / bubic
// OUT 1 : OUT X 
// OUT 2 : OUT Y
// LED 1 : out X
// LED 2 : out Y

inline void RND_LOOP_init_() {
  uint32_t i;
  for (i=0; i<32; i++) { thomas[0][i] = random32(); }
  for (i=0; i<32; i++) { thomas[1][i] = random32(); }
  for (i=0; i<32; i++) { thomas[2][i] = random32(); }
  // current_interpol_valueX = ; ...
  rnd_all_save = 0;
  init_chaos();
}

inline void RND_LOOP_loop_() {
  int32_t tmpS, CV3_value, CV4_value;
  uint32_t i, tmp, toggle;
  int32_t chaosX, chaosY, chaosZ;
  int32_t chaosdX, chaosdY, chaosdZ;
  uint32_t variation_speed;
  int32_t pot3_tmp, pot5_tmp;
  int32_t freq;
  
  filter16_nozori_68
  test_connect_loop_68();
  toggle = get_toggle();
  toggle_global = toggle;
  chaos(16); // for default mod values

  if (CV3_connect < 60) CV3_value = CV_filter16_out[index_filter_cv3] - CV3_0V; else CV3_value = chaos_dx>>16;
  if (CV4_connect < 60) CV4_value = CV_filter16_out[index_filter_cv4] - CV4_0V; else CV4_value = chaos_dy>>16;
  CV3_value = min(0x7FFF,max(-0x7FFF,CV3_value));
  CV4_value = min(0x7FFF,max(-0x7FFF,CV4_value));
  
  // CLOCK
  if (CV1_connect < 60) { // syncro sur l'entree
    freq = (CV_filter16_out[index_filter_pot1] + 4095) / 8192;
    tmp = (CV_filter16_out[index_filter_pot2] + 5461)/10923; // from 0.5 to 6.5
    clock_diviseur = tab_diviseur[freq] * tab_diviseur2[tmp];
    clock_multiplieur = tab_multiplieur[freq] * tab_multiplieur2[tmp];
  }
  else { // pas de syncro, on calcul l'increment normallement
    freq = (CV_filter16_out[index_filter_pot1])<<11;
    freq += 0x01000000;
    macro_FqMod_fine(pot2, CV2)
    macro_fq2increment
    LFO1_increment = increment1; //save
  }
  
  // STEP
  pot3_tmp = CV_filter16_out[index_filter_pot3];
  tmpS = CV3_value;
  tmpS *= CV_filter16_out[index_filter_pot4];
  tmpS >>= 16;
  pot3_tmp += tmpS;
  pot3_tmp = min(0xFFFF, max(0,pot3_tmp));
  tmp = pot3_tmp / 5462;//(0x10000/12); // 12 possible step value
  nb_step = possible_step[tmp];

  // variation speed
  pot5_tmp = CV_filter16_out[index_filter_pot5];
  tmpS = CV4_value;
  tmpS *= CV_filter16_out[index_filter_pot6];
  tmpS >>= 16;
  pot5_tmp += tmpS;
  pot5_tmp = min(0xFFFF, max(0,pot5_tmp));
  pot5_tmp *= pot5_tmp>>1;
  variation_speed = pot5_tmp >>15;
  
  // rnd all
  if ( (IN1_connect < 60) && (rnd_all_save == 0) && (audio_inL>0xB0000000) ) {
    rnd_all_save = 1;
    for (i=0; i<32; i++) { thomas[0][i] = random32(); }
    for (i=0; i<32; i++) { thomas[1][i] = random32(); }
    for (i=0; i<32; i++) { thomas[2][i] = random32(); }
  }
  else if ((IN1_connect < 60) && (audio_inL < 0xA0000000)) {
    rnd_all_save = 0;   
  }
  
  if (reset1 == 1) { // fin d'un step, on calcul le deplacement d'une valeur : pas possible de le faire en audio car trop lent
        
    loop_index = (loop_index+1) % nb_step;
   
    if ((IN2_connect < 60) && (audio_inR > 0xB0000000)) { // rnd 1 step
      thomas[0][loop_index] = random32();
      thomas[1][loop_index] = random32();
      thomas[2][loop_index] = random32();
    } 
   
    chaosX = thomas[0][loop_index];
    chaosY = thomas[1][loop_index];
    chaosZ = thomas[2][loop_index];
    chaosdX = fast_sin(chaosY)^0x80000000;
    chaosdY = fast_sin(chaosZ)^0x80000000;
    chaosdZ = fast_sin(chaosX)^0x80000000;
    
    chaosdX >>= 16;
    chaosdX *= variation_speed;
    chaosdY >>= 16;
    chaosdY *= variation_speed;
    chaosdZ >>= 16;
    chaosdZ *= variation_speed;
    
    thomas[0][loop_index] += chaosdX>>2;
    thomas[1][loop_index] += chaosdY>>2;
    thomas[2][loop_index] += chaosdZ>>2;
    reset1 = 0;
    
  }
  // LEDS
  //tmp = (LFO1_phase & 0x03FFFFFF);
  //led2(tmp>>17);
  //tmp = (LFO1_phase / (nb_step+1));
  //tmp = min(tmp, 0x03FFFFFF);
  //led3(tmp>>17);
  led2(audio_outL>>23);
  led4(audio_outR>>23);
}

inline void RND_LOOP_audio_() {
  uint32_t increment1;
  uint32_t current_step, tmp,tmp2,  outX, outY, toggle;
  int32_t tmpS, outSX, outSY;

  nb_tick = min(0x0FFFFFFF, nb_tick+1); // to prevent overflow with multiplier
  if( (last_clock_ == 0) && (CV1_connect < 60) && (CV_filter16_out[index_filter_cv1] > 0xB000) ) { // mode syncro, on a une syncro
    last_clock_ = 1;
    increment1 = 0xFFFFFFFF / nb_tick;
    increment1 /= clock_diviseur;
    increment1 *= clock_multiplieur;
    LFO1_increment = increment1>>6;
    nb_tick = 0;
  }
  else if ((CV1_connect < 60) && (CV_filter16_out[index_filter_CV1] < 0xA000) ) {
    last_clock_ = 0;
  }
  
  LFO1_phase += LFO1_increment;
  toggle = toggle_global;

  // step
  current_step = LFO1_phase >> 26; // 6 bit for step value
  
  if ( current_step != last_step) {
    current_step = current_step % nb_step;
    LFO1_phase = (current_step << 26) + (LFO1_phase & 0x03FFFFFF);

    reset1 = 1; // pour calculer la prochaine valeur

    previous_interpol_valueX = last_interpol_valueX;
    previous_interpol_valueY = last_interpol_valueY;
    last_interpol_valueX = current_interpol_valueX;
    last_interpol_valueY = current_interpol_valueY;   
    if ( toggle < 2) { // actualise juste la valeur acuel
      current_interpol_valueX =  abs((int32_t)thomas[0][current_step]);
      current_interpol_valueY =  abs((int32_t)thomas[1][current_step]);
    }
    else { // on a besion d'une valeur suplementaire pour l'interpolation cubic
      tmp = (current_step+1) % nb_step;
      current_interpol_valueX =  current_interpol_valueX2;
      current_interpol_valueY =  current_interpol_valueY2;
      current_interpol_valueX2 =  abs((int32_t)thomas[0][tmp]);
      current_interpol_valueY2 =  abs((int32_t)thomas[1][tmp]);
      }
    last_step = current_step;
  }

  switch(toggle) {
  case 0: // saw
    tmpS = current_interpol_valueX - 0x40000000;
    tmpS = tmpS >> 15;
    tmp2 = (LFO1_phase & 0x03FFFFFF)>>10;
    tmpS *= tmp2;
    outX = 0x80000000 - current_interpol_valueX + tmpS;
    tmpS = current_interpol_valueY - 0x40000000;
    tmpS = tmpS >> 16;
    tmpS *= tmp2;
    outY = 0x40000000 + tmpS;
    break;
  case 1: // no interpolation
    outX = current_interpol_valueX;
    outY = current_interpol_valueY;
    break;
/*  case 1: // linear interpolation
    tmpS = current_interpol_valueX - last_interpol_valueX;
    tmpS = tmpS >> 16;
    tmp2 = (LFO1_phase & 0x03FFFFFF)>>10;
    tmpS *= tmp2;
    outX = last_interpol_valueX + tmpS;

    tmpS = current_interpol_valueY - last_interpol_valueY;
    tmpS = tmpS >> 16;
    tmp2 = (LFO1_phase & 0x03FFFFFF)>>10;
    tmpS *= tmp2;
    outY = last_interpol_valueY + tmpS;

    break;*/
  case 2: // cubic interpolation 
    tmp2 = (LFO1_phase & 0x03FFFFFF)>>10;
    outX = tabread4(previous_interpol_valueX>>17, last_interpol_valueX>>17, current_interpol_valueX>>17, current_interpol_valueX2>>17, tmp2);
    outX <<= 17;
    outY = tabread4(previous_interpol_valueY>>17, last_interpol_valueY>>17, current_interpol_valueY>>17, current_interpol_valueY2>>17, tmp2);    
    outY <<= 17;
    break;   
  }

  outSX = outX^0x80000000;
  outSX += 0x40000000;
  outSX += outSX>>2;
  outX = outSX^0x80000000;

  outSY = outY^0x80000000;
  outSY += 0x40000000;
  outSY += outSY>>2;
  outY = outSY^0x80000000;
  
  audio_outL = outX;
  audio_outR = outY;  
}

