
TICK_COMBINER_TARGET=combine_ticks
TARGETS=$(TICK_COMBINER_TARGET)

all: clean $(TARGETS)

$(TICK_COMBINER_TARGET):
	g++ data/combine_ticks.cpp -o data/combine_ticks

clean:
	rm -rf data/*.o data/$(TICK_COMBINER_TARGET)