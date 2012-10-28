
# $File: Makefile
# $Date: Mon Aug 27 15:42:06 2012 +0800

OBJ_DIR = obj
TARGET = main

INCLUDE_DIR = -Iinclude 
#DEFINES =
ccFLAGS = $(INCLUDE_DIR) $(DEFINES) -fopenmp  -O3
LDFLAGS = -lpng -lX11 -lm  -fopenmp

cc = mpicxx
ccSOURCES = $(shell find -name "*.cc")
OBJS = $(addprefix $(OBJ_DIR)/,$(ccSOURCES:.cc=.o))
DEPFILES = $(OBJS:.o=.d)


.PHONY: all clean run 

all: $(TARGET)

$(OBJ_DIR)/%.o: %.cc
	@echo "[cc] $< ..."
	@$(cc) -c $< -o $@ $(ccFLAGS)

$(OBJ_DIR)/%.d: %.cc
	@mkdir -pv $(dir $@)
	@echo "[dep] $< ..."
	@$(cc) $(INCLUDE_DIR) $(DEFINES) -MM -MT "$(OBJ_DIR)/$(<:.cc=.o) $(OBJ_DIR)/$(<:.cc=.d)" "$<"  > "$@"

sinclude $(DEPFILES)

$(TARGET): $(OBJS)
	@echo "Linking ..."
	@$(cc) $(OBJS) -o $@ $(LDFLAGS)


cleanbuild:
	@rm -rf $(OBJ_DIR) $(TARGET) *.png 

clean:
	@rm -rf $(OBJ_DIR) $(TARGET) *.png mpi seq omp pthread

run: $(TARGET)
	./$(TARGET)

