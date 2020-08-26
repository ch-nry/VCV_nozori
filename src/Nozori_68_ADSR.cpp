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

#include "plugin.hpp"

struct Nozori_68_ADSR : Module {
	enum ParamIds {
		POT2_PARAM,
		POT1_PARAM,
		POT3_PARAM,
		POT4_PARAM,
		POT5_PARAM,
		POT6_PARAM,
		SWITCH_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CV4_INPUT,
		CV3_INPUT,
		CV1_INPUT,
		CV2_INPUT,
		IN1_INPUT,
		IN2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT2_OUTPUT,
		OUT1_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		LED4_LIGHT,
		LED2_LIGHT,
        TEXT_LIGHT_48,
        TEXT_LIGHT_96,
		NUM_LIGHTS
	};

	Nozori_68_ADSR() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(POT2_PARAM, 0.f, 1.f, 0.f, "pot2");
		configParam(POT1_PARAM, 0.f, 1.f, 0.f, "pot1");
		configParam(POT3_PARAM, 0.f, 1.f, 0.f, "pot3");
		configParam(POT4_PARAM, 0.f, 1.f, 0.f, "pot4");
		configParam(POT5_PARAM, 0.f, 1.f, 0.f, "pot5");
		configParam(POT6_PARAM, 0.f, 1.f, 0.f, "pot6");
		configParam(SWITCH_PARAM, 0.0, 2.0, 2.0, "switch");
	}

    #include "vcv2nozori.hpp"
    #include "a_tables.h"
    #include "c_macro.h"
    #include "c_variables.h"
    #include "c_fonctions.h"
    #include "a_utils.h"
    #include "m68_ADSR.ino"
    int reduce_data_speed_index;

    void onAdd() override {
        SR_needed = 96000.;
        init_random();
        ADSR_VCA_init_();
        lights[TEXT_LIGHT_48].setBrightness(1.f); 
        lights[TEXT_LIGHT_96].setBrightness(1.f); 
    }

    void onReset() override {
        onAdd();
    }

/*
    void onSampleRateChange() override {
        time_since_startup = 0.;
    }
*/
	void process(const ProcessArgs& args) override {
//        time_since_startup += 1/args.sampleRate;
        audio_inL = (uint32_t)((clamp(inputs[IN1_INPUT].getVoltage(), -6.24, 6.24)*322122547.2) + 2147483648.);
        audio_inR = (uint32_t)((clamp(inputs[IN2_INPUT].getVoltage(), -6.24, 6.24)*322122547.2) + 2147483648.);
        reduce_data_speed_index = (reduce_data_speed_index+1)%4;
        if (reduce_data_speed_index == 0) {
            ADSR_VCA_loop_(); // process data loop only at 1/4 the sampling rate in order to be more accurate with the hardware timing
            if (args.sampleRate != SR_needed) {
                if (SR_needed == 96000.) { 
                    lights[TEXT_LIGHT_96].setBrightness(0.f);
                } else if (SR_needed == 48000.) { 
                    lights[TEXT_LIGHT_48].setBrightness(0.f);
                } 
            } else {
                lights[TEXT_LIGHT_96].setBrightness(1.f);
                lights[TEXT_LIGHT_48].setBrightness(1.f);
            }
        }
        ADSR_VCA_audio_();
        outputs[OUT1_OUTPUT].setVoltage( ((float)audio_outL - 2147483648.)/322122547.2 );
        outputs[OUT2_OUTPUT].setVoltage( ((float)audio_outR - 2147483648.)/322122547.2 ); 
    }
};


template <typename BASE>
struct warningText_48 : BASE {
	warningText_48() {
		this->box.size = mm2px(Vec(61., 128.5));
        this->color.a = 0;
	}
	void drawLight(const widget::Widget::DrawArgs& args) override {
		nvgBeginPath(args.vg);
		if (this->color.a == 0.) { // Off actually means on, because all lights are on in the module selection menu
		    nvgRect(args.vg,0,  mm2px(123), this->box.size.x, mm2px(5.5));
		    nvgFillColor(args.vg, nvgRGBA(0x00,0x00,0x00,0x80));
			nvgFill(args.vg);
		    nvgFillColor(args.vg, nvgRGB(0xff,0xff,0xff));
    		nvgFontSize(args.vg, 10);
			nvgText(args.vg, mm2px(0.5), mm2px(127), "This module need a 48KHz Sampling Rate", NULL);
		}
	}
};

