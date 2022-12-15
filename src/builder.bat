del *.o
gcc -c -DVERSION=\"1.0.0\" -DLIBROOT=\"c:\\mstoical\\lib\" -Wall -g -O2   -c -o kernel.o kernel.c
gcc -c -DVERSION=\"1.0.0\" -DLIBROOT=\"c:\\mstoical\\lib\" -Wall -g -O2   -c -o mem.o mem.c
gcc -c -DVERSION=\"1.0.0\" -DLIBROOT=\"c:\\mstoical\\lib\" -Wall -g -O2   -c -o hash.o hash.c
gcc -c -DVERSION=\"1.0.0\" -DLIBROOT=\"c:\\mstoical\\lib\" -Wall -g -O2   -c -o dict.o dict.c
gcc -c -DVERSION=\"1.0.0\" -DLIBROOT=\"c:\\mstoical\\lib\" -Wall -g -O2   -c -o debug.o debug.c
gcc -c -DVERSION=\"1.0.0\" -DLIBROOT=\"c:\\mstoical\\lib\" -Wall -g -O2   -c -o string.o string.c
rem gcc kernel.o mem.o hash.o dict.o debug.o string.o -lm -ltermcap -lreadline -lpthread  -o mstoical.exe
gcc kernel.o mem.o hash.o dict.o debug.o string.o -lm -o mstoical.exe