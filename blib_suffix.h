#ifndef _BLIB_SUFFIX_DEF_
#define _BLIB_SUFFIX_DEF_
#include "blib_error.h"
#include <string.h>
typedef struct blib_suffix_array_t {
  size_t allocated;
  size_t used;
  char *original = 0;
  size_t *indexes = 0;

} blib_suffix_array;

blib_suffix_array *blib_suffix_array_new(size_t len) {
  blib_suffix_array *arr =
      (blib_suffix_array *)malloc(sizeof(blib_suffix_array));
  if (arr == NULL)
    BLIB_ERROR("Out of memory");
  arr->original = (char *)malloc(len * sizeof(char));
  if (arr->original == NULL)
    BLIB_ERROR("Out of memory");
  arr->indexes = (size_t *)malloc(len * sizeof(size_t));
  if (arr->indexes == NULL)
    BLIB_ERROR("Out of memory");
  arr->allocated = len;
  arr->used = 0;
  return arr;
}

void blib_suffix_array_free(blib_suffix_array *p) {
  free(p->indexes);
  free(p->original);
  free(p);
}

void blib_suffix_array_sort(char *original, size_t *indexes, size_t len) {}

void blib_suffix_array_insert(char *s, size_t size, blib_suffix_array *arr) {
  // memcopy the string
  char *new_s;
  size_t i, j, k;
  blib_suffix_array *new_a;
  // Is this legit? creating a struct just to rob it's allocation then freeing
  // the shell and old allocations? Pro simple ish. Con, memory fragmentation
  // between struct and child arrays.
  if (arr->used + size < arr->allocated) {
    new_a = blib_suffix_array_new(arr->used + size);
    memcpy(new_a->original, arr->original, arr->used * sizeof(char));
    free(arr->original);
    arr->original = new_a->original;
    memcpy(new_a->indexes, arr->indexes, arr->used * sizeof(size_t));
    free(arr->indexes);
    arr->indexes = new_a->indexes;
    arr->allocated = new_a->allocated;
    free(new_a);
  }
  memcpy(&arr->original[arr->used], s, size);
  arr->used += size;
  // re-calculate suffix array
  blib_suffix_array_sort(arr->original, arr->indexes, arr->used);
}

#ifdef BLIB_UNIT_TEST
int blib_suffix_unit(void) {

  // I can allocate and dealocate without crashing

  blib_suffix_array *b = blib_suffix_array_new(1);
  blib_suffix_array_free(b);

  b = blib_suffix_array_new(0);
  blib_suffix_array_free(b);

  b = blib_suffix_array_new(100);

  // I can insert
  blib_suffix_array_insert("cat", 3, b);
  if (b->used != 3)
    BLIB_ERROR("BAD INSERT");
  if (b->allocated != 100)
    BLIB_ERROR("BAD INSERT");
  blib_suffix_array_free(b);
  // I can insert bigger and it will grow
  b = blib_suffix_array_new(0);
  blib_suffix_array_insert("cat", 3, b);

  return 0;
}

#endif

#endif /*_BLIB_SUFFIX_DEF_*/
