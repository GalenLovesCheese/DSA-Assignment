#include <iostream>
#include <string>
#include <ctime>

#include "algs/quicksort.h"

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
void display_add_new_actor();
void display_add_new_movie();
void display_add_actor_to_movie();
void display_update_actor_details();
void display_update_movie_details();

int main()
{
    // Variables
    bool admin = false;
    clock_t original_start = clock();
    clock_t start = clock();

// Load data from CSV files
#ifdef LARGE
    actors = CSVParser::Parse<Actor>("data/actors-large.csv", &actor_count);
#else
    actors = CSVParser::Parse<Actor>("data/actors-demo.csv", &actor_count);
#endif // LARGE
    DEBUG_PRINTF("Loaded %zu actors in %.2f seconds\n", actor_count,
                 (double)(clock() - start) / CLOCKS_PER_SEC);

    start = clock();
#ifdef LARGE
    movies = CSVParser::Parse<Movie>("data/movies-large.csv", &movie_count);
#else
    movies = CSVParser::Parse<Movie>("data/movies-demo.csv", &movie_count);
#endif // LARGE
    DEBUG_PRINTF("Loaded %zu movies in %.2f seconds\n", movie_count,
                 (double)(clock() - start) / CLOCKS_PER_SEC);

    start = clock();
#ifdef LARGE
    actor_movies_csv = CSVParser::Parse<ActorMovie>("data/cast-large.csv", &actor_movie_count);
#else
    actor_movies_csv = CSVParser::Parse<ActorMovie>("data/cast-demo.csv", &actor_movie_count);
#endif // LARGE
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
    populate_actor_indices();
    populate_movie_indices();
    DEBUG_PRINTF("Populated index trees in %.2f seconds\n",
                 (double)(clock() - start) / CLOCKS_PER_SEC);

    DEBUG_PRINTF("Total time taken: %.2f seconds\n", (double)(clock() - original_start) / CLOCKS_PER_SEC);

    // Main user interface loop
    int input = 0;
    int isAdmin = -1; // 1 - yes, 0 - no
    do
    {
        // administrator panel access
        while ((isAdmin != 1 && isAdmin != 0))
        {
            std::cout << "\nAccess Administrator Panel? (Enter 1 to proceed, 0 to proceed with Unprivileged Mode): ";
            std::cin >> isAdmin;
            if (isAdmin == 1)
            {
                admin = true;
                break;
            }
            if (isAdmin == 0)
            {
                break;
            }
        }

        std::cout << "========== Movie App by Bowen & Galen ==========" << std::endl;
        std::cout << "1. Display actors between a certain age range" << std::endl;
        std::cout << "2. Display movies released within the past 3 years" << std::endl;
        std::cout << "3. Display all movies an actor starred in" << std::endl;
        std::cout << "4. Display all actors in a movie" << std::endl;
        std::cout << "5. Display all actors that an actor knows" << std::endl;
        std::cout << std::endl;

        if (admin)
        {
            std::cout << "========== Admin Commands ==========" << std::endl;
            std::cout << "6. Add a new actor" << std::endl;
            std::cout << "7. Add a new movie" << std::endl;
            std::cout << "8. Add a new actor to a movie" << std::endl;
            std::cout << "9. Update actor details" << std::endl;
            std::cout << "10. Update movie details" << std::endl;
        }

        std::cout << "\nChoice (Enter '0' to quit): ";
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

    return 0;
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

// ===============================
// Assignment Functions (Admin)
// ===============================

void admin_handler(int input)
{
    switch (input)
    {

    case 6:
        display_add_new_actor();
        break;
    case 7:
        display_add_new_movie();
        break;
    case 8:
        display_add_actor_to_movie();
        break;
    case 9:
        display_update_actor_details();
        break;
    case 10:
        display_update_movie_details();
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
        movie_names->insertNode(movie_name);
    }

    std::cout << "Movies starring " << name << ":" << std::endl;
    int i = 1;
    for (auto it = movie_names->begin(); it != movie_names->end(); ++it)
    {
        std::cout << i << ". " << *it << std::endl;
        i++;
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
        actor_names->insertNode(actor_name);
    }

    std::cout << "Actors in " << title << ":" << std::endl;
    int i = 1;
    for (auto it = actor_names->begin(); it != actor_names->end(); ++it)
    {
        std::cout << i << ". " << *it << std::endl;
        i++;
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
    for (auto it = actor_names->begin(); it != actor_names->end(); ++it)
    {
        std::cout << i << ". " << *it << std::endl;
        i++;
    }
}

void display_add_new_actor()
{
    // actor details
    std::string actor_name;
    int year;
    int actor_id;

    std::cout << "Enter name of new actor: ";
    std::cin.ignore(); // ignore any leftover newline character in the input buffer
    std::getline(std::cin, actor_name);

    // Check if actor already exists
    if (actor_name_index->search(actor_name.c_str()))
    {
        std::cout << "Actor already exists." << std::endl;
        return;
    }

    std::cout << "Enter the year of birth of " << actor_name << ": ";
    std::cin >> year;

    actor_id = actor_map->getSize();

    // make sure that ids are not duplicated
    while (actor_map->get(actor_id))
    {
        actor_id++;
    }

    // creation of new actor object
    Actor new_actor;
    new_actor.name = new char[actor_name.length() + 1];
    std::strcpy(new_actor.name, actor_name.c_str());
    new_actor.id = actor_id;
    new_actor.year = year;
    new_actor.movies = new LinkedList<int>();

    // populating of main, and index hashmaps
    actor_map->insert(actor_id, new_actor);
    actor_name_index->insert(new_actor.name, actor_id);
    actor_year_index->insert(year, actor_id);
}
void display_add_new_movie()
{
    // movie details
    std::string movie_title;
    int year;
    int movie_id = 0;

    std::cout << "Enter title of new movie: ";
    std::cin.ignore();
    std::getline(std::cin, movie_title);
    std::cout << "Enter the year of release of " << movie_title << ": ";
    std::cin >> year;

    // make sure that ids are not duplicated
    while (movie_map->get(movie_id))
    {
        movie_id++;
    }

    // creation of new movie object
    Movie new_movie;
    new_movie.title = new char[movie_title.length() + 1];
    std::strcpy(new_movie.title, movie_title.c_str());
    new_movie.id = movie_id;
    new_movie.year = year;
    new_movie.actors = new LinkedList<int>();

    movie_map->insert(movie_id, new_movie);
    movie_name_index->insert(new_movie.title, movie_id);
    movie_year_index->insert(new_movie.year, movie_id);
}

void display_add_actor_to_movie()
{
    std::string movie_title;
    int movie_id;

    std::cout << "Enter title of movie: ";
    std::cin.ignore();
    std::getline(std::cin, movie_title);

    // search for specific movie id by title
    int *movie_id_ptr = movie_name_index->search(movie_title.c_str());
    if (movie_id_ptr != nullptr)
    {
        movie_id = *movie_id_ptr;
    }
    else
    {
        std::cout << "Movie not found." << std::endl;
        return;
    }

    // obtain cast of movie
    Movie *movie = movie_map->get(movie_id);
    LinkedList<int> *actor_ids = movie->actors;
    std::string input;

    // add actors to movie
    do
    {
        std::cout << "Please enter the name of actor to add (Enter 0 to exit): ";
        std::getline(std::cin, input);
        if (!actor_name_index->search(input.c_str()))
        {
            std::cout << "Actor does not exist." << std::endl;
        }
        else
        {
            int actor_id = *actor_name_index->search(input.c_str());
            if (actor_ids->contain(actor_id))
            {
                std::cout << "This actor is already recorded as a cast of the movie." << std::endl;
            }
            else if (input != "0")
            {
                Actor *actor = actor_map->get(actor_id);

                // add movie to actor's list of involved movies
                LinkedList<int> *movie_ids = actor->movies;
                movie_ids->push_back(movie_id);

                // add actor to movie's list of involved actors
                actor_ids->push_back(actor_id);
            }
        }
    } while (input != "0");
}

// helper function prototypes for updating actor details
void display_change_actor_name(int actor_id, std::string &actor_name);
void display_change_add_movie(int actor_id);
void display_change_remove_movie(int actor_id);
void display_remove_actor(int actor_id, std::string actor_name);

void display_update_actor_details()
{
    std::string actor_name;
    std::cout << "Enter the name of the actor you would like to modify: ";

    std::cin.ignore();
    std::getline(std::cin, actor_name);

    // search for specific actor id by title
    int *actor_id_ptr = actor_name_index->search(actor_name.c_str());
    if (actor_id_ptr == nullptr)
    {
        std::cout << "Actor not found." << std::endl;
        return;
    }
    int actor_id = *actor_id_ptr;

    int input = 0;
    do
    {
        // list of posisble modifications to actor record
        std::cout << "========== Modify Actor Details ==========" << std::endl;
        std::cout << "1. Change actor name" << std::endl;
        std::cout << "2. Add a movie" << std::endl;
        std::cout << "3. Remove a movie" << std::endl;
        std::cout << "4. Delete actor from record" << std::endl;

        std::cout << std::endl;

        std::cout << "\nChoice (Enter '0' to quit): ";
        std::cin >> input;

        if (input > 0 && input < 5)
        {
            int actor_index;
            std::string new_actor_name;
            switch (input)
            {
            case 1:
                display_change_actor_name(actor_id, actor_name);
                break;
            case 2:
                display_change_add_movie(actor_id);
                break;
            case 3:
                display_change_remove_movie(actor_id);
                break;
            case 4:
                display_remove_actor(actor_id, actor_name);
                input = 0; // break out of loop after deleting actor
                break;
            }
        }
    } while (input != 0);
}

// helper function prototypes for updating movie details
void display_change_movie_title(int movie_id, std::string &movie_title);
void display_change_add_actor(int movie_id);
void display_change_remove_actor(int movie_id);
void display_remove_movie(int movie_id, std::string movie_title);

void display_update_movie_details()
{
    std::string movie_title;
    std::cout << "Enter the title of the movie you would like to modify: ";

    std::cin.ignore();
    std::getline(std::cin, movie_title);

    // search for specific movie id by title
    int *movie_id_ptr = movie_name_index->search(movie_title.c_str());
    if (movie_id_ptr == nullptr)
    {
        std::cout << "Movie not found." << std::endl;
        return;
    }
    int movie_id = *movie_id_ptr;

    int input = 0;
    do
    {
        // list of possible modifications to movie record
        std::cout << "========== Modify Movie Details ==========" << std::endl;
        std::cout << "1. Change movie title" << std::endl;
        std::cout << "2. Add actor(s)" << std::endl;
        std::cout << "3. Remove actor(s)" << std::endl;
        std::cout << "4. Delete movie from record" << std::endl;

        std::cout << std::endl;

        std::cout << "\nChoice (Enter '0' to quit): ";
        std::cin >> input;

        if (input > 0 && input < 5)
        {
            switch (input)
            {
            case 1:
                display_change_movie_title(movie_id, movie_title);
                break;
            case 2:
                display_change_add_actor(movie_id);
                break;
            case 3:
                display_change_remove_actor(movie_id);
                break;
            case 4:
                display_remove_movie(movie_id, movie_title);
                input = 0; // break out of loop after deleting movie
                break;
            }
        }
    } while (input != 0);
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
                    actor_names->insertNode(actor_name);

                    AVLTree<std::string> *deeper_relations = get_actor_relations(*it2, depth - 1, original_name);
                    if (deeper_relations != nullptr)
                    {
                        for (auto deeper_it = deeper_relations->begin(); deeper_it != deeper_relations->end(); ++deeper_it)
                        {
                            actor_names->insertNode(*deeper_it);
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
        LinkedList<int> *actor_movie = actor_movies->get(actor->id);
        if (actor_movie == nullptr)
        {
            actor->movies = new LinkedList<int>();
        }
        else
        {
            actor->movies = actor_movie;
        }
        actor_map->insert(actor->id, *actor);
    }

    for (size_t i = 0; i < movie_count; i++)
    {
        Movie *movie = &movies[i];
        LinkedList<int> *movie_actor = movie_actors->get(movie->id);
        if (movie_actor == nullptr)
        {
            movie->actors = new LinkedList<int>();
        }
        else
        {
            movie->actors = movie_actor;
        }
        movie_map->insert(movie->id, *movie);
    }
}

void populate_actor_indices()
{
    populate_actor_name_index();
    populate_actor_year_index();
}

void populate_actor_name_index()
{
    DEBUG_PRINTF("Populating actor name index...\n");
    Actor *actors_copy = new Actor[actor_count];
    memcpy(actors_copy, actors, sizeof(Actor) * actor_count);
    quicksort<Actor>(actors_copy, 0, actor_count - 1, compare_actor_name);

    const char **names = new const char *[actor_count];
    int *ids = new int[actor_count];
    for (size_t i = 0; i < actor_count; ++i)
    {
        names[i] = actors_copy[i].name;
        ids[i] = actors_copy[i].id;
    }

    actor_name_index->bulk_load(names, ids, actor_count);

    delete[] actors_copy;
    delete[] names;
    delete[] ids;
}

void populate_actor_year_index()
{
    DEBUG_PRINTF("Populating actor year index...\n");
    Actor *actors_copy = new Actor[actor_count];
    memcpy(actors_copy, actors, sizeof(Actor) * actor_count);
    quicksort<Actor>(actors_copy, 0, actor_count - 1, compare_actor_year);

    int *years = new int[actor_count];
    int *ids = new int[actor_count];
    for (size_t i = 0; i < actor_count; ++i)
    {
        years[i] = actors_copy[i].year;
        ids[i] = actors_copy[i].id;
    }

    actor_year_index->bulk_load(years, ids, actor_count);

    delete[] actors_copy;
    delete[] years;
    delete[] ids;
}

void populate_movie_indices()
{
    populate_movie_name_index();
    populate_movie_year_index();
}

void populate_movie_name_index()
{
    DEBUG_PRINTF("Populating movie name index...\n");
    Movie *movies_copy = new Movie[movie_count];
    memcpy(movies_copy, movies, sizeof(Movie) * movie_count);
    quicksort<Movie>(movies_copy, 0, movie_count - 1, compare_movie_title);

    const char **titles = new const char *[movie_count];
    int *ids = new int[movie_count];
    for (size_t i = 0; i < movie_count; ++i)
    {
        titles[i] = movies_copy[i].title;
        ids[i] = movies_copy[i].id;
    }

    movie_name_index->bulk_load(titles, ids, movie_count);

    delete[] movies_copy;
    delete[] titles;
    delete[] ids;
}

void populate_movie_year_index()
{
    DEBUG_PRINTF("Populating movie year index...\n");
    Movie *movies_copy = new Movie[movie_count];
    memcpy(movies_copy, movies, sizeof(Movie) * movie_count);
    quicksort<Movie>(movies_copy, 0, movie_count - 1, compare_movie_year);

    int *years = new int[movie_count];
    int *ids = new int[movie_count];
    for (size_t i = 0; i < movie_count; ++i)
    {
        years[i] = movies_copy[i].year;
        ids[i] = movies_copy[i].id;
    }

    movie_year_index->bulk_load(years, ids, movie_count);

    delete[] movies_copy;
    delete[] years;
    delete[] ids;
}

int get_year()
{
    std::time_t t = std::time(0);
    std::tm *now = std::localtime(&t);
    return now->tm_year + 1900;
}

void display_change_actor_name(int actor_id, std::string &actor_name)
{
    std::string new_actor_name;
    std::cout << "Enter new name for " << actor_name << ": ";
    std::cin.ignore();
    std::getline(std::cin, new_actor_name);
    // obtain actor_index
    int actor_index = actor_id;
    // update actor name
    Actor updated_actor = *actor_map->get(actor_index);
    updated_actor.name = new char[new_actor_name.length() + 1];
    std::strcpy(updated_actor.name, new_actor_name.c_str());

    // update main actor hashmap
    actor_map->insert(actor_index, updated_actor);

    // update actor index
    actor_name_index->insert(new_actor_name.c_str(), actor_index);
    actor_name_index->remove(actor_name.c_str());
    actor_name_index->insert(updated_actor.name, actor_index);
}

void display_change_add_movie(int actor_id)
{

    // obtain actor movie list to add to
    Actor *actor = actor_map->get(actor_id);
    LinkedList<int> *actor_movies = actor->movies;

    std::string movie_title;
    std::cin.ignore(); // ignore any leftover newline character in the input buffer
    do
    {
        std::cout << "Enter title of movie to add (Enter 0 to exit): ";
        std::getline(std::cin, movie_title);

        // search for specific movie id by title
        if (movie_title != "0")
        {
            int *movie_id_ptr = movie_name_index->search(movie_title.c_str());
            if (movie_id_ptr != nullptr)
            {
                int movie_id = *movie_id_ptr;

                // modify movie list to reflect actor involvement
                Movie *movie = movie_map->get(movie_id);
                LinkedList<int> *actor_list = movie->actors;

                // check if actor is already involved in movie
                if (actor_list->contain(actor_id))
                {
                    std::cout << "This actor is already recorded as a cast of the movie." << std::endl;
                    return;
                }

                actor_movies->push_back(movie_id);
                actor_list->push_back(actor_id);
            }
            else
            {
                std::cout << "Movie not found." << std::endl;
            }
        }

    } while (movie_title != "0");
}

void display_change_remove_movie(int actor_id)
{
    // obtain actor movie list to remove from
    Actor *actor = actor_map->get(actor_id);
    LinkedList<int> *actor_movies = actor->movies;

    std::string movie_title;
    std::cin.ignore(); // ignore any leftover newline character in the input buffer
    do
    {
        std::cout << "Enter title of movie to remove (Enter 0 to exit): ";
        std::getline(std::cin, movie_title);

        // search for specific movie id by title
        if (movie_title != "0")
        {
            int *movie_id_ptr = movie_name_index->search(movie_title.c_str());
            if (movie_id_ptr != nullptr)
            {
                int movie_id = *movie_id_ptr;

                // modify movie list to reflect actor removal
                Movie *movie = movie_map->get(movie_id);
                LinkedList<int> *actor_list = movie->actors;

                // check if actor is involved in movie
                if (!actor_list->contain(actor_id))
                {
                    std::cout << "This actor is not recorded as a cast of the movie." << std::endl;
                    return;
                }

                actor_movies->remove(movie_id);
                actor_list->remove(actor_id);
            }
            else
            {
                std::cout << "Movie not found." << std::endl;
            }
        }

    } while (movie_title != "0");
}

void display_remove_actor(int actor_id, std::string actor_name)
{
    // Retrieve actor and their movies
    Actor *actor = actor_map->get(actor_id);
    LinkedList<int> *actor_movies = actor->movies;

    // Remove actor from actor_map
    actor_map->remove(actor_id);

    // Remove actor from actor_name_index
    actor_name_index->remove(actor_name.c_str());

    // Remove actor from actor_year_index
    actor_year_index->remove(actor->year);

    // Remove actor from all movies they are associated with
    for (auto it = actor_movies->begin(); it != actor_movies->end(); ++it)
    {
        Movie *movie = movie_map->get(*it);
        movie->actors->remove(actor_id);
    }

    // Free memory allocated for actor name
    delete[] actor->name;
    delete actor_movies;
}

void display_change_movie_title(int movie_id, std::string &movie_title)
{
    std::string new_movie_title;
    std::cout << "Enter new title for " << movie_title << ": ";
    std::cin.ignore();
    std::getline(std::cin, new_movie_title);

    // Update movie title
    Movie updated_movie = *movie_map->get(movie_id);
    updated_movie.title = new char[new_movie_title.length() + 1];
    std::strcpy(updated_movie.title, new_movie_title.c_str());

    // Update main movie hashmap
    movie_map->insert(movie_id, updated_movie);

    // Update movie index
    movie_name_index->remove(movie_title.c_str());
    movie_name_index->insert(updated_movie.title, movie_id);

    movie_title = new_movie_title;
}

void display_change_add_actor(int movie_id)
{
    // Obtain movie actor list to add to
    Movie *movie = movie_map->get(movie_id);
    LinkedList<int> *movie_actors = movie->actors;

    std::string actor_name;
    std::cin.ignore();
    do
    {
        std::cout << "Enter name of actor to add (Enter 0 to exit): ";
        std::getline(std::cin, actor_name);

        // Search for specific actor id by name
        if (actor_name != "0")
        {
            int *actor_id_ptr = actor_name_index->search(actor_name.c_str());
            if (actor_id_ptr != nullptr)
            {
                int actor_id = *actor_id_ptr;

                // Modify actor list to reflect movie involvement
                Actor *actor = actor_map->get(actor_id);
                LinkedList<int> *movie_list = actor->movies;

                // Check if actor is already involved in movie
                if (movie_actors->contain(actor_id))
                {
                    std::cout << "This actor is already recorded as a cast of the movie." << std::endl;
                    return;
                }

                movie_actors->push_back(actor_id);
                movie_list->push_back(movie_id);
            }
            else
            {
                std::cout << "Actor not found." << std::endl;
            }
        }

    } while (actor_name != "0");
}

void display_change_remove_actor(int movie_id)
{
    // Obtain movie actor list to remove from
    Movie *movie = movie_map->get(movie_id);
    LinkedList<int> *movie_actors = movie->actors;

    std::string actor_name;
    std::cin.ignore();
    do
    {
        std::cout << "Enter name of actor to remove (Enter 0 to exit): ";
        std::getline(std::cin, actor_name);

        // Search for specific actor id by name
        if (actor_name != "0")
        {
            int *actor_id_ptr = actor_name_index->search(actor_name.c_str());
            if (actor_id_ptr != nullptr)
            {
                int actor_id = *actor_id_ptr;

                // Modify actor list to reflect actor removal
                Actor *actor = actor_map->get(actor_id);
                LinkedList<int> *movie_list = actor->movies;

                // Check if actor is involved in movie
                if (!movie_actors->contain(actor_id))
                {
                    std::cout << "This actor is not recorded as a cast of the movie." << std::endl;
                    return;
                }

                movie_actors->remove(actor_id);
                movie_list->remove(movie_id);
            }
            else
            {
                std::cout << "Actor not found." << std::endl;
            }
        }

    } while (actor_name != "0");
}

void display_remove_movie(int movie_id, std::string movie_title)
{
    Movie *movie = movie_map->get(movie_id);

    // Remove movie from movie_map
    movie_map->remove(movie_id);

    // Remove movie from movie_name_index
    movie_name_index->remove(movie_title.c_str());

    // Remove movie from movie_year_index
    movie_year_index->remove(movie->year);

    // Remove movie from all actors associated with it
    LinkedList<int> *movie_actors = movie->actors;
    for (auto it = movie_actors->begin(); it != movie_actors->end(); ++it)
    {
        Actor *actor = actor_map->get(*it);
        actor->movies->remove(movie_id);
    }

    // Free memory allocated for movie title
    delete[] movie->title;
    delete movie_actors;
}
