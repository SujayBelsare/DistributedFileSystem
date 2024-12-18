#include "identifiers.h"

#define MAX_SIZE 100000

MinHeap *createMinHeap()
{
    MinHeap *heap = (MinHeap *)malloc(sizeof(MinHeap));
    heap->array = (Connection **)malloc(MAX_SIZE * sizeof(Connection *));
    heap->size = 0;
    heap->capacity = MAX_SIZE;
    return heap;
}

// Helper function to swap two Connection pointers
void swap(Connection **a, Connection **b)
{
    Connection *temp = *a;
    *a = *b;
    *b = temp;
}

// Function to get parent index
int parent(int i)
{
    return (i - 1) / 2;
}

// Function to get left child index
int leftChild(int i)
{
    return 2 * i + 1;
}

// Function to get right child index
int rightChild(int i)
{
    return 2 * i + 2;
}

// Function to insert a new Connection pointer into the heap
void insert(MinHeap *heap, Connection *connection)
{
    if (heap->size >= heap->capacity)
    {
        printf("Heap overflow\n");
        return;
    }

    // Insert the new element at the end
    heap->size++;
    int i = heap->size - 1;
    heap->array[i] = connection;

    // Fix the min heap property if it's violated
    while (i != 0 && heap->array[parent(i)]->filecount > heap->array[i]->filecount)
    {
        swap(&heap->array[i], &heap->array[parent(i)]);
        i = parent(i);
    }
}

// Function to heapify a subtree rooted at given index
void minHeapify(MinHeap *heap, int i)
{
    int l = leftChild(i);
    int r = rightChild(i);
    int smallest = i;

    if (l < heap->size && heap->array[l]->filecount < heap->array[smallest]->filecount)
        smallest = l;

    if (r < heap->size && heap->array[r]->filecount < heap->array[smallest]->filecount)
        smallest = r;

    if (smallest != i)
    {
        swap(&heap->array[i], &heap->array[smallest]);
        minHeapify(heap, smallest);
    }
}

// Function to extract the minimum element from heap
Connection *extractMin(MinHeap *heap)
{
    if (heap->size <= 0)
    {
        return NULL;
    }

    if (heap->size == 1)
    {
        heap->size--;
        return heap->array[0];
    }

    Connection *root = heap->array[0];
    heap->array[0] = heap->array[heap->size - 1];
    heap->size--;
    minHeapify(heap, 0);

    return root;
}

// Function to get the minimum element without removing it
Connection *getMin(MinHeap *heap)
{
    if (heap->size <= 0)
    {
        return NULL;
    }
    return heap->array[0];
}

// Function to free the heap memory
void destroyMinHeap(MinHeap *heap)
{
    // Note: This does not free the Connection structures themselves
    // as they are managed externally
    free(heap->array);
    free(heap);
}