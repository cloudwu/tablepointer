tablepointer.dll : tablepointer.c
	gcc -Wall --shared -o $@ $^ -I/d/project/lua/src -L/usr/local/bin -llua53

clean:
	rm *.dll
