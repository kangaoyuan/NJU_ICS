APP=gen-expr
INC_DIR += /home/gaoyuankan/ics2020/nemu/include
INCLUDES  = $(addprefix -I, $(INC_DIR))

$(APP): gen-expr.c
	gcc -O2 -Wall -Werror $(INCLUDES) -o $@ $<

.PHONY: clean
clean:
	-rm $(APP) 2> /dev/null
