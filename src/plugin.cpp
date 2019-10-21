#include "plugin.hpp"

Plugin* pluginInstance;

void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	// p->addModel(modelMyModule);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.

  p->addModel(modelNozori_68_ADSR);

  p->addModel(modelNozori_68_AR);

  p->addModel(modelNozori_68_CRUSH);

  p->addModel(modelNozori_68_CV_REC);

  p->addModel(modelNozori_68_DELAY);

  p->addModel(modelNozori_68_EQ);

  p->addModel(modelNozori_68_GRAN);

  p->addModel(modelNozori_68_KS);

  p->addModel(modelNozori_68_LFO);

  p->addModel(modelNozori_68_LFO_MOD);

  p->addModel(modelNozori_68_NOISE);

  p->addModel(modelNozori_68_PITCH);

  p->addModel(modelNozori_68_RND_LOOP);

  p->addModel(modelNozori_68_SIN_MOD);

  p->addModel(modelNozori_68_VCA_PAN);

  p->addModel(modelNozori_68_VCF_MOD);

  p->addModel(modelNozori_68_VCF_MORPH);

  p->addModel(modelNozori_68_VCO);

  p->addModel(modelNozori_68_VCO_LOOP);

  p->addModel(modelNozori_68_VCO_MORPH);

  p->addModel(modelNozori_68_VCO_QTZ);

  p->addModel(modelNozori_68_VCO_WS);

  p->addModel(modelNozori_68_WS);

  p->addModel(modelNozori_84_8_BITS);

  p->addModel(modelNozori_84_ADD);

  p->addModel(modelNozori_84_ADSR);

  p->addModel(modelNozori_84_CLOCK);

  p->addModel(modelNozori_84_DOPPLER);

  p->addModel(modelNozori_84_FM);

  p->addModel(modelNozori_84_FM_LIN);

  p->addModel(modelNozori_84_HARMONICS);

  p->addModel(modelNozori_84_JONG_LFO);

  p->addModel(modelNozori_84_JONG_VCO);

  p->addModel(modelNozori_84_LFO);

  p->addModel(modelNozori_84_LFO_SEQ);

  p->addModel(modelNozori_84_MODULATE);

  p->addModel(modelNozori_84_PARAM);

  p->addModel(modelNozori_84_SEQ_4);

  p->addModel(modelNozori_84_SEQ_8);

  p->addModel(modelNozori_84_SIN_AM);

  p->addModel(modelNozori_84_SIN_FM);

  p->addModel(modelNozori_84_SIN_PM);

  p->addModel(modelNozori_84_SIN_WS);

  p->addModel(modelNozori_84_THOMAS);

  p->addModel(modelNozori_84_VCF);

  p->addModel(modelNozori_84_WS);
}
