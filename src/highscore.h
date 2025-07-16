#ifndef HIGHSCORE_H
#define HIGHSCORE_H

#include <stdio.h>

#define HIGHSCORE_FILE "highscore.txt"

int LoadHighscore() {
    FILE *file = fopen(HIGHSCORE_FILE, "r");
    if (!file) return 0;

    int score = 0;
    fscanf(file, "%d", &score);
    fclose(file);
    return score;
}

void SaveHighscore(int score) {
    FILE *file = fopen(HIGHSCORE_FILE, "w");
    if (!file) return;

    fprintf(file, "%d\n", score);
    fclose(file);
}

#endif // HIGHSCORE_H