#include <iostream>
#include <string>
#include <ctime>
#include <thread>

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

// Function prototypes
void populate_main_hashmap();
void populate_actor_indices();
void populate_actor_name_index();
void populate_actor_year_index();
void populate_movie_name_index();
void populate_movie_year_index();
void populate_movie_indices();
void populate_relation_hashmaps();

AVLTree<std::string> *get_actor_relations(int actor_id, int depth, const std::string &original_name);

int get_year();

void admin_handler(int input);
void user_handler(int input);

void display_actor_age_range();
void display_recent_movies();
void display_actor_movies();
void display_movie_actors();
void display_actor_relations();

int main()
{
    // Variables
    bool admin = false;
    clock_t original_start = clock();
    clock_t start = clock();

    // Load data from CSV files
    actors = CSVParser::Parse<Actor>("data/actors.csv", &actor_count);
    DEBUG_PRINTF("Loaded %zu actors in %.2f seconds\n", actor_count,
                 (double)(clock() - start) / CLOCKS_PER_SEC);

    start = clock();
    movies = CSVParser::Parse<Movie>("data/movies.csv", &movie_count);
    DEBUG_PRINTF("Loaded %zu movies in %.2f seconds\n", movie_count,
                 (double)(clock() - start) / CLOCKS_PER_SEC);

    start = clock();
    actor_movies_csv = CSVParser::Parse<ActorMovie>("data/cast.csv", &actor_movie_count);
    DEBUG_PRINTF("Loaded %zu cast relations in %.2f seconds\n", actor_movie_count,
                 (double)(clock() - start) / CLOCKS_PER_SEC);

    // Initialise main hashmap, index trees & relation hashmaps
    actor_map = new HashMap<int, Actor>(actor_count);
    movie_map = new HashMap<int, Movie>(movie_count);

    actor_name_index = new BPlusTree<const char *, int>();
    movie_name_index = new BPlusTree<const char *, int>();

    actor_year_index = new BPlusTree<int, int>();
    movie_year_index = new BPlusTree<int, int>();

    // Populate main hashmap, index trees & relation hashmaps
    start = clock();
    populate_main_hashmap();
    DEBUG_PRINTF("Populated hashmaps in %.2f seconds\n",
                 (double)(clock() - start) / CLOCKS_PER_SEC);

    start = clock();
    std::thread actor_index_thread(populate_actor_indices);
    std::thread movie_index_thread(populate_movie_indices);
    actor_index_thread.join();
    movie_index_thread.join();
    DEBUG_PRINTF("Populated index trees in %.2f seconds\n",
                 (double)(clock() - start) / CLOCKS_PER_SEC);

    DEBUG_PRINTF("Total time taken: %.2f seconds\n", (double)(clock() - original_start) / CLOCKS_PER_SEC);

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
    case 4:
        display_movie_actors();
        break;
    case 5:
        display_actor_relations();
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

    Actor *actor = actor_map->get(*actor_id);
    LinkedList<int> *movie_ids = actor->movies;
    if (movie_ids == nullptr)
    {
        std::cout << "Actor has no movies." << std::endl;
        return;
    }

    AVLTree<std::string> *movie_names = new AVLTree<std::string>();
    for (auto it = movie_ids->begin(); it != movie_ids->end(); ++it)
    {
        Movie *movie = movie_map->get(*it);
        std::string movie_name = movie->title;
        movie_name += " (" + std::to_string(movie->year) + ")";
        movie_names->insert(movie_name);
    }

    std::cout << "Movies starring " << name << ":" << std::endl;
    int i = 1;
    auto it = movie_names->begin();
    while (it.has_next())
    {
        std::cout << i << ". " << *it << std::endl;
        i++;
        ++it;
    }
}

void display_movie_actors()
{
    std::string title;
    std::cout << "Enter movie title: ";
    std::cin.ignore();
    std::getline(std::cin, title);

    const char *title_data = title.c_str();
    int *movie_id = movie_name_index->search(title_data);
    if (movie_id == nullptr)
    {
        std::cout << "Movie not found." << std::endl;
        return;
    }

    Movie *movie = movie_map->get(*movie_id);
    LinkedList<int> *actor_ids = movie->actors;
    if (actor_ids == nullptr)
    {
        std::cout << "Movie has no actors." << std::endl;
        return;
    }

    AVLTree<std::string> *actor_names = new AVLTree<std::string>();
    for (auto it = actor_ids->begin(); it != actor_ids->end(); ++it)
    {
        Actor *actor = actor_map->get(*it);
        std::string actor_name = actor->name;
        actor_name += " (" + std::to_string(actor->year) + ")";
        actor_names->insert(actor_name);
    }

    std::cout << "Actors in " << title << ":" << std::endl;
    int i = 1;
    auto it = actor_names->begin();
    while (it.has_next())
    {
        std::cout << i << ". " << *it << std::endl;
        i++;
        ++it;
    }
}

