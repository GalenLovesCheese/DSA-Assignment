#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <cstdio>
#include <cstdlib>

namespace CSVParser {
    // Main template interface
    template<typename T>
    struct Traits;
    
    // Primary parse function
    template<typename T>
    T* Parse(const char* filename, size_t* outCount);
    
    // Memory cleanup function
    template<typename T>
    void FreeResults(T* data, size_t count);
    
    // Implementation namespace
    namespace detail {
        // String duplication
        char* strdup(const char* s);
        
        // Line parsing
        char** parseLine(const char* line, int& colCount);
        void freeColumns(char** cols, int count);
        
        // Type conversions
        int convertInt(const char* s);
        double convertDouble(const char* s);
        char* convertString(const char* s);
    }
}

struct Actor;
struct Movie;
struct ActorMovie;

extern template Actor* CSVParser::Parse<Actor>(const char*, size_t*);
extern template void CSVParser::FreeResults<Actor>(Actor*, size_t);

extern template Movie* CSVParser::Parse<Movie>(const char*, size_t*);
extern template void CSVParser::FreeResults<Movie>(Movie*, size_t);

extern template ActorMovie* CSVParser::Parse<ActorMovie>(const char*, size_t*);
extern template void CSVParser::FreeResults<ActorMovie>(ActorMovie*, size_t);

#endif // CSV_PARSER_H