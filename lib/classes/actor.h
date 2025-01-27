#ifndef ACTOR_H
#define ACTOR_H

#include "utils/csvparser.h"
#include "dst/linkedlist.h"

struct Actor {
    int id;
    char* name;
    int year;
    LinkedList<int>* movies;
};

namespace CSVParser {
    template<>
    struct Traits<Actor> {
        static void apply(Actor& obj, char** cols, int& idx) {
            obj.id = detail::convertInt(cols[idx++]);
            obj.name = detail::convertString(cols[idx++]);
            obj.year = detail::convertInt(cols[idx++]);
        }
        
        static void free(Actor& obj) {
            ::free(obj.name);
        }
    };
}

#endif // ACTOR_H