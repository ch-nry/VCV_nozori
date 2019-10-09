rm -rf dep dist res src
rm Makefile plugin.json plugin.so

cp template_nozori_68/* -r .
rm src/nozori_68.cpp

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

# remove last comma
sed -i '$ d' plugin.json 
sed -i '$ d' plugin.json 
sed -i '$ d' plugin.json 
echo "  ]" >> plugin.json 
echo "}" >> plugin.json 

make install

