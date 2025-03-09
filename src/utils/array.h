#ifndef ARRAY_H_
#define ARRAY_H_

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include "arena.h"

#if __STDC_VERSION__ >= 202311L
#define CLAY__DEFAULT_STRUCT \
  {}
#else
#define DEFAULT_STRUCT {0}
#endif

#define ARRAY_BOUNDS_CHECK(index, upper) \
  (index < upper && index >= 0)          \
      ? true                             \
      : (assert(false && "index out of bounds"), false)

#define ARRAY__DECLARE(typeName, arrayName)                                   \
  typedef struct {                                                            \
    int32_t capacity;                                                         \
    int32_t length;                                                           \
    typeName* internalArray;                                                  \
  } arrayName;                                                                \
                                                                              \
  typedef struct {                                                            \
    int32_t length;                                                           \
    typeName* internalArray;                                                  \
  } arrayName##Slice;                                                         \
                                                                              \
  arrayName arrayName##_ArenaAllocate(int32_t capacity, Arena* arena);        \
  void arrayName##_Clear(arrayName* array);                                   \
  typeName* arrayName##_Get(arrayName* array, int32_t index);                 \
  typeName arrayName##_GetValue(arrayName* array, int32_t index);             \
  typeName* arrayName##_Add(arrayName* array, typeName item);                 \
  typeName* arrayName##Slice_Get(arrayName##Slice* slice, int32_t index);     \
  typeName arrayName##Slice_GetValue(arrayName##Slice* slice, int32_t index); \
  typeName arrayName##_Remove(arrayName* array, int32_t index);               \
  typeName arrayName##_RemoveSwapback(arrayName* array, int32_t index);       \
  void arrayName##_Set(arrayName* array, int32_t index, typeName value);      \
  int32_t arrayName##_FindIndex(arrayName* array, typeName value,             \
                                bool (*eqFunction)(typeName a, typeName b));  \
  void arrayName##_Insert(arrayName* array, int32_t index, typeName value);   \
  void arrayName##_InsertSorted(arrayName* array, typeName value,             \
                                bool (*gtFunction)(typeName a, typeName b));

#define ARRAY__DEFINE(typeName, arrayName)                                     \
  typeName typeName##_ARRAY_DEFAULT = DEFAULT_STRUCT;                          \
                                                                               \
  arrayName arrayName##_ArenaAllocate(int32_t capacity, Arena* arena) {        \
    return (arrayName){.capacity = capacity,                                   \
                       .length = 0,                                            \
                       .internalArray = (typeName*)Arena_Allocate(             \
                           capacity * sizeof(typeName), arena)};               \
  }                                                                            \
                                                                               \
  void arrayName##_Clear(arrayName* array) {                                   \
    array->length = 0;                                                         \
  }                                                                            \
                                                                               \
  typeName* arrayName##_Get(arrayName* array, int32_t index) {                 \
    return (ARRAY_BOUNDS_CHECK(index, array->length))                          \
               ? &array->internalArray[index]                                  \
               : &typeName##_ARRAY_DEFAULT;                                    \
  }                                                                            \
                                                                               \
  typeName arrayName##_GetValue(arrayName* array, int32_t index) {             \
    return (ARRAY_BOUNDS_CHECK(index, array->length))                          \
               ? array->internalArray[index]                                   \
               : typeName##_ARRAY_DEFAULT;                                     \
  }                                                                            \
                                                                               \
  typeName* arrayName##_Add(arrayName* array, typeName item) {                 \
    if (ARRAY_BOUNDS_CHECK(array->length + 1, array->capacity + 1)) {          \
      int32_t index = array->length++;                                         \
      array->internalArray[index] = item;                                      \
      return &array->internalArray[index];                                     \
    }                                                                          \
    return &typeName##_ARRAY_DEFAULT;                                          \
  }                                                                            \
                                                                               \
  typeName* arrayName##Slice_Get(arrayName##Slice* slice, int32_t index) {     \
    return (ARRAY_BOUNDS_CHECK(index, slice->length))                          \
               ? &slice->internalArray[index]                                  \
               : &typeName##_ARRAY_DEFAULT;                                    \
  }                                                                            \
                                                                               \
  typeName arrayName##Slice_GetValue(arrayName##Slice* slice, int32_t index) { \
    return (ARRAY_BOUNDS_CHECK(index, slice->length))                          \
               ? slice->internalArray[index]                                   \
               : typeName##_ARRAY_DEFAULT;                                     \
  }                                                                            \
                                                                               \
  typeName arrayName##_Remove(arrayName* array, int32_t index) {               \
    if (ARRAY_BOUNDS_CHECK(index, array->length)) {                            \
      int32_t arrayLength = array->length--;                                   \
      typeName removed = array->internalArray[index];                          \
      for (int32_t i = index; i < arrayLength; i++) {                          \
        array->internalArray[i] = array->internalArray[i + 1];                 \
      }                                                                        \
      return removed;                                                          \
    }                                                                          \
    return typeName##_ARRAY_DEFAULT;                                           \
  }                                                                            \
                                                                               \
  typeName arrayName##_RemoveSwapback(arrayName* array, int32_t index) {       \
    if (ARRAY_BOUNDS_CHECK(index, array->length)) {                            \
      typeName removed = array->internalArray[index];                          \
      array->internalArray[index] = array->internalArray[--array->length];     \
      return removed;                                                          \
    }                                                                          \
    return typeName##_ARRAY_DEFAULT;                                           \
  }                                                                            \
                                                                               \
  void arrayName##_Set(arrayName* array, int32_t index, typeName value) {      \
    if (ARRAY_BOUNDS_CHECK(index, array->capacity)) {                          \
      array->internalArray[index] = value;                                     \
      array->length = index < array->length ? array->length : index + 1;       \
    }                                                                          \
  }                                                                            \
                                                                               \
  int32_t arrayName##_FindIndex(arrayName* array, typeName value,              \
                                bool (*eqFunction)(typeName a, typeName b)) {  \
    for (int32_t i = 0; i < array->length; i++) {                              \
      if (eqFunction(array->internalArray[i], value))                          \
        return i;                                                              \
    }                                                                          \
    return -1;                                                                 \
  }                                                                            \
                                                                               \
  void arrayName##_Insert(arrayName* array, int32_t index, typeName value) {   \
    if (index < 0) {                                                           \
      index = array->length + 1 + index;                                       \
    }                                                                          \
    array->length += 1;                                                        \
    for (int32_t i = array->length - 1; i > index - 1; i--) {                  \
      array->internalArray[i + 1] = array->internalArray[i];                   \
    }                                                                          \
    array->internalArray[index] = value;                                       \
  }                                                                            \
                                                                               \
  void arrayName##_InsertSorted(arrayName* array, typeName value,              \
                                bool (*gtFunction)(typeName a, typeName b)) {  \
    int32_t largerIndex = arrayName##_FindIndex(array, value, gtFunction);     \
    arrayName##_Insert(array, largerIndex, value);                             \
  }

#endif  // ARRAY_H_
