#!/bin/bash

g++ main.cpp -c -o main.o

g++ ./header_repository/deserializer_header_repository.cpp -c -o deserializer_header_repository.o
g++ ./header_repository/header_repository_node.cpp -c -o header_repository_node.o
g++ ./header_repository/header_repository.cpp -c -o header_repository.o
g++ ./header_repository/serializer_header_repository.cpp -c -o serializer_header_repository.o

g++ main.o deserializer_header_repository.o header_repository_node.o header_repository.o serializer_header_repository.o -o program
