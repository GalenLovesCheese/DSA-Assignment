#include <iostream>
#include <string>
#include <ctime>

#include "dst/bplustree.h"
#include "dst/hashmap.h"

#include "classes/actor.h"
#include "classes/movie.h"
#include "classes/actor-movie.h"

#include "utils/debug.h"

// Global variables
size_t actor_count, movie_count, actor_movie_count;
Actor *actors;
Movie *movies;
ActorMovie *actor_movies_csv;

HashMap<int, Actor> *actor_map;
HashMap<int, Movie> *movie_map;

BPlusTree<std::string, int> *actor_name_index;
BPlusTree<int, int> *actor_year_index;
BPlusTree<int, int> *movie_year_index;

HashMap<int, LinkedList<int>> *actor_movies;
HashMap<int, LinkedList<int>> *movie_actors;

// Function prototypes
void populate_main_hashmap();
void populate_index_trees();
void populate_relation_hashmaps();

int get_year();

void admin_handler(int input);

void user_handler(int input);
void display_actor_age_range();
void display_recent_movies();

void display_actor_age(BPlusTree<int, int> *actor_year_index, HashMap<int, Actor> *actor_map);

int main()
{
    // Variables
    bool admin = false;

    // Load data from CSV files
    actors = CSVParser::Parse<Actor>("data/actors.csv", &actor_count);
    movies = CSVParser::Parse<Movie>("data/movies.csv", &movie_count);
    actor_movies_csv = CSVParser::Parse<ActorMovie>("data/cast.csv", &actor_movie_count);

    // Initialise main hashmap, index trees & relation hashmaps
    actor_map = new HashMap<int, Actor>(actor_count);
    movie_map = new HashMap<int, Movie>(movie_count);

    actor_name_index = new BPlusTree<std::string, int>();
    actor_year_index = new BPlusTree<int, int>();
    movie_year_index = new BPlusTree<int, int>();

    actor_movies = new HashMap<int, LinkedList<int>>();
    movie_actors = new HashMap<int, LinkedList<int>>();

    // Populate main hashmap, index trees & relation hashmaps
    populate_main_hashmap();
    populate_index_trees();
    populate_relation_hashmaps();

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
    switch (input)
    {
    case 1:
        display_actor_age_range();
        break;
    case 2:
        display_recent_movies();
        break;
    default:
        break;
    }
}

void display_actor_age_range()
{
    int min_year, max_year, in;
    int current_year = get_year();

    std::cout << "Enter minimum age: ";
    std::cin >> in;
    max_year = current_year - in;

    std::cout << "Enter maximum age: ";
    std::cin >> in;
    min_year = current_year - in;

    auto it = actor_year_index->range_query(min_year, max_year);

    std::cout << "Actors born between " << min_year << " and " << max_year << ":" << std::endl;
    while (it.has_next())
    {
        Actor *actor = actor_map->get(*it.next());
        std::cout << actor->name << " (" << actor->year << ")" << std::endl;
    }
}

void display_recent_movies()
{
    int current_year = get_year();
    auto it = movie_year_index->range_query(current_year - 3, current_year);

    std::cout << "Movies released in the past 3 years:" << std::endl;
    while (it.has_next())
    {
        Movie *movie = movie_map->get(*it.next());
        std::cout << movie->title << " (" << movie->year << ")" << std::endl;
    }
}

// ===============================
// Helper functions
// ===============================

void populate_main_hashmap()
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

void populate_index_trees()
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

void populate_relation_hashmaps()
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

int get_year()
{
    std::time_t t = std::time(0);
    std::tm *now = std::localtime(&t);
    return now->tm_year + 1900;
}
