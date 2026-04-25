CPP = c++
CPP_FLAGS = -std=c++20 -DDEBUG

PROGRAM_NAME = scop

SRCDIR := ./srcs
INCDIR := ./includes

SRC := $(wildcard $(SRCDIR)/*.cpp) 

INCLUDES := -I$(INCDIR)

LIBS	 := -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm

OBJDIR := objs
OBJS := $(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

all: $(PROGRAM_NAME)

$(PROGRAM_NAME): $(OBJS)
	$(CPP) $(CPP_FLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
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
	rm -f $(PROGRAM_NAME)

re: fclean all

.PHONY: all clean fclean re