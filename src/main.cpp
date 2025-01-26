#include <iostream>
#include <string>

#include "dst/bplustree.h"
#include "dst/hashmap.h"

#include "classes/actor.h"
#include "classes/movie.h"
#include "classes/actor-movie.h"

#include "utils/debug.h"

// Function prototypes
void populate_main_hashmap(HashMap<int, Actor> *actor_map, HashMap<int, Movie> *movie_map, Actor *actors, Movie *movies, size_t actor_count, size_t movie_count);
void populate_index_trees(BPlusTree<std::string, int> *actor_name_index, BPlusTree<int, int> *actor_year_index, BPlusTree<int, int> *movie_year_index, Actor *actors, Movie *movies, size_t actor_count, size_t movie_count);
void populate_relation_hashmaps(HashMap<int, LinkedList<int>> *actor_movies, HashMap<int, LinkedList<int>> *movie_actors, ActorMovie *actor_movies_csv, size_t actor_movie_count);

void admin_handler(int input);

void user_handler(int input);

void display_actor_age(BPlusTree<int, int> *actor_year_index, HashMap<int, Actor> *actor_map);

int main()
{
    // Variables
    bool admin = false;

    // Load data from CSV files
    size_t actor_count, movie_count, actor_movie_count;
    Actor *actors = CSVParser::Parse<Actor>("data/actors.csv", &actor_count);
    Movie *movies = CSVParser::Parse<Movie>("data/movies.csv", &movie_count);
    ActorMovie *actor_movies_csv = CSVParser::Parse<ActorMovie>("data/cast.csv", &actor_movie_count);

    // Initialise main hashmap, index trees & relation hashmaps
    HashMap<int, Actor> *actor_map = new HashMap<int, Actor>(actor_count);
    HashMap<int, Movie> *movie_map = new HashMap<int, Movie>(movie_count);

    BPlusTree<std::string, int> *actor_name_index = new BPlusTree<std::string, int>();
    BPlusTree<int, int> *actor_year_index = new BPlusTree<int, int>();
    BPlusTree<int, int> *movie_year_index = new BPlusTree<int, int>();

    HashMap<int, LinkedList<int>> *actor_movies = new HashMap<int, LinkedList<int>>();
    HashMap<int, LinkedList<int>> *movie_actors = new HashMap<int, LinkedList<int>>();

    // Populate main hashmap, index trees & relation hashmaps
    populate_main_hashmap(actor_map, movie_map, actors, movies, actor_count, movie_count);
    populate_index_trees(actor_name_index, actor_year_index, movie_year_index, actors, movies, actor_count, movie_count);
    populate_relation_hashmaps(actor_movies, movie_actors, actor_movies_csv, actor_movie_count);

    // Main user interface loop
    int input = 0;
    do
    {
        std::cout << "========== Movie App by Bowen & Galen ==========" << std::endl;
        std::cout << "1. Display actors between a certain age range" << std::endl;
        std::cout << "2. Display movies released within the past 3 years" << std::endl;
        std::cout << "3. Display all movies an actor starred in" << std::endl;
        std::cout << "4. Display all actors in a movie" << std::endl;
        std::cout << "5. Display all actors that an actor knows" << std::endl;

        if (admin)
        {
            std::cout << "========== Admin Commands ==========" << std::endl;
            std::cout << "6. Add a new actor" << std::endl;
            std::cout << "7. Add a new movie" << std::endl;
            std::cout << "8. Add a new actor to a movie" << std::endl;
            std::cout << "9. Update actor details" << std::endl;
            std::cout << "10. Update movie details" << std::endl;
        }

        std::cout << "\nChoice (Type '0' to quit): ";
        std::cin >> input;

        if (input >= 6 && input <= 10 && admin)
        {
            admin_handler(input);
        }
        else if (input >= 1 && input <= 5)
        {
            user_handler(input);
        }
        else if (input == 0)
        {
            std::cout << "Exiting..." << std::endl;
        }
        else
        {
            std::cout << "Invalid input. Please try again." << std::endl;
        }

        std::cout << std::endl;
    } while (input != 0);

    // DO NOT REMOVE: Free up the memory
    CSVParser::FreeResults(actors, actor_count);
    CSVParser::FreeResults(movies, movie_count);
    return 0;
}

// ===============================
// Assignment Functions (Admin)
// ===============================

void admin_handler(int input)
{
    // TODO: Implement admin handler
}

// ===============================
// Assignment Functions (User)
// ===============================

void user_handler(int input)
{
    // TODO: Implement user handler
}

// ===============================
// Helper functions
// ===============================

void populate_main_hashmap(HashMap<int, Actor> *actor_map, HashMap<int, Movie> *movie_map, Actor *actors, Movie *movies, size_t actor_count, size_t movie_count)
{
    // Populate main hashmap
    for (size_t i = 0; i < actor_count; i++)
    {
        actor_map->insert(actors[i].id, actors[i]);
    }
    for (size_t i = 0; i < movie_count; i++)
    {
        movie_map->insert(movies[i].id, movies[i]);
    }
}

void populate_index_trees(BPlusTree<std::string, int> *actor_name_index, BPlusTree<int, int> *actor_year_index, BPlusTree<int, int> *movie_year_index, Actor *actors, Movie *movies, size_t actor_count, size_t movie_count)
{
    // Populate index trees
    for (size_t i = 0; i < actor_count; i++)
    {
        actor_name_index->insert(actors[i].name, actors[i].id);
        actor_year_index->insert(actors[i].year, actors[i].id);
    }
    for (size_t i = 0; i < movie_count; i++)
    {
        movie_year_index->insert(movies[i].year, movies[i].id);
    }
}

void populate_relation_hashmaps(HashMap<int, LinkedList<int>> *actor_movies, HashMap<int, LinkedList<int>> *movie_actors, ActorMovie *actor_movies_csv, size_t actor_movie_count)
{
    // Populate relation hashmaps
    for (size_t i = 0; i < actor_movie_count; i++)
    {
        // Handle actor_movies hashmap
        LinkedList<int> *actor_movies_list = actor_movies->get(actor_movies_csv[i].actor_id);
        if (actor_movies_list == nullptr)
        {
            LinkedList<int> new_list;
            new_list.push_back(actor_movies_csv[i].movie_id);
            actor_movies->insert(actor_movies_csv[i].actor_id, new_list);
        }
        else
        {
            actor_movies_list->push_back(actor_movies_csv[i].movie_id);
        }

        // Handle movie_actors hashmap
        LinkedList<int> *movie_actors_list = movie_actors->get(actor_movies_csv[i].movie_id);
        if (movie_actors_list == nullptr)
        {
            LinkedList<int> new_list;
            new_list.push_back(actor_movies_csv[i].actor_id);
            movie_actors->insert(actor_movies_csv[i].movie_id, new_list);
        }
        else
        {
            movie_actors_list->push_back(actor_movies_csv[i].actor_id);
        }
    }
}
