# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: abronner <abronner@student.42berlin.de>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/12 15:21:49 by abronner          #+#    #+#              #
#    Updated: 2026/02/12 16:20:46 by abronner         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

include colors.mk

NAME		:= lex

# * DIRECTORIES ************************************************************** #

SRC_DIR		:= src
OBJ_DIR		:= obj
INC_DIR		:= inc

# * SOURCES & OBJECTS ******************************************************** #

SRCS		?=
vpath %.c $(SRC_DIR)
SRCS		+= automata.c
SRCS		+= state.c
SRCS		+= transition.c
SRCS		+= simple_test.c

OBJS		:= $(SRCS:%.c=$(OBJ_DIR)/%.o)
DEPS		:= $(OBJS:%.o=%.d)

# * COMMANDS ***************************************************************** #

CC			:= cc
LD			:= cc
RM			:= rm -rf
AR			:= ar
PRNT		:= printf

# * FLAGS ******************************************************************** #

CFLAGS		?=

ifndef NWERR
	CFLAGS		+=	-Wall -Wextra -Werror
endif

CPPFLAGS	:=
CPPFLAGS	+= -MMD -MP
CPPFLAGS	+= $(addprefix -I, $(INC_DIR))

LDFLAGS		:=

ifneq "$(or $(DEBUG), $(or $(UB), $(or $(ASAN), $(LSAN))))" ""
	CFLAGS += -O0
	CFLAGS += -g3
else
	CFLAGS += -O3
endif

ifeq ($(UB), 1)
	CFLAGS += -fsanitize=undefined
	LDFLAGS += -fsanitize=undefined
endif

ifeq ($(ASAN), 1)
	CFLAGS += -fsanitize=address
	LDFLAGS += -fsanitize=address
endif

ifeq ($(LSAN), 1)
	CFLAGS += -fsanitize=leak
	LDFLAGS += -fsanitize=leak
endif

# * RULES ******************************************************************** #

all: $(NAME)

$(NAME): $(OBJS)
	@$(PRNT) "Creating $(P_NAME).\n"
	@$(LD) $(LDFLAGS) $^ -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)
	@$(PRNT) "Creating $(P_ODIR) directory.\n"

P_CFLAGS	:=$(C_MAG)$(BLD)$(CFLAGS)$(RESET)
P_CPPFLAGS	:=$(C_MAG)$(BLD)$(CPPFLAGS)$(RESET)

$(OBJ_DIR)/%.o: %.c $(OBJ_DIR)
	@$(if $(P_CFLAGS),echo "CFLAGS: $(P_CFLAGS)" $(eval P_CFLAGS:=))
	@$(if $(P_CPPFLAGS),echo "CPPFLAGS: $(P_CPPFLAGS)" $(eval P_CFLAGS:=))
	@$(CC) $(CFLAGS) $(CPPFLAGS) $< -c -o $@
	@$(PRNT) "Compiling $(P_AT).\n"

-include $(DEPS)

re: fclean all

clean:
	@$(RM) $(OBJ_DIR)
	@$(PRNT) "Deleting object files for $(P_NAME).\n"

fclean: clean
	@$(RM) $(NAME)
	@$(PRNT) "Deleting $(P_NAME).\n"

.PHONY: all bonus re clean fclean make

# * PRINTING PRESETS ********************************************************************** #

P_NAME=$(C_RED)$(BLD)$(NAME)$(RESET)
P_ODIR=$(C_BLU)$(BLD)$(OBJ_DIR)$(RESET)
P_AT=$(C_GRN)$(BLD)$@$(RESET)
