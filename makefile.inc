# Makefile

CC		:= gcc
CFLAGS		:= -g -Wall -O2 -pthread -I$(HOME)/.local/include -I. -Werror
#CFLAGS		+= -I$(PWD)/lib/atomic_hash/src
#CFLAGS		+= -std=c11
# UDS == Unix Domain Socket (super fast)
#CFLAGS		+= -DUSE_YAML
CFLAGS		+= -DUSE_RAND -DPRNGBUFSZ=64
#CFLAGS		+= -DUSE_CORO='"libaco"' -I$(PWD)/lib
#CFLAGS		+= -DUSE_UNIX_SOCKET
#CFLAGS		+= -DUSE_LINUX_AIO
#CFLAGS		+= -DUSE_REDIS -DREDIS_SOCK='"/vh/p/sock.d/redis.sock"'
#CFLAGS		+= -DUSE_HS $(shell pkg-config --cflags libhs)

# Produce Captcha
CFLAGS		+= $(shell pkg-config --cflags hat-trie-0.1)
#CFLAGS		+= $(shell pkg-config --cflags hiredis)
#CFLAGS		+= $(shell pkg-config --cflags yaml-0.1)
#CFLAGS		+= $(shell pkg-config --cflags lua)
#CFLAGS		+= $(shell pkg-config --cflags libpcre)
#CFLAGS		+= -Iext-lib/ini_parser
#TARGETS		:= config
#LDFLAGS		:= -ldl $(shell pkg-config --libs cmph)
LDFLAGS		:= -ldl
#LDFLAGS		+= -I$(PWD)/lib/atomic_hash/src -latomic_hash
#LDFLAGS		+= $(shell pkg-config --libs uuid)
#LDFLAGS		+= $(shell pkg-config --libs libpcre)
LDFLAGS		+= $(shell pkg-config --libs hat-trie-0.1) -lhat-trie
#LDFLAGS		+= $(shell pkg-config --libs hiredis)
#LDFLAGS		+= $(shell pkg-config --libs libhs)
#LDFLAGS		+= $(shell pkg-config --libs yaml-0.1)
LDFLAGS		+= -lhttp_parser
#LDFLAGS		+= $(shell pkg-config --libs lua)
#LDFLAGS		:=
RAGEL		:= ragel
