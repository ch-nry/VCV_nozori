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

// macro definition

#define _min(X, Y) (((X) < (Y)) ? (X) : (Y))
#define _max(X, Y) (((X) > (Y)) ? (X) : (Y))
#define min(X, Y) (((X) < (Y)) ? (X) : (Y))
#define max(X, Y) (((X) > (Y)) ? (X) : (Y))

#define saturate_S31(in, out) out = clamp(in,-1073741824, 1073741823 )
#define saturate_S30(in, out) out = clamp(in, -536870912,  536870911 )
#define saturate_S29(in, out) out = clamp(in, -268435456,  268435455 )

#define test_connect_loop_start
#define test_connect_loop_test_cv1
#define test_connect_loop_test_cv2
#define test_connect_loop_test_cv3
#define test_connect_loop_test_cv4
#define test_connect_loop_test_in2
#define test_connect_loop_test_end

#define macro_fq_in_tlg                                     \
int32_t freq;                                               \
  switch (get_toggle()) {                                   \
  case 0 : /* High : full audio range*/                     \
    freq = (CV_filter16_out[index_filter_pot1])<<11;        \
    freq += 0x07C00000;                                     \
  break;                                                    \
  case 1 : /* medium (for external control using 1V/Oct) */ \
    freq = (CV_filter16_out[index_filter_pot1])<<9;         \
    freq += 0x09000000;                                     \
  break;                                                    \
  case 2 :  /* low (LFO) */                                 \
    freq = (CV_filter16_out[index_filter_pot1])<<11;        \
    freq += 0x03000000;                                     \
  break;                                                    \
  }

