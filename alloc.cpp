#include <unistd.h>
#include <string.h>
#include <pthread.h>

struct header
{
    header *next;
    size_t block_size;
    bool free;
};

header *head = NULL, *tail = NULL;
pthread_mutex_t global_malloc_lock;

void *malloc(size_t size)
{
    header *block;
    pthread_mutex_lock(&global_malloc_lock);
    if (!head)
    {
        block = (header *)sbrk(size + sizeof(header));
        block->block_size = size;
        block->free = 0;
        block->next = NULL;
        head = block;
        tail = head;
        pthread_mutex_unlock(&global_malloc_lock);
        return (void *)(block + 1);
    }
    header *tmp = head;
    while (tmp != NULL)
    {
        if (tmp->free && tmp->block_size >= size)
        {
            tmp->block_size = size;
            tmp->free = 0;
            pthread_mutex_unlock(&global_malloc_lock);
            return (void *)(tmp + 1);
        }
        tmp = tmp->next;
    }
    block = (header *)sbrk(size + sizeof(header));
    block->block_size = size;
    block->free = 0;
    block->next = NULL;
    tail->next = block;
    tail = block;
    pthread_mutex_unlock(&global_malloc_lock);
    return (void *)(block + 1);
}

void free(void *ptr)
{
    if (head == NULL)
        return;
    header *block = (header *)ptr - 1;
    if (block->free)
        return;
    pthread_mutex_lock(&global_malloc_lock);
    block->free = 1;
    if (tail == head)
    {
        head = tail = NULL;
        sbrk(-(block->block_size + sizeof(header)));
        pthread_mutex_unlock(&global_malloc_lock);

        return;
    }
    if (block == tail)
    {
        header *tmp = head;
        while (tmp->next != block)
            tmp = tmp->next;
        tmp->next = NULL;
        tail = tmp;
        sbrk(-(block->block_size + sizeof(header)));
        pthread_mutex_unlock(&global_malloc_lock);
        return;
    }
    pthread_mutex_unlock(&global_malloc_lock);
}

void *realloc(void *ptr, size_t size)
{
    header *block = (header *)ptr - 1;
    if (block->block_size >= size)
        return ptr;
    if (block == tail)
    {
        pthread_mutex_lock(&global_malloc_lock);
        size_t difference = size - block->block_size;
        sbrk(difference);
        block->block_size = size;
        pthread_mutex_unlock(&global_malloc_lock);
        return ptr;
    }
    header *tmp = head;
    while (tmp)
    {
        if (tmp->free && tmp->block_size >= size)
        {
            pthread_mutex_lock(&global_malloc_lock);
            header *dest = tmp + 1;
            memcpy((void *)dest, ptr, block->block_size);
            free(ptr);
            pthread_mutex_unlock(&global_malloc_lock);
            return (void *)dest;
        }
        tmp = tmp->next;
    }
    void *new_block = malloc(size);
    memcpy(new_block, ptr, block->block_size);
    free(ptr);
    return (void *)new_block;
}