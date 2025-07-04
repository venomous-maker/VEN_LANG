cmake_minimum_required(VERSION 3.8)
project(venom)

set(CMAKE_CXX_STANDARD 11)
add_compile_options(-g3)

set(CMAKE_C_COMPILER "gcc-10")
set(CMAKE_CXX_COMPILER "g++-10")

add_executable(venom
        src/main.cpp
        src/inclussion.h
        src/lexer/lexer.cpp
        src/lexer/lexer.h
        src/lexer/token.cpp
        src/lexer/token.h
        src/parser/parser.cpp
        src/parser/parser.h
        src/parser/ast.cpp
        src/parser/ast.h
        src/visitor/visitor.h
        src/visitor/xml_visitor.cpp
        src/visitor/xml_visitor.h
        src/visitor/semantic_analysis.cpp
        src/visitor/semantic_analysis.h
        src/visitor/interpreter.cpp
        src/visitor/interpreter.h
        src/table/table.h
        src/structures/array.cpp
        src/structures/array.h
)

#set_target_properties(venom PROPERTIES COMPILE_FLAGS "-save-temps -fverbose-asm -O0 -Wa,-adhlmn=main.lst")
