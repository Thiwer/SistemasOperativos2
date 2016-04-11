rm Disco
make clean
make
echo "Enter para seguir"
read var
./mi_mkfs Disco 100000
echo "Enter para seguir"
read var
./escribir Disco
echo "Enter para seguir"
read var
./leer Disco 1