ifndef IPC
	IPC=fifo
endif

.SILENT:

OBJS_S = simulation.o fileManager.o graph.o medList.o free.o companyReader.o\
	 defines.h ipcs.h $(IPC).o marshalling.o semaphore.o map.h
OBJS_M = map.o marshalling.o $(IPC).o graph.o fileManager.o medList.o free.o semaphore.o
OBJS_C = company.o marshalling.o $(IPC).o graph.o fileManager.o medList.o free.o\
		companyReader.o semaphore.o
OBJS_MT = multitasker.o marshalling.o $(IPC).o graph.o fileManager.o medList.o free.o\
	companyReader.o	semaphore.o
OBJS_IO = io.o marshalling.o $(IPC).o graph.o fileManager.o medList.o free.o\
	companyReader.o	semaphore.o

TARGET = simulation
CC = gcc
LD = $(CC)
CCOPTS = -pthread -c -g -w
LDOPTS = -pthread -o

$(TARGET): $(OBJS_S)
	- $(LD) $(LDOPTS) $(TARGET) $(OBJS_S)
	- make map
	- make company
	- make multitasker
	- make io
	
map: $(OBJS_M)
	$(LD) $(LDOPTS) map $(OBJS_M)

		
company: $(OBJS_C)
	$(LD) $(LDOPTS) company $(OBJS_C)

multitasker: $(OBJS_MT)
	$(LD) $(LDOPTS) multitasker $(OBJS_MT)

io: $(OBJS_IO)
	$(LD) $(LDOPTS) io $(OBJS_IO)
	
fileManager.o: fileManager.h
graph.o: graph.h
medList.o: medList.h
companyReader.o: companyReader.h
free.o: free.h
marshalling.o: marshalling.h
semaphore.o: semaphore.h
map.o: map.h
io.o: io.h

.c.o:
	echo Compilando $<
	$(CC) $(CCOPTS) $<

clear:
	echo Borrando .o y ejecutables
	-rm *.o
	-rm $(TARGET)
	-rm map
	-rm company
	-rm multitasker
	-rm /tmp/fifo*
	-rm io
	-rm /tmp/socket*
	-rm aero*Log.txt
	-rm mapLog.txt
