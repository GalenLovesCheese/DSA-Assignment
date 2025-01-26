#include <iostream>
#include <string>

#include "dst/bplustree.h"
#include "dst/hashmap.h"

#include "classes/actor.h"
#include "classes/movie.h"

#include "utils/debug.h"

int main() {
    // Load data from CSV files
    size_t actor_count, movie_count;
    Actor* actors = CSVParser::Parse<Actor>("data/actors.csv", &actor_count);
    Movie* movies = CSVParser::Parse<Movie>("data/movies.csv", &movie_count);

    // Initialise main hashmap, index trees & relation hashmaps
    HashMap<int, Actor> actor_map(actor_count);
    HashMap<int, Movie> movie_map(movie_count);

    BPlusTree<std::string, int> actor_name_index();
    BPlusTree<int, int> actor_year_index();
    BPlusTree<int, int> movie_year_index();

    HashMap<int, LinkedList<int>> actor_movies();
    HashMap<int, LinkedList<int>> movie_actors();

    // DO NOT REMOVE: Free up the memory
    CSVParser::FreeResults(actors, actor_count);
    CSVParser::FreeResults(movies, movie_count);
    return 0;
}