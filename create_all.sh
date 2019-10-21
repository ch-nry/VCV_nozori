rm -rf dep dist res src
rm Makefile plugin.json plugin.so

cp template_nozori/* -r .
rm src/nozori_68.cpp
rm src/nozori_84.cpp

#create module_name source_file.ino fuction_name img_file.svg 
./create_68.sh Nozori_68_ADSR       m68_ADSR.ino        ADSR_VCA        nozori_68_ADSR.svg
./create_68.sh Nozori_68_AR         m68_AR.ino          AR              nozori_68_AR.svg
./create_68.sh Nozori_68_CRUSH      m68_CRUSH.ino       BIT_Crush       nozori_68_CRUSH.svg
./create_68.sh Nozori_68_CV_REC     m68_CV_REC.ino      CV_REC          nozori_68_CV_REC.svg
./create_68.sh Nozori_68_DELAY      m68_Delay.ino       Delay           nozori_68_DELAY.svg
./create_68.sh Nozori_68_EQ         m68_EQ.ino          EQ              nozori_68_EQ.svg
./create_68.sh Nozori_68_GRAN       m68_Gran.ino        Gran            nozori_68_GRANULATOR.svg
./create_68.sh Nozori_68_KS         m68_KS.ino          KS              nozori_68_KS.svg
./create_68.sh Nozori_68_LFO        m68_LFO.ino         LFO_Dual        nozori_68_LFO.svg
./create_68.sh Nozori_68_LFO_MOD    m68_LFO_MOD.ino     LFO_MOD         nozori_68_LFO_MOD.svg
./create_68.sh Nozori_68_NOISE      m68_Noise.ino       noise           nozori_68_NOISE.svg
./create_68.sh Nozori_68_PITCH      m68_PITCH.ino       PShift          nozori_68_PITCH.svg
./create_68.sh Nozori_68_RND_LOOP   m68_RND_LOOP.ino    RND_LOOP        nozori_68_RND_LOOP.svg
./create_68.sh Nozori_68_SIN_MOD    m68_SIN_MOD.ino     VCO_SIN_MOD     nozori_68_SIN_MOD.svg
./create_68.sh Nozori_68_VCA_PAN    m68_VCA_PAN.ino     Pan             nozori_68_VCA_PAN.svg
./create_68.sh Nozori_68_VCF_MOD    m68_VCF.ino         VCF             nozori_68_VCF_MOD.svg
./create_68.sh Nozori_68_VCF_MORPH  m68_VCF_MORPH.ino   VCF_Param       nozori_68_VCF_MORPH.svg
./create_68.sh Nozori_68_VCO        m68_VCO.ino         VCO             nozori_68_VCO.svg
./create_68.sh Nozori_68_VCO_LOOP   m68_VCO_LOOP.ino    VCO_LOOP        nozori_68_VCO_LOOP.svg
./create_68.sh Nozori_68_VCO_MORPH  m68_VCO_MORPH.ino   VCO_Param       nozori_68_VCO_MORPH.svg
./create_68.sh Nozori_68_VCO_QTZ    m68_VCO_Q.ino       VCO_Q           nozori_68_VCO_Qtz.svg
./create_68.sh Nozori_68_VCO_WS     m68_VCO-WS.ino      VCO_WS          nozori_68_VCO_WS.svg
./create_68.sh Nozori_68_WS         m68_WS.ino          WS              nozori_68_WS.svg


./create_84.sh Nozori_84_8_BITS      m84_8_bit.ino       VCO_8bit        nozori_84_8BITS.svg
./create_84.sh Nozori_84_ADD         m84_ADD.ino         VCO_Add         nozori_84_ADD.svg
./create_84.sh Nozori_84_ADSR        m84_ADSR.ino        ADSR_Loop       nozori_84_ADSR.svg
./create_84.sh Nozori_84_CLOCK       m84_CLOCK.ino       CLK_ADSR        nozori_84_CLOCK.svg
./create_84.sh Nozori_84_DOPPLER     m84_Doppler.ino     Doppler         nozori_84_DOPPLER.svg
./create_84.sh Nozori_84_FM          m84_FM.ino          FM              nozori_84_FM.svg
./create_84.sh Nozori_84_FM_LIN      m84_FM_lin.ino      sin_FM_lin      nozori_84_FM_LIN.svg
./create_84.sh Nozori_84_HARMONICS   m84_HARMONICS.ino   VCO_Harmo       nozori_84_HARMONICS.svg
./create_84.sh Nozori_84_JONG_LFO    m84_JONG.ino        DEJONG_LFO      nozori_84_JONG_LFO.svg
./create_84.sh Nozori_84_JONG_VCO    m84_JONG.ino        DEJONG_AUDIO    nozori_84_JONG_VCO.svg
./create_84.sh Nozori_84_LFO         m84_LFO.ino         LFO_Mod         nozori_84_LFO.svg
./create_84.sh Nozori_84_LFO_SEQ     m84_LFO_SEQ.ino     LFO_SEQ         nozori_84_LFO_SEQ.svg
./create_84.sh Nozori_84_MODULATE    m84_Modulate.ino    modulation      nozori_84_MODULATE.svg
./create_84.sh Nozori_84_PARAM       m84_PARAMTRIC.ino   OSC_Param       nozori_84_PARAM.svg
./create_84.sh Nozori_84_SEQ_4       m84_SEQ4.ino        SEQ4            nozori_84_SEQ4.svg
./create_84.sh Nozori_84_SEQ_8       m84_SEQ8.ino        SEQ8            nozori_84_SEQ8.svg
./create_84.sh Nozori_84_SIN_AM      m84_Sin_AM.ino      sin_AM          nozori_84_SIN_AM.svg
./create_84.sh Nozori_84_SIN_FM      m84_Sin_FM.ino      sin_FM          nozori_84_SIN_FM.svg
./create_84.sh Nozori_84_SIN_PM      m84_Sin_PM.ino      sin_PM          nozori_84_SIN_PM.svg
./create_84.sh Nozori_84_SIN_WS      m84_Sin_WS.ino      SIN_WS          nozori_84_SIN_WS.svg
./create_84.sh Nozori_84_THOMAS      m84_Thomas.ino      THOMAS          nozori_84_THOMAS.svg
./create_84.sh Nozori_84_VCF         m84_VCF_MORPH.ino   VCF_Param84     nozori_84_VCF.svg
./create_84.sh Nozori_84_WS          m84_Stereo_WS.ino   Stereo_WS       nozori_84_WS.svg


# remove last comma
sed -i '$ d' plugin.json 
sed -i '$ d' plugin.json 
sed -i '$ d' plugin.json 
echo "  ]" >> plugin.json 
echo "}" >> plugin.json 

RACK_DIR=.. make install

