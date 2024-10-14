#ifndef GC_BYTEFIXUP_H
#define GC_BYTEFIXUP_H

#include "types.h"
#include "common/Vector.h"

void ByteReverseVector(Vector&);

template <typename T>
void ByteReverse(T& start)
{
    char *buffer = (char*)(&start);
    int size = sizeof(T);

    for (int i = 0; i < size / 2; i++) {
        char tmp = buffer[i];
        buffer[i] = buffer[size - i - 1];
        buffer[size - i - 1] = tmp;
    }
}

template <typename T>
static void Fixup(T*& data, int baseAddress) {
    if (data != NULL) {
        ByteReverse<T*>(data);
        data = (T*)((int)data + baseAddress);
    }
}

/*template <>
static void Fixup<Vector>(Vector*& data, int baseAddress) {
    if (data != NULL) {
        ByteReverse<Vector*>(data);
        data = (Vector*)((int)data + baseAddress);
    }
}

template <>
static void Fixup<AnimationData::Node::KeyFrame>(AnimationData::Node::KeyFrame*& data, int baseAddress) {
    if (data != NULL) {
        ByteReverse<AnimationData::Node::KeyFrame*>(data);
        data = (AnimationData::Node::KeyFrame*)((int)data + baseAddress);
    }
}

template <>
static void Fixup<AnimDef>(AnimDef*& data, int baseAddress) {
    if (data != NULL) {
        ByteReverse<AnimDef*>(data);
        data = (AnimDef*)((int)data + baseAddress);
    }
}

template <>
static void Fixup<AnimationData::Node>(AnimationData::Node*& data, int baseAddress) {
    if (data != NULL) {
        ByteReverse<AnimationData::Node*>(data);
        data = (AnimationData::Node*)((int)data + baseAddress);
    }
}*/

#endif // GC_BYTEFIXUP_H