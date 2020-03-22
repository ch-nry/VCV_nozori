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

#define index_filter_pot1 0
#define index_filter_pot2 1
#define index_filter_pot3 2
#define index_filter_pot4 3
#define index_filter_pot5 4
#define index_filter_pot6 5
#define index_filter_pot7 6
#define index_filter_pot8 7
#define index_filter_CV1 8
#define index_filter_CV2 9
#define index_filter_CV3 10
#define index_filter_CV4 11
#define index_filter_cv1 8
#define index_filter_cv2 9
#define index_filter_cv3 10
#define index_filter_cv4 11

//convert from vcv fader value and CV input to nozori level
#define filter16_nozori_68  \
    CV_filter16_out[index_filter_pot1]= (uint32_t)(65535 * params[POT1_PARAM].getValue()); \
    CV_filter16_out[index_filter_pot2]= (uint32_t)(65535 * params[POT2_PARAM].getValue()); \
    CV_filter16_out[index_filter_pot3]= (uint32_t)(65535 * params[POT3_PARAM].getValue()); \
    CV_filter16_out[index_filter_pot4]= (uint32_t)(65535 * params[POT4_PARAM].getValue()); \
    CV_filter16_out[index_filter_pot5]= (uint32_t)(65535 * params[POT5_PARAM].getValue()); \
    CV_filter16_out[index_filter_pot6]= (uint32_t)(65535 * params[POT6_PARAM].getValue()); \
    CV_filter16_out[index_filter_CV1] = inputs[CV1_INPUT].isConnected()? (uint32_t)(65535 * (clamp(inputs[CV1_INPUT].getVoltage()/10.6,-0.5f,0.5) + 0.5)) : 32768; \
    CV_filter16_out[index_filter_CV2] = inputs[CV2_INPUT].isConnected()? (uint32_t)(65535 * (clamp(inputs[CV2_INPUT].getVoltage()/10.6,-0.5f,0.5) + 0.5)) : 32768; \
    CV_filter16_out[index_filter_CV3] = inputs[CV3_INPUT].isConnected()? (uint32_t)(65535 * (clamp(inputs[CV3_INPUT].getVoltage()/10.6,-0.5f,0.5) + 0.5)) : 32768; \
    CV_filter16_out[index_filter_CV4] = inputs[CV4_INPUT].isConnected()? (uint32_t)(65535 * (clamp(inputs[CV4_INPUT].getVoltage()/10.6,-0.5f,0.5) + 0.5)) : 32768; \
    CV1_connect = inputs[CV1_INPUT].isConnected()? 0 : 100; \
    CV2_connect = inputs[CV2_INPUT].isConnected()? 0 : 100; \
    CV3_connect = inputs[CV3_INPUT].isConnected()? 0 : 100; \
    CV4_connect = inputs[CV4_INPUT].isConnected()? 0 : 100; \
    IN1_connect = inputs[IN1_INPUT].isConnected()? 0 : 100; \
    IN2_connect = inputs[IN2_INPUT].isConnected()? 0 : 100; 

#define filter16_nozori_84  \
    CV_filter16_out[index_filter_pot1]= (uint32_t)(65535 * params[POT1_PARAM].getValue()); \
    CV_filter16_out[index_filter_pot2]= (uint32_t)(65535 * params[POT2_PARAM].getValue()); \
    CV_filter16_out[index_filter_pot3]= (uint32_t)(65535 * params[POT3_PARAM].getValue()); \
    CV_filter16_out[index_filter_pot4]= (uint32_t)(65535 * params[POT4_PARAM].getValue()); \
    CV_filter16_out[index_filter_pot5]= (uint32_t)(65535 * params[POT5_PARAM].getValue()); \
    CV_filter16_out[index_filter_pot6]= (uint32_t)(65535 * params[POT6_PARAM].getValue()); \
    CV_filter16_out[index_filter_pot7]= (uint32_t)(65535 * params[POT7_PARAM].getValue()); \
    CV_filter16_out[index_filter_pot8]= (uint32_t)(65535 * params[POT8_PARAM].getValue()); \
    IN1_connect = inputs[IN1_INPUT].isConnected()? 0 : 100; \
    IN2_connect = inputs[IN2_INPUT].isConnected()? 0 : 100; 

#define REG_CV1 CV_filter16_out[index_filter_CV1]

// Calibration value : this are const in VCV since there are no hardware variation (no ADC to calibrate)
const uint32_t CV1_0V = 1<<15;
const uint32_t CV2_0V = 1<<15;
const uint32_t CV3_0V = 1<<15;
const uint32_t CV4_0V = 1<<15;

const int32_t CV1_1Vminus0V = 6183;
const int32_t CV2_1Vminus0V = 6183;
const int32_t CV3_1Vminus0V = 6183;
const int32_t CV4_1Vminus0V = 6183;

const uint32_t CV1_1V = (48<<18) / CV1_1Vminus0V;
const uint32_t CV2_1V = (48<<18) / CV2_1Vminus0V;
const uint32_t CV3_1V = (48<<18) / CV3_1Vminus0V;
const uint32_t CV4_1V = (48<<18) / CV4_1Vminus0V;

const uint32_t IN1_0V = 2147483648; // valeur moyenne qd il n'y a pas de calibration valable
const int32_t IN1_1V_value = 322122547; // idem
const int32_t IN1_1V = (48<<18) / (IN1_1V_value>>16); //avec un CV sur 16 bit seulement :  pitch = ((audio_inL-(1<<31)>>16)*IN1_1V)
// IN1_1V = 2560

const uint32_t IN2_0V = 2147483648; // valeur moyenne qd il n'y a pas de calibration valable
const int32_t IN2_1V_value = 322122547; // idem
const int32_t IN2_1V = (48<<18) / (IN2_1V_value>>16); //avec un CV sur 16 bit seulement :  pitch = ((audio_inL-(1<<31)>>16)*IN1_1V)

// TODO
const int32_t OUT1_0V = 1<<31; // valeur moyenne qd il n'y a pas de calibration valable
const int32_t OUT1_1V = (1<<31) + (0xFFFFFF00/14) - OUT1_0V;// idem

const int32_t OUT2_0V = 1<<31; // valeur moyenne qd il n'y a pas de calibration valable
const int32_t OUT2_1V = (1<<31) + (0xFFFFFF00/14) - OUT2_0V; // idem