template <typename BASE>
struct warningText_96 : BASE {
	warningText_96() {
		this->box.size = mm2px(Vec(61., 128.5));
        this->color.a = 0;
	}
	void drawLight(const widget::Widget::DrawArgs& args) override {
		nvgBeginPath(args.vg);
		if (this->color.a == 0.) { // Off actually means on, because all lights are on in the module selection menu
		    nvgRect(args.vg,0,  mm2px(123), this->box.size.x, mm2px(5.5));
		    nvgFillColor(args.vg, nvgRGBA(0x00,0x00,0x00,0x80));
			nvgFill(args.vg);
		    nvgFillColor(args.vg, nvgRGB(0xff,0xff,0xff));
    		nvgFontSize(args.vg, 10);
			nvgText(args.vg, mm2px(0.), mm2px(127), "This module need a 96KHz Sampling Rate", NULL);
		}
	}
};

struct NozoriKnob : RoundKnob {
	NozoriKnob() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/NozoriKnob.svg")));
	}
};

struct NoLight : GrayModuleLightWidget {
 	NoLight() {
 		addBaseColor(SCHEME_BLACK);
 	}
};

struct Nozori_68_ADSRWidget : ModuleWidget {
	Nozori_68_ADSRWidget(Nozori_68_ADSR* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/nozori_68_ADSR.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<NozoriKnob>(mm2px(Vec(46.1, 30.3)), module, Nozori_68_ADSR::POT2_PARAM));
		addParam(createParamCentered<NozoriKnob>(mm2px(Vec(15.1, 30.3)), module, Nozori_68_ADSR::POT1_PARAM));
		addParam(createParamCentered<NozoriKnob>(mm2px(Vec(15.1, 52.8)), module, Nozori_68_ADSR::POT3_PARAM));
		addParam(createParamCentered<NozoriKnob>(mm2px(Vec(46.1, 52.8)), module, Nozori_68_ADSR::POT4_PARAM));
		addParam(createParamCentered<NozoriKnob>(mm2px(Vec(15.1, 75.1)), module, Nozori_68_ADSR::POT5_PARAM));
		addParam(createParamCentered<NozoriKnob>(mm2px(Vec(46.1, 75.1)), module, Nozori_68_ADSR::POT6_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(49.3, 98.0)), module, Nozori_68_ADSR::CV4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(36.8, 98.0)), module, Nozori_68_ADSR::CV3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11.9, 98.0)), module, Nozori_68_ADSR::CV1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.4, 98.0)), module, Nozori_68_ADSR::CV2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11.9, 113.3)), module, Nozori_68_ADSR::IN1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.4, 113.3)), module, Nozori_68_ADSR::IN2_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(49.3, 113.3)), module, Nozori_68_ADSR::OUT2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(36.8, 113.3)), module, Nozori_68_ADSR::OUT1_OUTPUT));

		addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(58.3, 41.5)), module, Nozori_68_ADSR::LED4_LIGHT));
		addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(2.9, 41.5)), module, Nozori_68_ADSR::LED2_LIGHT));

		addParam(createParamCentered<NKK>(mm2px(Vec(30.6, 14.3)), module, Nozori_68_ADSR::SWITCH_PARAM));
		addChild(createLightCentered<warningText_48<NoLight>>(mm2px(Vec(30.5, 64.3)), module, Nozori_68_ADSR::TEXT_LIGHT_48));
		addChild(createLightCentered<warningText_96<NoLight>>(mm2px(Vec(30.5, 64.3)), module, Nozori_68_ADSR::TEXT_LIGHT_96));
	}
};

Model* modelNozori_68_ADSR = createModel<Nozori_68_ADSR, Nozori_68_ADSRWidget>("Nozori_68_ADSR");
