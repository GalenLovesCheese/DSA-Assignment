#include <string>

#include "classes/actor.h"
#include "classes/movie.h"

// Comparator functions for Actor
int compare_actor_name(const Actor* a, const Actor* b) {
    return strcmp(a->name, b->name);
}

int compare_actor_year(const Actor* a, const Actor* b) {
    if (a->year != b->year) return a->year - b->year;
    return strcmp(a->name, b->name); // Ensure total order
}

// Comparator functions for Movie
int compare_movie_title(const Movie* a, const Movie* b) {
    return strcmp(a->title, b->title);
}

int compare_movie_year(const Movie* a, const Movie* b) {
    if (a->year != b->year) return a->year - b->year;
    return strcmp(a->title, b->title); // Ensure total order
}

// Generic quicksort for any type
template <typename T>
void quicksort(T* arr, int left, int right, int (*comp)(const T*, const T*)) {
    if (left >= right) return;

    int pivot_idx = (left + right) / 2;
    T pivot = arr[pivot_idx];

    int i = left;
    int j = right;

    while (i <= j) {
        while (comp(&arr[i], &pivot) < 0) i++;
        while (comp(&arr[j], &pivot) > 0) j--;
        if (i <= j) {
            T temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
            i++;
            j--;
        }
    }

    quicksort(arr, left, j, comp);
    quicksort(arr, i, right, comp);
}