void display_actor_relations()
{
    std::string actor_name;
    std::cout << "Enter actor name: ";
    std::cin.ignore();
    std::getline(std::cin, actor_name);

    const char *actor_name_data = actor_name.c_str();
    int *actor_id = actor_name_index->search(actor_name_data);

    if (actor_id == nullptr)
    {
        std::cout << "Actor not found." << std::endl;
        return;
    }

    Actor *actor = actor_map->get(*actor_id);
    LinkedList<int> *actor_movies = actor->movies;
    if (actor_movies == nullptr)
    {
        std::cout << "Actor has no movies." << std::endl;
        return;
    }

    std::string formatted_actor_name = actor_name;
    formatted_actor_name += " (" + std::to_string(actor->year) + ")";
    AVLTree<std::string> *actor_names = get_actor_relations(*actor_id, 2, formatted_actor_name);

    std::cout << "Actors who have worked with " << actor_name << ":" << std::endl;
    int i = 1;
    auto it = actor_names->begin();
    while (it.has_next())
    {
        std::cout << i << ". " << *it << std::endl;
        i++;
        ++it;
    }
}

// ===============================
// Helper functions
// ===============================

AVLTree<std::string> *get_actor_relations(int actor_id, int depth, const std::string &original_name)
{
    if (depth <= 0)
        return nullptr;

    AVLTree<std::string> *actor_names = new AVLTree<std::string>();
    Actor *actor = actor_map->get(actor_id);

    LinkedList<int> *actor_movies = actor->movies;
    if (actor_movies == nullptr)
        return actor_names;

    for (auto it = actor_movies->begin(); it != actor_movies->end(); ++it)
    {
        Movie *movie = movie_map->get(*it);
        LinkedList<int> *movie_actors = movie->actors;

        for (auto it2 = movie_actors->begin(); it2 != movie_actors->end(); ++it2)
        {
            if (*it2 != actor_id)
            {
                Actor *other_actor = actor_map->get(*it2);
                std::string actor_name = other_actor->name;
                actor_name += " (" + std::to_string(other_actor->year) + ")";

                // Skip if it's the original actor or already visited
                if (actor_name != original_name)
                {
                    actor_names->insert(actor_name);

                    AVLTree<std::string> *deeper_relations = get_actor_relations(*it2, depth - 1, original_name);
                    if (deeper_relations != nullptr)
                    {
                        auto deeper_it = deeper_relations->begin();
                        while (deeper_it.has_next())
                        {
                            actor_names->insert(*deeper_it);
                            ++deeper_it;
                        }
                        delete deeper_relations;
                    }
                }
            }
        }
    }
    return actor_names;
}

void populate_main_hashmap()
{
    // Initialise hashmap cache
    HashMap<int, LinkedList<int>> *actor_movies = new HashMap<int, LinkedList<int>>(actor_movie_count);
    HashMap<int, LinkedList<int>> *movie_actors = new HashMap<int, LinkedList<int>>(actor_movie_count);

    // For each actor movie relation, populate hashmaps caches
    for (size_t i = 0; i < actor_movie_count; i++)
    {
        ActorMovie relationship = actor_movies_csv[i];
        int actor_id = relationship.actor_id;
        int movie_id = relationship.movie_id;

        LinkedList<int> *actor_movies_list = actor_movies->get(actor_id);
        if (actor_movies_list == nullptr)
        {
            actor_movies->insert(actor_id, LinkedList<int>());
            actor_movies_list = actor_movies->get(actor_id);
        }
        actor_movies_list->push_back(movie_id);

        LinkedList<int> *movie_actors_list = movie_actors->get(movie_id);
        if (movie_actors_list == nullptr)
        {
            movie_actors->insert(movie_id, LinkedList<int>());
            movie_actors_list = movie_actors->get(movie_id);
        }
        movie_actors_list->push_back(actor_id);
    }

    // Loop through actors and movies and populate their relations
    for (size_t i = 0; i < actor_count; i++)
    {
        Actor *actor = &actors[i];
        actor->movies = actor_movies->get(actor->id);
        actor_map->insert(actor->id, *actor);
    }

    for (size_t i = 0; i < movie_count; i++)
    {
        Movie *movie = &movies[i];
        movie->actors = movie_actors->get(movie->id);
        movie_map->insert(movie->id, *movie);
    }
}

void populate_actor_indices()
{
    DEBUG_PRINTF("Populating actor indices...\n");
    std::thread t1(populate_actor_name_index);
    std::thread t2(populate_actor_year_index);
    t1.join();
    t2.join();
}

void populate_actor_name_index()
{
    DEBUG_PRINTF("Populating actor name index...\n");
    for (size_t i = 0; i < actor_count; i++)
    {
        actor_name_index->insert(actors[i].name, actors[i].id);
    }
}

void populate_actor_year_index()
{
    DEBUG_PRINTF("Populating actor year index...\n");
    for (size_t i = 0; i < actor_count; i++)
    {
        actor_year_index->insert(actors[i].year, actors[i].id);
    }
}

void populate_movie_indices()
{
    DEBUG_PRINTF("Populating movie indices...\n");
    std::thread t1(populate_movie_name_index);
    std::thread t2(populate_movie_year_index);
    t1.join();
    t2.join();
}

void populate_movie_name_index()
{
    DEBUG_PRINTF("Populating movie name index...\n");
    for (size_t i = 0; i < movie_count; i++)
    {
        movie_name_index->insert(movies[i].title, movies[i].id);
    }
}

void populate_movie_year_index()
{
    DEBUG_PRINTF("Populating movie year index...\n");
    for (size_t i = 0; i < movie_count; i++)
    {
        movie_year_index->insert(movies[i].year, movies[i].id);
    }
}

int get_year()
{
    std::time_t t = std::time(0);
    std::tm *now = std::localtime(&t);
    return now->tm_year + 1900;
}
