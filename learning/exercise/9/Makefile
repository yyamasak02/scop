CPP = c++
CPP_FLAGS = -std=c++20 -DDEBUG

# Programs to build
PROGRAMS := ex01 ex02 

# registre alias for each program
ex01: ex01.exe
ex02: ex02.exe

SRCDIR := ./srcs
INCDIR := ./includes

INCLUDES := -I$(INCDIR)

LIBS	 := -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm

OBJDIR := objs

# Find sources (maxdepth 2 to include immediate subdirs like ex01)
ALL_SRCS := $(shell find $(SRCDIR) -maxdepth 2 -name '*.cpp')

all: $(PROGRAMS)

ex01.exe: $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(wildcard $(SRCDIR)/ex01/*.cpp))
	$(CPP) $(CPP_FLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

ex02.exe: $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(wildcard $(SRCDIR)/ex02/*.cpp))
	$(CPP) $(CPP_FLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

# Generic rule: create object file under objs/... mirroring srcs/...
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	@mkdir -p $(dir $@)
	$(CPP) $(CPP_FLAGS) $(INCLUDES) -o $@ -c $<

install:
	@echo "Installing system dependencies..."
	sudo apt update
	sudo apt install -y \
		libglfw3-dev \
		libgl1-mesa-dev \
		libx11-dev \
		libxi-dev

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(PROGRAMS)

re: fclean all

.PHONY: all clean fclean re install $(PROGRAMS)