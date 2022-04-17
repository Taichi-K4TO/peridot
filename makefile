CC       = g++
CFLAGS   = -O2
#CFLAGS   = -Wall
CVINC    = `pkg-config --cflags opencv`
CVLIB    = `pkg-config --libs opencv`
#PATHS    = -I/usr/local/include -L/usr/local/lib -I/usr/local/include/opencv

clean :
	rm -f peri
all :
	make ${EXECS}
peri : src/peri.cpp src/peri_pars.cpp src/peri_tkn.cpp src/peri_tbl.cpp src/peri_code.cpp src/peri_misc.cpp
	${CC} ${CFLAGS} src/peri.cpp src/peri_pars.cpp src/peri_tkn.cpp src/peri_tbl.cpp src/peri_code.cpp src/peri_misc.cpp ${CVINC} ${CVLIB} -o peri