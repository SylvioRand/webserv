NAME = webserv

SRC = ./src/main.cpp \
			./src/core/Server.cpp \
			./src/core/Client.cpp \
			./src/core/Config.cpp \
			./src/core/HttpRequest.cpp \
			./src/core/HttpResponse.cpp \
			./src/core/ServerCGI.cpp \
			./src/utils/Logger.cpp \
			./src/utils/throwWithLog.cpp \
			./src/utils/caseInsensitiveEqual.cpp \
			./src/utils/getFileSize.cpp \
			./src/utils/toLower.cpp \
			./src/utils/toUpper.cpp \

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

