#include <iostream>
#include <string>
#include <ctime>

#include "dst/bplustree.h"
#include "dst/hashmap.h"
#include "dst/avl.h"

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

BPlusTree<const char *, int> *actor_name_index;
BPlusTree<const char *, int> *movie_name_index;
BPlusTree<int, int> *actor_year_index;
BPlusTree<int, int> *movie_year_index;

HashMap<int, AVLTree<const char *>> *actor_movies;
HashMap<int, AVLTree<const char *>> *movie_actors;

// Function prototypes
void populate_main_hashmap();
void populate_index_trees();
void populate_relation_hashmaps();

int get_year();

void admin_handler(int input);

void user_handler(int input);
void display_actor_age_range();
void display_recent_movies();
void display_actor_movies();

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

    actor_name_index = new BPlusTree<const char *, int>();
    movie_name_index = new BPlusTree<const char *, int>();

    actor_year_index = new BPlusTree<int, int>();
    movie_year_index = new BPlusTree<int, int>();

    actor_movies = new HashMap<int, AVLTree<const char *>>();
    movie_actors = new HashMap<int, AVLTree<const char *>>();

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
    case 3:
        display_actor_movies();
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
    int i = 1;
    while (it.has_next())
    {
        Actor *actor = actor_map->get(*it.next());
        std::cout << i << ". " << actor->name << " (" << actor->year << ")" << std::endl;
        i++;
    }
}

void display_recent_movies()
{
    int current_year = get_year();
    auto it = movie_year_index->range_query(current_year - 3, current_year);

    std::cout << "Movies released in the past 3 years:" << std::endl;

    if (!it.has_next())
    {
        std::cout << "No movies found." << std::endl;
        return;
    }

    int i = 1;
    while (it.has_next())
    {
        Movie *movie = movie_map->get(*it.next());
        std::cout << i << ". " << movie->title << " (" << movie->year << ")" << std::endl;
        i++;
    }
}

void display_actor_movies()
{
    std::string name;
    std::cout << "Enter actor name: ";
    std::cin.ignore();
    std::getline(std::cin, name);

    const char *name_data = name.c_str();
    int *actor_id = actor_name_index->search(name_data);
    if (actor_id == nullptr)
    {
        std::cout << "Actor not found." << std::endl;
        return;
    }

    AVLTree<const char *> *movies = actor_movies->get(*actor_id);
    if (movies == nullptr)
    {
        std::cout << "Actor has no movies." << std::endl;
        return;
    }

    std::cout << "Movies starring " << name << ":" << std::endl;
    auto it = movies->begin();
    int i = 1;
    while (it.has_next())
    {
        const char *movie_name = *it;
        int *movie_id = movie_name_index->search(movie_name);
        Movie *movie = movie_map->get(*movie_id);
        std::cout << i << ". " << movie->title << " (" << movie->year << ")" << std::endl;
        ++it;
        i++;
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
        movie_name_index->insert(movies[i].title, movies[i].id);
        movie_year_index->insert(movies[i].year, movies[i].id);
    }
}

void populate_relation_hashmaps()
{
    for (size_t i = 0; i < actor_movie_count; i++)
    {
        ActorMovie *actor_movie = &actor_movies_csv[i];
        Actor *actor = actor_map->get(actor_movie->actor_id);
        Movie *movie = movie_map->get(actor_movie->movie_id);

        AVLTree<const char *> *movies = actor_movies->get(actor->id);
        if (!movies)
        {
            actor_movies->insert(actor->id, AVLTree<const char *>());
            movies = actor_movies->get(actor->id);
        }
        movies->insert(movie->title);

        AVLTree<const char *> *actors = movie_actors->get(movie->id);
        if (!actors)
        {
            movie_actors->insert(movie->id, AVLTree<const char *>());
            actors = movie_actors->get(movie->id);
        }
        actors->insert(actor->name);
    }
}

int get_year()
{
    std::time_t t = std::time(0);
    std::tm *now = std::localtime(&t);
    return now->tm_year + 1900;
}
