git clone https://github.com/sisoputnfrba/ansisop-parser
git clone https://github.com/sisoputnfrba/so-commons-library
cp -ar ansisop-parser/programas-ejemplo/evaluacion-final-esther/Scripts-Prueba/ /home/utnso/scripts
cd so-commons-library
sudo make install
cd ..
cd ansisop-parser/parser
sudo make install
cd ..
cd ..
cd funciones/Instalador
sudo make install
cd ..
cd ..
make all