NAME = webserv
CONFIG_PATH = "./config/default.conf"

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
			./src/utils/uniqueFilename.cpp \
			./src/utils/utils.cpp \

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

run: ${NAME}
	./${NAME} ${CONFIG_PATH}

re_run: re run

.PHONY: all clean fclean re run re_run

