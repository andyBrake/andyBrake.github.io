OBJ_C += testServer.cpp
APP = server.out

${APP}:${OBJ_C}
	g++ -pthread -o ${APP} ${OBJ_C}

clean:
	rm -f *.o *.out