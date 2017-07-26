

#!/bin/bash
cd ..
git clone https://github.com/sisoputnfrba/so-commons-library.git
cd so-commons-library
sudo make install
cd ..
git clone https://github.com/sisoputnfrba/ansisop-parser.git
cd ansisop-parser
sudo make install
cd ..
cd tp-2017-1c-No-Se-Recursa/funciones/Instalador
sudo make install
cd ..
cd ..
sudo make all