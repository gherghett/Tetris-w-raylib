// vectormath2i.h
#ifndef VECTORMATH2I_H
#define VECTORMATH2I_H

typedef struct {
    int x;
    int y;
} Vector2i;

// Add two vectors
static inline Vector2i  Vector2iAdd(Vector2i a, Vector2i b) {
    return (Vector2i){ a.x + b.x, a.y + b.y };
}

// Subtract two vectors
static inline Vector2i Vector2iSubtract(Vector2i a, Vector2i b) {
    return (Vector2i){ a.x - b.x, a.y - b.y };
}

// Scale a vector by a scalar
static inline Vector2i Vector2iScale(Vector2i v, int scalar) {
    return (Vector2i){ v.x * scalar, v.y * scalar };
}

// Dot product
static inline int Vector2iDotProduct(Vector2i a, Vector2i b) {
    return a.x * b.x + a.y * b.y;
}

// Length squared (avoid float sqrt)
static inline int Vector2iLengthSqr(Vector2i v) {
    return v.x * v.x + v.y * v.y;
}

// Equality check
static inline int Vector2iEquals(Vector2i a, Vector2i b) {
    return (a.x == b.x) && (a.y == b.y);
}

#endif // VECTORMATH2I_H
