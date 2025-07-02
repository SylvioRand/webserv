NAME = webserv

SRC = ./src/main.cpp \
			./src/utils/printError.cpp \

OBJS = ${SRC:.cpp=.o}

CC = c++
CFLAGS = -std=c++98 -Wall -Wextra -Werror
RM = rm -rf

.cpp.o:
	${CC} ${CFLAGS} -I./include -c $< -o ${<:.cpp=.o}

all: ${NAME}

${NAME}: ${OBJS}
	${CC} ${CFLAGS} -o ${NAME} ${OBJS}

clean:
	${RM} ${OBJS}

fclean: clean
	${RM} ${NAME}

re: fclean all

.PHONY: all clean fclean re