#define macro_fq_in_tlg_pot(pot)                            \
  switch (get_toggle()) {                                   \
  case 0 : /* High : full audio range*/                     \
    freq = (CV_filter16_out[index_filter_ ## pot])<<11;     \
    freq += 0x07C00000;                                     \
  break;                                                    \
  case 1 : /* medium (for external control using 1V/Oct) */ \
    freq = (CV_filter16_out[index_filter_ ## pot])<<9;      \
    freq += 0x09000000;                                     \
  break;                                                    \
  case 2 :  /* low (LFO) */                                 \
    freq = (CV_filter16_out[index_filter_ ## pot])<<11;     \
    freq += 0x03000000;                                     \
  break;                                                    \
  }

#define macro_fq_in                                         \
int32_t freq;                                               \
  freq = (CV_filter16_out[index_filter_pot1])<<11;          \
  freq += 0x06000000;

#define macro_fq_in_novar                                   \
  freq = (CV_filter16_out[index_filter_pot1])<<11;          \
  freq += 0x06000000;

#define macro_1VOct_CV1                                     \
  if (CV1_connect < 60) {                                   \
    tmpS = CV_filter16_out[index_filter_cv1];               \
    tmpS -= CV1_0V;                                         \
    tmpS *= CV1_1V;                                         \
    freq += tmpS;                                           \
  }

#define macro_1VOct_CV1_Q                                   \
  if (CV1_connect < 60) {                                   \
    tmpS = CV_filter16_out[index_filter_cv1];               \
    tmpS -= CV1_0V;                                         \
    tmpS *= CV1_1V;                                         \
    tmpS &= 0xFFFFF000;                                      \
    freq += tmpS;                                           \
  }
 
#define macro_1VOct_IN1                                     \
  if (IN1_connect < 60) {                                   \
    tmpS = audio_inL>>16;                                   \
    tmpS -= IN1_0V>>16;                                     \
    tmpS *= IN1_1V;                                         \
    freq += tmpS;                                           \
  }

#define macro_FqMod_fine_simple(pot)                        \
    freq += CV_filter16_out[index_filter_ ## pot]*(48<<2);  \

#define macro_FqMod_fine(pot,CV)                            \
  if (CV ## _connect < 60) {                                \
    tmpS = CV_filter16_out[index_filter_ ## CV];            \
    tmpS -= CV ## _0V;                                      \
    tmpS *= min(4086<<4,CV_filter16_out[index_filter_ ## pot]);\
    tmpS /= (4086<<4);                                      \
    tmpS *= CV ## _1V;                                      \
    freq += tmpS;                                           \
  }                                                         \
  else { /* fine tune */                                    \
    freq += CV_filter16_out[index_filter_ ## pot]*(48<<2);  \
  }

#define macro_FqMod_fine_IN1(pot)                           \
  if (IN1_connect < 60) {                                   \
    tmpS = audio_inL>>17;                                   \
    tmpS -= IN1_0V>>17;                                     \
    tmpS *= min(4086<<4, CV_filter16_out[index_filter_ ## pot]); \
    tmpS /= (4086<<3);                                      \
    tmpS *= IN1_1V;                                         \
    freq += tmpS;                                           \
  }                                                         \
  else { /* fine tune */                                    \
    freq += CV_filter16_out[index_filter_ ## pot]*(48<<2);  \
  }

#define macro_FqMod_fine_IN2(pot)                           \
  if (IN2_connect < 60) {                                   \
    tmpS = audio_inR>>17;                                   \
    tmpS -= IN2_0V>>17;                                     \
    tmpS *= min(4086<<4, CV_filter16_out[index_filter_ ## pot]); \
    tmpS /= (4086<<3);                                      \
    tmpS *= IN2_1V;                                         \
    freq += tmpS;                                           \
  }                                                         \
  else { /* fine tune */                                    \
    freq += CV_filter16_out[index_filter_ ## pot]*(48<<2);  \
  }

// overflow possible si CV-CV_0V n'est pas centré
#define macro_FqMod(pot,CV)                                 \
  if (CV ## _connect < 60) {                                \
    tmpS = CV_filter16_out[index_filter_ ## CV];            \
    tmpS -= CV ## _0V;                                      \
    tmpS *= CV_filter16_out[index_filter_ ## pot];          \
    tmpS >>= 16;                                            \
    tmpS *= CV ## _1V;                                      \
    freq += tmpS;                                           \
  }

#define macro_FqMod_value(pot, CV, value)                   \
  tmpS = value;                                             \
  tmpS *= CV_filter16_out[index_filter_ ## pot];            \
  tmpS >>= 16;                                              \
  tmpS *= CV ## _1V;                                        \
  freq += tmpS;

    
# define macro_fq2increment                                 \
uint32_t freq_MSB, freq_LSB;                                \
uint32_t increment1, increment2;                            \
  freq = _min(0xFA00000, freq); /*20KHz max*/               \
  freq = _max(0, freq);                                     \
  freq_MSB = freq >> 18; /* keep the 1st 10 bits */         \
  freq_LSB = freq & 0x3FFFF; /* other 18  bits */           \
  increment1 = table_CV2increment[freq_MSB];                \
  increment2 = table_CV2increment[freq_MSB+1];              \
  increment2 -= increment1;                                 \
  increment1 += ((increment2>>8)*(freq_LSB>>2))>>8;         

# define macro_fq2increment_novar                           \
  freq = _min(0xFA00000, freq); /*20KHz max*/               \
  freq = _max(0, freq);                                     \
  freq_MSB = freq >> 18; /* keep the 1st 10 bits */         \
  freq_LSB = freq & 0x3FFFF; /* other 18  bits */           \
  increment1 = table_CV2increment[freq_MSB];                \
  increment2 = table_CV2increment[freq_MSB+1];              \
  increment2 -= increment1;                                 \
  increment1 += ((increment2>>8)*(freq_LSB>>2))>>8;         

# define macro_fq2cutoff                                    \
uint32_t freq_MSB, freq_LSB;                                \
uint32_t increment1, increment2;                            \
  freq = _min(0xFA00000, freq); /*20KHz max*/               \
  freq = _max(0, freq);                                     \
  freq_MSB = freq >> 18; /* keep the 1st 10 bits */         \
  freq_LSB = freq & 0x3FFFF; /* other 18  bits */           \
  increment1 = table_CV2cutof[freq_MSB];                    \
  increment2 = table_CV2cutof[freq_MSB+1];                  \
  increment2 -= increment1;                                 \
  increment1 += ((increment2>>8)*(freq_LSB>>2))>>8;         

# define macro_fq2cutoff_novar                              \
  freq = _min(0xFA00000, freq); /*20KHz max*/               \
  freq = _max(0, freq);                                     \
  freq_MSB = freq >> 18; /* keep the 1st 10 bits */         \
  freq_LSB = freq & 0x3FFFF; /* other 18  bits */           \
  increment1 = table_CV2cutof[freq_MSB];                    \
  increment2 = table_CV2cutof[freq_MSB+1];                  \
  increment2 -= increment1;                                 \
  increment1 += ((increment2>>8)*(freq_LSB>>2))>>8;         

// ---------------------------------------------------------------------

# define macro_out_gain_pan                                           \
uint32_t macro_panR, macro_panL, macro_outL, macro_outR;              \
int32_t macro_outS, macro_tmp2, macro_tmp3;                \
  if (IN1_connect < 60) {                                             \
    macro_outS = (audio_inL>>1) - (IN1_0V>>1);                        \
    macro_outS = _min(0x2AAAAAAA,macro_outS);                         \
    macro_outS += macro_outS>>1;                                      \
    macro_outS = _max(0,macro_outS);                                  \
    macro_outS = m_u32xu32_u32H(macro_outS, macro_outS);               \
    out = m_s32xs32_s32H(out^0x80000000, macro_outS);                  \
    out <<= 4;                                                        \
    out ^= 0x80000000;                                                \
  }                                                                   \
  if (IN2_connect < 60) {                                             \
    out ^= 0x80000000;                                                \
    macro_panR = audio_inR;                                           \
    macro_panR = max(macro_panR, 0x3FFFFFC0);                         \
    macro_panR = min(macro_panR - 0x3FFFFFC0, 0x7FFFFF80);            \
    macro_panR <<= 1;                                                 \
    macro_panL = macro_panR;                                          \
    macro_panL = fast_sin((macro_panL>>2) + 0x40000000);              \
    macro_panL -= 0x7FFFFFFF;                                         \
    macro_panR = fast_sin(macro_panR>>2);                             \
    macro_panR -= 0x7FFFFFFF;                                         \
    macro_outL = m_s32xs32_s32H(macro_panL, out);                      \
    macro_outR = m_s32xs32_s32H(macro_panR, out);                      \
    audio_outR = (macro_outR<<1)^0x80000000;                          \
    audio_outL = (macro_outL<<1)^0x80000000;                          \
  }                                                                   \
  else {                                                              \
    macro_out                                                         \
  }

# define macro_out_gain                                               \
uint32_t macro_panR, macro_panL, macro_outL, macro_outR;              \
int32_t macro_outS, macro_tmp, macro_tmp2, macro_tmp3;                \
  if (IN1_connect < 60) {                                             \
    macro_outS = (audio_inL>>1) - (IN1_0V>>1);                        \
    macro_outS = _min(0x2AAAAAAA,macro_outS);                         \
    macro_outS += macro_outS>>1;                                      \
    macro_outS = _max(0,macro_outS);                                 \
    macro_outS = m_u32xu32_u32H(macro_outS, macro_outS);   \
    out = m_s32xs32_s32H(out^0x80000000, macro_outS);      \
    out <<= 4;                                                        \
    out ^= 0x80000000;                                                \
  }                                                                   \
  macro_out                                                         


  # define macro_out_gainL_stereo                                     \
  int32_t macro_outS;                \
  if (IN1_connect < 60) {                                             \
    macro_outS = (audio_inL>>1) - (IN1_0V>>1);                        \
    macro_outS = _min(0x2AAAAAAA,macro_outS);                         \
    macro_outS += macro_outS>>1;                                      \
    macro_outS = _max(0,macro_outS);                                  \
    macro_outS = m_u32xu32_u32H(macro_outS, macro_outS);   \
    out = m_s32xs32_s32H(out^0x80000000, macro_outS);      \
    out <<= 4;                                                        \
    out ^= 0x80000000;                                                \
    out2 = m_s32xs32_s32H(out2^0x80000000, macro_outS);    \
    out2 <<= 4;                                                       \
    out2 ^= 0x80000000;                                               \
  }                                                                   \
  macro_out_stereo

  # define macro_out_gainR_stereo                                     \
uint32_t macro_panR, macro_panL, macro_outL, macro_outR;              \
int32_t macro_outS, macro_tmp, macro_tmp2, macro_tmp3;                \
  if (IN2_connect < 60) {                                             \
    macro_outS = (audio_inR>>1) - (IN2_0V>>1);                        \
    macro_outS = _min(0x2AAAAAAA,macro_outS);                         \
    macro_outS += macro_outS>>1;                                      \
    macro_outS = _max(0,macro_outS);                                  \
    macro_outS = m_u32xu32_u32H(macro_outS, macro_outS);   \
    out = m_s32xs32_s32H(out^0x80000000, macro_outS);      \
    out <<= 4;                                                        \
    out ^= 0x80000000;                                                \
    out2 = m_s32xs32_s32H(out2^0x80000000, macro_outS);    \
    out2 <<= 4;                                                       \
    out2 ^= 0x80000000;                                               \
  }                                                                   \
  macro_out_stereo

  
# define macro_out_gain_pan_stereo                                    \
uint32_t macro_panR, macro_panL, macro_outL, macro_outR;              \
int32_t macro_outS;                \
  if (IN1_connect < 60) {                                             \
    macro_outS = (audio_inL>>1) - (IN1_0V>>1);                        \
    macro_outS = _min(0x2AAAAAAA,macro_outS);                         \
    macro_outS += macro_outS>>1;                                      \
    macro_outS = _max(0,macro_outS);                                  \
    macro_outS = m_u32xu32_u32H(macro_outS, macro_outS);   \
    out = m_s32xs32_s32H(out^0x80000000, macro_outS);      \
    out <<= 4;                                                        \
    out ^= 0x80000000;                                                \
    out2 = m_s32xs32_s32H(out2^0x80000000, macro_outS);    \
    out2 <<= 4;                                                       \
    out2 ^= 0x80000000;                                               \
  }                                                                   \
  if (IN2_connect < 60) {                                             \
    out ^= 0x80000000;                                                \
    macro_panR = audio_inR;                                           \
    macro_panR = max(macro_panR, 0x3FFFFFC0);                         \
    macro_panR = min(macro_panR - 0x3FFFFFC0, 0x7FFFFF80);            \
    macro_panR <<= 1;                                                 \
    macro_panL = macro_panR;                                          \
    macro_panL = fast_sin((macro_panL>>2) + 0x40000000);              \
    macro_panL -= 0x7FFFFFFF;                                         \
    macro_panR = fast_sin(macro_panR>>2);                             \
    macro_panR -= 0x7FFFFFFF;                                         \
    macro_outL = m_s32xs32_s32H(macro_panL, out);           \
    macro_outR = m_s32xs32_s32H(macro_panR, out);           \
    audio_outR = (macro_outR<<1)^0x80000000;                          \
    audio_outL = (macro_outL<<1)^0x80000000;                          \
  }                                                                   \
  else {                                                              \
    macro_out_stereo                                                  \
  }

# define macro_out_gain_pan_stereo2                                   \
uint32_t macro_panR, macro_panL, macro_outL, macro_outR;              \
int32_t macro_outS;                \
  if (IN1_connect < 60) {                                             \
    macro_outS = (audio_inL>>1) - (IN1_0V>>1);                        \
    macro_outS = _min(0x2AAAAAAA,macro_outS);                         \
    macro_outS += macro_outS>>1;                                      \
    macro_outS = _max(0,macro_outS);                                  \
    macro_outS = m_u32xu32_u32H(macro_outS, macro_outS);   \
    out = m_s32xs32_s32H(out^0x80000000, macro_outS);      \
    out <<= 4;                                                        \
    out ^= 0x80000000;                                                \
    out2 = m_s32xs32_s32H(out2^0x80000000, macro_outS);    \
    out2 <<= 4;                                                       \
    out2 ^= 0x80000000;                                               \
  }                                                                   \
  if (IN2_connect < 60) {                                             \
    out ^= 0x80000000;                                                \
    out2 ^= 0x80000000;                                               \
    macro_panR = audio_inR;                                           \
    macro_panR = max(macro_panR, 0x3FFFFFC0);                         \
    macro_panR = min(macro_panR - 0x3FFFFFC0, 0x7FFFFF80);            \
    macro_panR <<= 1;                                                 \
    macro_panL = macro_panR;                                          \
    macro_panL = fast_sin((macro_panL>>2) + 0x40000000);              \
    macro_panL -= 0x7FFFFFFF;                                         \
    macro_panR = fast_sin(macro_panR>>2);                             \
    macro_panR -= 0x7FFFFFFF;                                         \
    macro_outL = m_s32xs32_s32H(macro_panL, out);           \
    macro_outR = m_s32xs32_s32H(macro_panR, out2);          \
    audio_outR = (macro_outR<<1)^0x80000000;                          \
    audio_outL = (macro_outL<<1)^0x80000000;                          \
  }                                                                   \
  else {                                                              \
    macro_out_stereo                                                  \
  }

#define macro_out_pan                                                 \
uint32_t macro_panR, macro_panL, macro_outL, macro_outR;   \
  if (IN2_connect < 60) {                                             \
    out ^= 0x80000000;                                                \
    macro_panR = audio_inR;                                           \
    macro_panR = max(macro_panR, 0x3FFFFFC0);                         \
    macro_panR = min(macro_panR - 0x3FFFFFC0, 0x7FFFFF80);            \
    macro_panR <<= 1;                                                 \
    macro_panL = macro_panR;                                          \
    macro_panL = fast_sin((macro_panL>>2) + 0x40000000);              \
    macro_panL -= 0x7FFFFFFF;                                         \
    macro_panR = fast_sin(macro_panR>>2);                             \
    macro_panR -= 0x7FFFFFFF;                                         \
    macro_outL = m_s32xs32_s32H(macro_panL, out);           \
    macro_outR = m_s32xs32_s32H(macro_panR, out);           \
    audio_outR = (macro_outR<<1)^0x80000000;                          \
    audio_outL = (macro_outL<<1)^0x80000000;                          \
  }                                                                   \
  else {                                                              \
    macro_out                                                         \
  }

#define macro_out_pan_stereo                                          \
uint32_t macro_panR, macro_panL, macro_outL, macro_outR;   \
  if (IN2_connect < 60) {                                             \
    out ^= 0x80000000;                                                \
    macro_panR = audio_inR;                                           \
    macro_panR = max(macro_panR, 0x3FFFFFC0);                         \
    macro_panR = min(macro_panR - 0x3FFFFFC0, 0x7FFFFF80);            \
    macro_panR <<= 1;                                                 \
    macro_panL = macro_panR;                                          \
    macro_panL = fast_sin((macro_panL>>2) + 0x40000000);              \
    macro_panL -= 0x7FFFFFFF;                                         \
    macro_panR = fast_sin(macro_panR>>2);                             \
    macro_panR -= 0x7FFFFFFF;                                         \
    macro_outL = m_s32xs32_s32H(macro_panL, out);           \
    macro_outR = m_s32xs32_s32H(macro_panR, out);           \
    audio_outR = (macro_outR<<1)^0x80000000;                          \
    audio_outL = (macro_outL<<1)^0x80000000;                          \
  }                                                                   \
  else {                                                              \
    macro_out_stereo                                                  \
  }


# define macro_out_pan_novar                                          \
  if (IN2_connect < 60) {                                             \
    out ^= 0x80000000;                                                \
    macro_panR = audio_inR;                                           \
    macro_panR = max(macro_panR, 0x3FFFFFC0);                         \
    macro_panR = min(macro_panR - 0x3FFFFFC0, 0x7FFFFF80);            \
    macro_panR <<= 1;                                                 \
    macro_panL = macro_panR;                                          \
    macro_panL = fast_sin((macro_panL>>2) + 0x40000000);              \
    macro_panL -= 0x7FFFFFFF;                                         \
    macro_panR = fast_sin(macro_panR>>2);                             \
    macro_panR -= 0x7FFFFFFF;                                         \
    macro_outL = m_s32xs32_s32H(macro_panL, out);           \
    macro_outR = m_s32xs32_s32H(macro_panR, out);           \
    audio_outR = (macro_outR<<1)^0x80000000;                          \
    audio_outL = (macro_outL<<1)^0x80000000;                          \
  }                                                                   \
  else {                                                              \
    macro_out                                                         \
  }


# define macro_out                                                    \
    audio_outL = out;                                                 \
    audio_outR = out;

# define macro_out_stereo                                             \
    audio_outL = out;                                                 \
    audio_outR = out2;

