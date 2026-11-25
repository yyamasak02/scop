CPP = c++
CPP_FLAGS = -std=c++20 -Wall -Wextra -Werror -MMD -MP -DDEBUG

# Programs to build
PROGRAMS := scop

SRCDIR := ./srcs
INCDIR := ./includes

OS := $(shell uname)

ifeq ($(OS), Darwin)
    INCLUDES := -I$(INCDIR) $(shell pkg-config --cflags glfw3)
    LIBS     := $(shell pkg-config --libs glfw3) -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
else
    INCLUDES := -I$(INCDIR)
    LIBS     := $(shell pkg-config --libs --static glfw3)
endif

OBJDIR := objs

# Find sources (maxdepth 2 to include immediate subdirs like ex01)
ALL_SRCS := $(shell find $(SRCDIR) -maxdepth 2 -name '*.cpp')

all: $(PROGRAMS)

# Root program (scop) uses .cpp files directly under srcs/
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(wildcard $(SRCDIR)/*.cpp))

# 実ファイル scop を成果物として扱う(.PHONY にしない)ので、
# オブジェクトが更新されたときだけ再リンクされる
scop: $(OBJS)
	$(CPP) $(CPP_FLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

# -MMD が出力するヘッダ依存(.d)を読み込み、.hpp の変更でも再ビルドされるようにする
-include $(OBJS:.o=.d)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

# Generic rule: create object file under objs/... mirroring srcs/...
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	@mkdir -p $(dir $@)
	$(CPP) $(CPP_FLAGS) $(INCLUDES) -o $@ -c $<

# root(コンテナ等)なら sudo は不要
SUDO := $(if $(filter 0,$(shell id -u)),,sudo)

install:
	@echo "Installing system dependencies..."
	$(SUDO) apt update
	$(SUDO) apt install -y \
		libglfw3-dev \
		libgl1-mesa-dev \
		libx11-dev \
		libxi-dev

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(PROGRAMS)

re: fclean all

.PHONY: all clean fclean re install