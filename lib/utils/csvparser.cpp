#include "utils/csvparser.h"

#include "classes/actor.h"
#include "classes/movie.h"

#include <cstring>

namespace CSVParser {
    namespace detail {
        char* strdup(const char* s) {
            char* d = static_cast<char*>(malloc(strlen(s) + 1));
            if(d) strcpy(d, s);
            return d;
        }

        char** parseLine(const char* line, int& colCount) {
            char** cols = nullptr;
            colCount = 0;
            char* buffer = static_cast<char*>(malloc(strlen(line) + 1));
            size_t bufIndex = 0;
            bool inQuotes = false;

            for(const char* p = line; *p; p++) {
                if(*p == '"') {
                    inQuotes = !inQuotes;
                }
                else if(*p == ',' && !inQuotes) {
                    buffer[bufIndex] = '\0';
                    cols = static_cast<char**>(realloc(cols, (colCount + 1) * sizeof(char*)));
                    cols[colCount++] = strdup(buffer);
                    bufIndex = 0;
                }
                else {
                    buffer[bufIndex++] = *p;
                }
            }

            // Add last column
            buffer[bufIndex] = '\0';
            cols = static_cast<char**>(realloc(cols, (colCount + 1) * sizeof(char*)));
            cols[colCount++] = strdup(buffer);
            
            free(buffer);
            return cols;
        }

        void freeColumns(char** cols, int count) {
            for(int i = 0; i < count; i++) free(cols[i]);
            free(cols);
        }

        int convertInt(const char* s) { return atoi(s); }
        double convertDouble(const char* s) { return atof(s); }
        char* convertString(const char* s) { return strdup(s); }
    }
}

// Template implementations
template<typename T>
T* CSVParser::Parse(const char* filename, size_t* outCount) {
    using namespace CSVParser::detail;
    
    FILE* file = fopen(filename, "r");
    if(!file) throw "File open failed";

    T* results = nullptr;
    size_t count = 0;
    char line[4096];

    // Skip header line
    if(!fgets(line, sizeof(line), file)) {
        fclose(file);
        throw "File contains no data after header";
    }

    while(fgets(line, sizeof(line), file)) {
        // Remove newline characters
        line[strcspn(line, "\r\n")] = 0;

        int colCount = 0;
        char** cols = parseLine(line, colCount);
        T obj;
        int currentCol = 0;

        Traits<T>::apply(obj, cols, currentCol);

        // Add to results array
        results = static_cast<T*>(realloc(results, (count + 1) * sizeof(T)));
        results[count++] = obj;

        freeColumns(cols, colCount);
    }

    fclose(file);
    *outCount = count;
    return results;
}

template<typename T>
void CSVParser::FreeResults(T* data, size_t count) {
    for(size_t i = 0; i < count; i++) {
        Traits<T>::free(data[i]);
    }
    free(data);
}

template Actor* CSVParser::Parse<Actor>(const char*, size_t*);
template void CSVParser::FreeResults<Actor>(Actor*, size_t);

template Movie* CSVParser::Parse<Movie>(const char*, size_t*);
template void CSVParser::FreeResults<Movie>(Movie*, size_t);
