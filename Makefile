SOURCES = mtServer.cpp mtClient.cpp
PLINK = g++ -ggdb -pthread
CXX = g++
CXX_FLAGS = -c

all: mtServer mtClient 

mtServer: mtServer.cpp
	$(PLINK) -o mtServer mtServer.cpp

mtClient: mtClient.cpp
	$(PLINK) -o mtClient mtClient.cpp

clean: 
	$(RM) *.o *.out mtServer mtClient Files.txt
	
clean_dirs:
	rm -rf Thread*files
