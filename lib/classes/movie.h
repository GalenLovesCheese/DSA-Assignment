#ifndef MOVIE_H
#define MOVIE_H

#include "utils/csvparser.h"
#include "dst/linkedlist.h"

struct Movie {
    int id;
    char* title;
    char* plot;
    int year;
    LinkedList<int>* actors;
};

namespace CSVParser {
    template<>
    struct Traits<Movie> {
        static void apply(Movie& obj, char** cols, int& idx) {
            obj.id = detail::convertInt(cols[idx++]);
            obj.title = detail::convertString(cols[idx++]);
            obj.plot = detail::convertString(cols[idx++]);
            obj.year = detail::convertInt(cols[idx++]);
        }
        
        static void free(Movie& obj) {
            ::free(obj.title);
        }
    };
}

#endif // MOVIE_H