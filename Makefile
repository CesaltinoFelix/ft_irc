NAME        = ircserv
CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98 -I./inc

SRC_DIR     = src
CMD_DIR     = cmd
INC_DIR     = inc
OBJ_DIR     = obj

SRCS        = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(CMD_DIR)/*.cpp)
OBJS        = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

RM          = rm -rf
MKDIR       = mkdir -p

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
$(OBJ_DIR)/%.o: %.cpp
	@$(MKDIR) $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean rere
