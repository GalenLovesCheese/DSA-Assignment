#ifndef ACTOR_MOVIE_H
#define ACTOR_MOVIE_H

#include "utils/csvparser.h"

struct ActorMovie
{
    int actor_id;
    int movie_id;
};

namespace CSVParser
{
    template <>
    struct Traits<ActorMovie>
    {
        static void apply(ActorMovie &obj, char **cols, int &idx)
        {
            obj.actor_id = detail::convertInt(cols[idx++]);
            obj.movie_id = detail::convertInt(cols[idx++]);
        }

        static void free(ActorMovie &obj)
        {
        }
    };
}

#endif // ACTOR_MOVIE_H