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

float SR_needed;

// ------------------------------------------------------------------------------------------------------------
// input value when no jack are connected
inline void default5(uint32_t in){}
inline void default6(uint32_t in){}


// ------------------------------------------------------------------------------------------------------------
// Not used function
inline void  noInterrupts() {}
inline void  interrupts() {}

// function used to switch to 48HKZ
inline void send_dac(uint32_t, uint32_t) { SR_needed = 48000.; }

// ------------------------------------------------------------------------------------------------------------
// random

inline void init_random() { 
    random::init();
    rnd_seed = random::u32();
}

inline uint32_t pseudo_rnd(uint32_t seed) {
  uint32_t rnd_bit;
  uint32_t tmp = seed;

  rnd_bit  = ((seed >> 0) ^ (seed >> 2) ^ (seed >> 3) ^ (seed >> 5) );
  tmp =  (seed >> 1) | (rnd_bit << 31);
  return(tmp);
}
inline uint32_t random32() {
  rnd_seed =  pseudo_rnd(rnd_seed); 
  return(rnd_seed);
}

// ------------------------------------------------------------------------------------------------------------
void test_connect_loop_68() {} // No need to test the phisical connection since VCV it's accesible in VCV API
void test_connect_loop_84() {}

// ------------------------------------------------------------------------------------------------------------
// LEDS
//  replace fonction that control nozori hardware led with simple VCV function
inline void led2(uint32_t value) { this->lights[LED2_LIGHT].setBrightness((float)value/256); }
inline void led4(uint32_t value) { this->lights[LED4_LIGHT].setBrightness((float)value/256); }

inline void set_led2(uint32_t value) { this->lights[LED2_LIGHT].setBrightness((float)value/128); }
inline void set_led4(uint32_t value) { this->lights[LED4_LIGHT].setBrightness((float)value/128); }



// ------------------------------------------------------------------------------------------------------------
// toggle

inline uint32_t get_toggle() { 
 return(2 - params[SWITCH_PARAM].getValue()); 
}

// ------------------------------------------------------------------------------------------------------------
// chaos for default modulation
int32_t chaos_X, chaos_Y, chaos_Z;
int32_t chaos_dx, chaos_dy, chaos_dz;

inline void init_chaos() {
  chaos_X = random32();
  chaos_Y = random32();
  chaos_Z = random32();
}

inline void chaos(uint32_t dt) { // Thomas chaotic attractor
  // smooth movement is obtained using the derivative of the output, with b= 0
  int32_t chaos_X_local,chaos_Y_local, chaos_Z_local;
  chaos_X_local = chaos_X;
  chaos_Y_local = chaos_Y;
  chaos_Z_local = chaos_Z;
  
  chaos_dx = fast_sin(chaos_Y_local)^0x80000000;
  chaos_dy = fast_sin(chaos_Z_local)^0x80000000;
  chaos_dz = fast_sin(chaos_X_local)^0x80000000;
  
  chaos_X = chaos_X_local + (chaos_dx>>dt);
  chaos_Y = chaos_Y_local + (chaos_dy>>dt);
  chaos_Z = chaos_Z_local + (chaos_dz>>dt);
}

inline void chaos_div(uint32_t diviseur) { // Thomas chaotic attractor
  // smooth movement is obtained using the derivative of the output, with b= 0
  int32_t chaos_X_local,chaos_Y_local, chaos_Z_local;
  chaos_X_local = chaos_X;
  chaos_Y_local = chaos_Y;
  chaos_Z_local = chaos_Z;
  
  chaos_dx = fast_sin(chaos_Y_local)^0x80000000;
  chaos_dy = fast_sin(chaos_Z_local)^0x80000000;
  chaos_dz = fast_sin(chaos_X_local)^0x80000000;
  
  chaos_X = chaos_X_local + (chaos_dx/diviseur);
  chaos_Y = chaos_Y_local + (chaos_dy/diviseur);
  chaos_Z = chaos_Z_local + (chaos_dz/diviseur);
}


