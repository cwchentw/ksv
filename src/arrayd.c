#include <assert.h>
#include <stdlib.h>
#include "arrayd.h"
#include "print.h"

struct arrayd_t {
    size_t size;
    size_t capacity;
    size_t head;
    size_t tail;
    double *elements;
};

arrayd_t * arrayd_new(void)
{
    arrayd_t *arr = (arrayd_t *) malloc(sizeof(arrayd_t));
    if (!arr) {
    #if DEBUG
        PUTERR("Failed to allocate memory for array of double");
        PUTERR("Check available system memory");
    #endif
        return arr;
    }

    arr->size = 0;
    arr->capacity = 2;

    arr->head = 0;
    arr->tail = 0;

    arr->elements = (double *) calloc(arr->capacity, sizeof(double));
    if (!(arr->elements)) {
    #if DEBUG
        PUTERR("Failed to allocate memory for the elements of array of double");
        PUTERR("Check available system memory");
    #endif
        free(arr);
        arr = NULL;
        return arr;
    }

    return arr;
}

void arrayd_delete(void *self)
{
    assert(self);

    double *arr = ((arrayd_t *) self)->elements;
    free((void *) arr);

    free(self);
}

ARRAYD_PUBLIC double arrayd_at(arrayd_t *self, size_t index)
{
    assert(self);
    assert(index < self->size);

    size_t i = (self->head + index) % self->size;

    return self->elements[i];
}

static BOOL arrayd_expand(arrayd_t *self);

BOOL arrayd_push(arrayd_t *self, double data)
{
    assert(self);

    if (!arrayd_expand(self))
        return FALSE;

    if (self->size > 0)
        self->tail += 1;

    self->elements[self->tail] = data;
    self->size += 1;

    return TRUE;
}

static BOOL arrayd_expand(arrayd_t *self)
{
    if (self->size < self->capacity)
        return TRUE;

    self->capacity <<= 1;
    double *old_arr = self->elements;
    double *new_arr = (double *) calloc(self->capacity, sizeof(double));
    if (!new_arr) {
    #if DEBUG
        PUTERR("Failed to allocate new array of double");
        PUTERR("Check available system memory");
    #endif
        return FALSE;
    }

    {
        size_t i = 0;
        size_t j = 0;
        while (i < self->size) {
            size_t index = (self->head + i) % self->size;
            new_arr[j] = old_arr[index];

            ++i;
            ++j;
        }
    }

    self->elements = new_arr;
    free((void *) old_arr);

    self->head = 0;
    self->tail = self->size - 1;

    return TRUE;
}

ARRAYD_PUBLIC void arrayd_sort(arrayd_t *self)
{
    assert(self);

    size_t i;
    for (i = 1; i < self->size; ++i) {
        size_t j = i;
        size_t p = (self->head + j - 1) % self->size;
        size_t q = (self->head + j) % self->size;
        while (j > 0 && self->elements[p] > self->elements[q]) {
            double temp = self->elements[q];
            self->elements[q] = self->elements[p];
            self->elements[p] = temp;
            
            --j;
            p = (self->head + j - 1) % self->size;
            q = (self->head + j) % self->size;
        }
    }
}