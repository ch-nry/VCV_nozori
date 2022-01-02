cp template_nozori/src/nozori_84.cpp src/$1.cpp

sed -i 's/Nozori_all/'$1'/g' src/$1.cpp
sed -i 's/module_src_file.ino/'$2'/' src/$1.cpp
sed -i 's/module_function/'$3'/' src/$1.cpp
sed -i 's/module_face/'$4'/' src/$1.cpp

sed -i 's/}//' src/plugin.cpp
echo "  p->addModel(modelNozori_all);" >> src/plugin.cpp
echo } >> src/plugin.cpp
sed -i 's/Nozori_all/'$1'/' src/plugin.cpp

echo "extern Model* modelNozori_all;" >> src/plugin.hpp
sed -i 's/Nozori_all/'$1'/' src/plugin.hpp


# suprime les 2 dernieres lignes du fichier : } et ]
sed -i '$ d' plugin.json 
sed -i '$ d' plugin.json 
#ajout le module slug
echo "    {" >> plugin.json 
echo "      \"slug\": \"Nozori_all\"," >> plugin.json 
echo "      \"name\": \"Nozori_all\"," >> plugin.json 
echo "      \"description\": \"\"," >> plugin.json 
echo "      \"tags\": []" >> plugin.json 
echo "    }" >> plugin.json 
echo "    ," >> plugin.json 

#ajout les 2 lignes suprimés precedement
echo "  ]" >> plugin.json 
echo "}" >> plugin.json 
sed -i 's/Nozori_all/'$1'/' plugin.json




