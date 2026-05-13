#include "../utilities/utilities.h"
#include "bar.h"

static inline void swap(Bar *a, Bar *b) {
    Bar temp = *a;
    *a = *b;
    *b = temp;
}

static int partition(Bar arr[], int low, int high) {
    uint64_t pivot = arr[high].size;/*last element as pivot*/
    int i = (low - 1),j;

    for (j = low; j <= high - 1; j++) {
        if (arr[j].size > pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void quickSortBars(Bar arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);

        quickSortBars(arr, low, pi - 1);
        quickSortBars(arr, pi + 1, high);
    }
}

