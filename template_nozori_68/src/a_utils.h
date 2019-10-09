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


#define _m_u32xu32_u32(in1, in2, outH, outL) \
   outH =  m_u32xu32_u32H(in1, in2);

#define _m_s32xs32_s32(in1, in2, outH, outL) \
   outH =  m_s32xs32_s32H(in1, in2);

// this file regroup usefull generic function

inline uint32_t m_u32xu32_u32H(uint32_t in1, uint32_t in2) {
  //uint32_t outH, outL;
  //asm volatile("umull %0, %1, %2, %3" : "=r" (outL), "=r" (outH) : "r" (in1), "r" (in2));
  //return(outH);
  uint64_t tmp;
  tmp = (uint64_t)in1 * (uint64_t)in2;
  tmp>>=32;
  return((uint32_t)tmp);
}

inline uint32_t m_s32xs32_s32H(int32_t in1, int32_t in2) {
  //int32_t outH, outL;
  //asm volatile("smull %0, %1, %2, %3" : "=r" (outL), "=r" (outH) : "r" (in1), "r" (in2));
  //return(outH);
  int64_t tmp;
  tmp = (int64_t)in1 * (int64_t)in2;
  tmp>>=32;
  return((int32_t)tmp);
}



inline uint32_t MIX16U(uint32_t src, uint32_t dest, uint32_t factor) { // Bug qd le resultat est trop grand et deviens negatif
  int32_t tmp;
  tmp = dest-src;
  tmp *= factor;
  tmp >>= 16;
  tmp += src;
  return(tmp);
}

inline uint32_t MIX16S(int32_t src, int32_t dest, uint32_t factor) { 
  int32_t tmp;
  tmp = dest-src;
  tmp *= factor;
  tmp >>= 16;
  tmp += src;
  return(tmp);
}

inline uint32_t filter(uint32_t goal, uint32_t current, uint32_t filter) {
  int32_t diff;
  uint32_t result;
  
  diff =  goal - current;
  result = current + (diff >> filter);
  return(result);  
}

inline uint32_t filterS(int32_t goal, int32_t current, uint32_t filter) {
  int32_t diff;
  int32_t result;
  
  diff =  goal - current;
  result = current + (diff >> filter);
  return(result);  
}

inline uint32_t median3 (uint32_t last3, uint32_t last2, uint32_t last1) {
 return(max(max(min(last1, last2), min(last1, last3)),min(last3, last2)));
}

inline uint32_t fast_sin(const uint32_t phase) {
  uint32_t sinus, from_table;
  int32_t diff;
  
  from_table = table_sinus_diff[phase>>19];
  sinus = (from_table & 0xFFFFF800);
  from_table <<= 21;
  diff = (int32_t)from_table >> 21; // passage en signed
  diff *= (int32_t)(((phase>>3) & 0xFFFF)>>5);
  return (sinus + diff);
}


// 4 point interpolation
// all input on 16 bits
inline int32_t tabread4(int32_t a, int32_t b, int32_t c, int32_t d, int32_t f) {
  int32_t a1, a2, a3;
  int32_t out;
  
  a1 = (c-a)/2;
  a2 = a - (5*b)/2 +2*c -d/2;
  a3 = (d-a)/2 + 3*(b-c)/2;

  out = (a3 * f) >> 16;
  out += a2;
  out = (out * f) >> 16;
  out += a1;
  out = (out * f) >> 16;
  out += b;
  return(out);
}

// band limited saw generation
inline uint32_t sawBL(uint32_t phase, uint32_t BLsize) {
  uint32_t sinc, table_index;

  table_index = phase;
  table_index = (table_index & 0x80000000)? -table_index: table_index;
  sinc = m_u32xu32_u32H(table_index, BLsize); 
  sinc = (sinc & 0xFFFFF000)? 0xFFF : sinc; 
  sinc = table_Isinc[sinc];
  sinc = m_s32xs32_s32H(phase^0x80000000, (sinc << 16)^0x80000000);
  sinc -=  phase >> 1;
  sinc -= 0x40000000;
  sinc += sinc >>1;
  sinc -= 0x40000000;
  return(sinc);
}

