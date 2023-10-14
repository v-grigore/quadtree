// GRIGORE Vlad - 313CC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

// Type definitions for pixel and quad tree node
typedef struct __attribute__((packed)) Pixel {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} Pixel;

typedef struct QuadTreeNode {
    struct QuadTreeNode *children[4];
    unsigned char type;
    Pixel values;
    int level;
} QuadTreeNode;

enum Flags{
    FLAG_ERROR,
    FLAG_C1,
    FLAG_C2,
    FLAG_D
};

// Return flag value based on argument
int getFlag(const char *arg) {
    if (!strcmp(arg, "-d"))
        return FLAG_D;

    if (!strcmp(arg, "-c1"))
        return FLAG_C1;

    if (!strcmp(arg, "-c2"))
        return FLAG_C2;

    return FLAG_ERROR;
}

// Read size and pixel matrix from .ppm file
int readImage(int *width, int *height, Pixel ***pixels, char *fileName) {
    char header[5];
    int maxValue;

    // Open .ppm file
    FILE *inputFile = fopen(fileName, "rb");
    if (!inputFile) {
        fprintf(stderr, "Error: Failed to open file\n");
        return -1;
    }

    // Check if format is valid
    if (!fgets(header, 5, inputFile) || header[0] != 'P' || header[1] != '6') {
        fprintf(stderr, "Error: Invalid file format\n");
        return -1;
    }

    // Read image size
    if (fscanf(inputFile, "%d %d", width, height) != 2) {
        fprintf(stderr, "Error: Invalid file format\n");
        return -1;
    }

    // Read max value for pixels
    if (fscanf(inputFile, "%d", &maxValue) != 1 || maxValue != 255) {
        fprintf(stderr, "Error: Invalid file format\n");
        return -1;
    }

    // Trailing newline
    char newLine;
    fread(&newLine, 1, 1, inputFile);

    // Allocate memory for pixel matrix
    *pixels = malloc(*height * sizeof(Pixel *));
    if (!*pixels) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return -1;
    }

    int i;
    for (i = 0; i < *height; i++) {

        // Allocate memory for pixels[i]
        (*pixels)[i] = malloc(*width * sizeof(Pixel));
        if (!(*pixels)[i]) {
            fprintf(stderr, "Error: Failed to allocate memory\n");
            return -1;
        }

        // Read pixels[i]
        if (fread((*pixels)[i], sizeof(Pixel), *width, inputFile) != *width) {
            fprintf(stderr, "Error: Failed to read pixel data\n");
            return -1;
        }
    }

    fclose(inputFile);
    return 0;
}

// Initialize quad tree
int initQuadTree(QuadTreeNode **root) {
    *root = malloc(sizeof(QuadTreeNode));
    if (!*root) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return -1;
    }

    int i;
    for (i = 0; i < 4; i++)
        (*root)->children[i] = NULL;

    return 0;
}

// Create new quad tree node
QuadTreeNode *newNode() {
    QuadTreeNode *newNode = malloc(sizeof(QuadTreeNode));
    if (!newNode) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return NULL;
    }

    int i;
    for (i = 0; i < 4; i++)
        newNode->children[i] = NULL;

    return newNode;
}

// Set corresponding node values
void setNodeValues(QuadTreeNode *node, Pixel **pixels, int x, int y, int size) {
    unsigned long long redSum = 0;
    unsigned long long greenSum = 0;
    unsigned long long blueSum = 0;

    int i, j;
    for (i = x; i < x + size; i++) {
        for (j = y; j < y + size; j++) {
            redSum += pixels[i][j].red;
            greenSum += pixels[i][j].green;
            blueSum += pixels[i][j].blue;
        }
    }

    node->values.red = redSum / (size * size);
    node->values.green = greenSum / (size * size);
    node->values.blue = blueSum / (size * size);
}

// Get the mean (similarity score) for node
unsigned int getMean(QuadTreeNode *node, Pixel **pixels, int x, int y,
                     int size) {

    unsigned long long sum = 0;

    int i, j;
    for (i = x; i < x + size; i++) {
        for (j = y; j < y + size; j++) {
            int red, green, blue;
            red = node->values.red - pixels[i][j].red;
            green = node->values.green - pixels[i][j].green;
            blue = node->values.blue - pixels[i][j].blue;

            sum += red * red + green * green + blue * blue;
        }
    }
    return sum / (size * size * 3);
}

// Recursively add nodes to quad tree until the tree is fully built
void addNode(QuadTreeNode *root, Pixel **pixels, int x, int y, int size,
             int level, unsigned int factor) {

    root->level = level;

    // Factors for moving in the matrix. 1 = move right/down
    int yFactor[] = {0, 1, 1, 0};
    int xFactor[] = {0, 0, 1, 1};

    setNodeValues(root, pixels, x, y, size);
    unsigned int mean = getMean(root, pixels, x, y, size);

    // Leaf node
    if (mean <= factor) {
        root->type = 1;

        return;
    }

    // Parent node
    root->type = 0;

    int i;
    for (i = 0; i < 4; i++) {
        int newX = x + size / 2 * xFactor[i];
        int newY = y + size / 2 * yFactor[i];

        root->children[i] = newNode();

        if (!root->children[i]) {
            fprintf(stderr,
                    "Error: Insufficient memory. Terminating program\n");
            exit(-1);
        }

        addNode(root->children[i], pixels, newX, newY, size / 2, level + 1,
                factor);
    }
}

// Compress the image and get corresponding quad tree
int getQuadTree(int width, Pixel **pixels, QuadTreeNode **root,
                unsigned int factor) {

    if (initQuadTree(root)) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return -1;
    }

    addNode(*root, pixels, 0, 0, width, 1, factor);

    return 0;
}

// Get the lowest (closest to root) level for a leaf node
int getLowestLevel(QuadTreeNode *root) {
    Queue *queue;

    // Initialize queue
    if (initQueue(&queue)) {
        fprintf(stderr, "Error: Insufficient memory. Terminating program\n");
        exit(-1);
    }

    // Enqueue root
    if (enqueue(queue, root)) {
        fprintf(stderr, "Error: Insufficient memory. Terminating program\n");
        exit(-1);
    }

    // Check for lowest level through bfs
    while (!isQueueEmpty(queue)) {
        QuadTreeNode *currentNode = queue->head->data;

        // Found the first (lowest level) leaf
        if (currentNode->type) {
            freeQueue(queue);
            return currentNode->level;
        }

        int i;
        for (i = 0; i < 4; i++) {
            if (enqueue(queue, currentNode->children[i])) {
                fprintf(stderr,
                        "Error: Insufficient memory. Terminating program\n");
                exit(-1);
            }
        }

        dequeue(queue);
    }

    // No leaf nodes found
    fprintf(stderr, "Error: Corrupted tree\n");
    return -1;
}

// Utility function for getting height through recursive dfs
void getHeightUtil(QuadTreeNode *root, int *height) {
    int i;
    if (!root->type) {
        for (i = 0; i < 4; i++)
            getHeightUtil(root->children[i], height);
    }

    if (root->level > *height)
        *height = root->level;
}

// Get height of quad tree
int getHeight(QuadTreeNode *root) {
    int height = root->level;
    getHeightUtil(root, &height);
    return height;
}

// Utility function for getting blocks count through recursive dfs
void getBlocksCntUtil(QuadTreeNode *root, int *cnt) {
    int i;
    if (!root->type)
        for (i = 0; i < 4; i++)
            getBlocksCntUtil(root->children[i], cnt);
    else
        (*cnt)++;
}

// Get blocks count for quad tree
int getBlocksCnt(QuadTreeNode *root) {
    int cnt = 0;
    getBlocksCntUtil(root, &cnt);
    return cnt;
}

// Write size and quad tree to binary file
void writeQuadTree(QuadTreeNode *root, unsigned int size, char *fileName) {

    // Open output file
    FILE *outputFile = fopen(fileName, "wb");
    if (!outputFile) {
        fprintf(stderr, "Error: Failed to open file. Terminating program\n");
        exit(-1);
    }

    // Write size
    fwrite(&size, sizeof(size), 1, outputFile);

    Queue *queue;

    // Initialize queue
    if (initQueue(&queue)) {
        fprintf(stderr, "Error: Insufficient memory. Terminating program\n");
        exit(-1);
    }

    // Enqueue root
    if (enqueue(queue, root)) {
        fprintf(stderr, "Error: Insufficient memory. Terminating program\n");
        exit(-1);
    }

    // Traverse quad tree through bfs
    while (!isQueueEmpty(queue)) {
        QuadTreeNode *currentNode = queue->head->data;

        fwrite(&currentNode->type, sizeof(currentNode->type), 1, outputFile);

        // If leaf node, write values and go to next node
        if (currentNode->type) {
            fwrite(&currentNode->values, sizeof(currentNode->values), 1,
                   outputFile);

            dequeue(queue);
            continue;
        }

        int i;
        for (i = 0; i < 4; i++) {
            if (enqueue(queue, currentNode->children[i])) {
                fprintf(stderr,
                        "Error: Insufficient memory. Terminating program\n");
                exit(-1);
            }
        }

        dequeue(queue);
    }

    freeQueue(queue);
    fclose(outputFile);
}

// Read quad tree from binary file
void readQuadTree(QuadTreeNode **root, unsigned int *size, char *fileName) {

    // Open input file
    FILE *inputFile = fopen(fileName, "rb");
    if (!inputFile) {
        fprintf(stderr, "Error: Failed to open file. Terminating program\n");
        exit(-1);
    }

    // Read size
    if (!fread(size, sizeof(unsigned int), 1, inputFile)) {
        fprintf(stderr, "Error: Corrupted file. Terminating program\n");
        exit(-1);
    }

    Queue *queue;

    // Initialize queue
    if (initQueue(&queue)) {
        fprintf(stderr, "Error: Insufficient memory. Terminating program\n");
        exit(-1);
    }

    // Initialize quad tree
    if (initQuadTree(root)) {
        fprintf(stderr, "Error: Insufficient memory. Terminating program\n");
        exit(-1);
    }
    (*root)->level = 1;

    // Enqueue root
    if (enqueue(queue, *root)) {
        fprintf(stderr, "Error: Insufficient memory. Terminating program\n");
        exit(-1);
    }

    // Read quad tree through bfs
    while (!isQueueEmpty(queue)) {
        QuadTreeNode *currentNode = queue->head->data;

        // Read node type
        if (!fread(&currentNode->type, sizeof(currentNode->type), 1,
                   inputFile)) {

            fprintf(stderr, "Error: Corrupted file. Terminating program\n");
            exit(-1);
        }

        // If leaf node, read values and go to next node
        if (currentNode->type) {
            if (!fread(&currentNode->values, sizeof(Pixel), 1, inputFile)) {
                fprintf(stderr, "Error: Corrupted file. Terminating program\n");
                exit(-1);
            }

            dequeue(queue);
            continue;
        }

        int i;
        for (i = 0; i < 4; i++) {
            currentNode->children[i] = newNode();

            if (!currentNode->children[i]) {
                fprintf(stderr,
                        "Error: Insufficient memory. Terminating program\n");
                exit(-1);
            }

            currentNode->children[i]->level = currentNode->level + 1;

            if (enqueue(queue, currentNode->children[i])) {
                fprintf(stderr,
                        "Error: Insufficient memory. Terminating program\n");
                exit(-1);
            }
        }

        dequeue(queue);
    }

    fclose(inputFile);
    freeQueue(queue);
}

// Utility function for recursively adding blocks to pixel matrix
void getImageUtil(QuadTreeNode *root, int size, int x, int y, Pixel **pixels) {

    // Factors for moving in the matrix. 1 = move right/down
    int yFactor[] = {0, 1, 1, 0};
    int xFactor[] = {0, 0, 1, 1};

    // If leaf node, add block to pixel matrix and return
    if (root->type) {
        int i, j;
        for (i = x; i < x + size; i++) {
            for (j = y; j < y + size; j++) {
                pixels[i][j] = root->values;
            }
        }

        return;
    }

    // Call getImageUtil() for children
    int i;
    for (i = 0; i < 4; i++) {
        int newX = x + size / 2 * xFactor[i];
        int newY = y + size / 2 * yFactor[i];

        getImageUtil(root->children[i], size / 2, newX, newY, pixels);
    }
}

// Decompress image and get corresponding pixel matrix
void getImage(QuadTreeNode *root, unsigned int size, Pixel ***pixels) {

    // Allocate memory for pixel matrix
    *pixels = malloc(size * sizeof(Pixel *));
    if (!*pixels) {
        fprintf(stderr, "Error: Insufficient memory. Terminating program\n");
        exit(-1);
    }

    // Allocate memory for pixels[i]
    int i;
    for (i = 0; i < size; i++) {
        (*pixels)[i] = malloc(size * sizeof(Pixel));
        if (!(*pixels)[i]) {
            fprintf(stderr,
                    "Error: Insufficient memory. Terminating program\n");
            exit(-1);
        }
    }

    getImageUtil(root, (int)size, 0, 0, *pixels);
}

// Write image to .ppm file
void writeImage(Pixel **pixels, unsigned int size, char *fileName) {

    // Open .ppm file for writing text data
    FILE *outputFile = fopen(fileName, "w");
    if (!outputFile) {
        fprintf(stderr, "Error: Failed to open file. Terminating program\n");
        exit(-1);
    }

    // Print text data
    fprintf(outputFile, "P6\n%u %u\n255\n", size, size);

    // Re-open .ppm file for appending binary data
    fclose(outputFile);
    outputFile = fopen(fileName, "ab");

    // Write binary data
    int i;
    for (i = 0; i < size; i++) {
        fwrite(pixels[i], sizeof(Pixel), size, outputFile);
    }

    fclose(outputFile);
}

// Free pixel matrix
void freePixelMatrix(int height, Pixel **pixels) {
    int i;
    for (i = 0; i < height; i++)
        free(pixels[i]);
    free(pixels);
}

// Free quad tree
void freeQuadTree(QuadTreeNode *root) {
    int i;
    if (!root->type)
        for (i = 0; i < 4; i++)
            freeQuadTree(root->children[i]);

    free(root);
}

int main(int argc, char *argv[]) {

    // Check if program was called right (4-5 arguments)
    if (argc < 4 || argc > 6) {
        fprintf(stderr, "./quadtree [-c1 factor | -c2 factor | -d]");
        fprintf(stderr, " [input_file] [output_file]\n");
        return -1;
    }

    // Get flag
    int flag = getFlag(argv[1]);

    // Check if flag is valid
    if (flag == FLAG_ERROR) {
        fprintf(stderr, "./quadtree [-c1 factor | -c2 factor | -d]");
        fprintf(stderr, " [input_file] [output_file]\n");
        return -1;
    }

    // File names
    char *inputFileName = argv[argc - 2];
    char *outputFileName = argv[argc - 1];

    // Get provided factor (if format is invalid, 0 value is used)
    unsigned int factor;
    if (flag != FLAG_D)
        factor = atoi(argv[2]);

    switch (flag) {
        case FLAG_C1: {
            int width, height;
            Pixel **pixels;
            QuadTreeNode *root;

            // Read image
            if (readImage(&width, &height, &pixels, inputFileName))
                return -1;

            // Get quad tree
            if (getQuadTree(width, pixels, &root, factor))
                return -1;

            // Get required data
            int treeHeight = getHeight(root);
            int lowestLevel = getLowestLevel(root);
            int blocksCnt = getBlocksCnt(root);
            int highestBlockSize = width;
            while (--lowestLevel)
                highestBlockSize /= 2;

            // Open output file
            FILE *outputFile = fopen(outputFileName, "w");
            if (!outputFile) {
                fprintf(stderr, "Error: Failed to open file\n");
                return -1;
            }

            // Write data to output file
            fprintf(outputFile, "%d\n%d\n%d\n", treeHeight, blocksCnt,
                    highestBlockSize);

            freeQuadTree(root);
            freePixelMatrix(height, pixels);
            fclose(outputFile);

            break;
        }
        case FLAG_C2: {
            int width, height;
            Pixel **pixels;
            QuadTreeNode *root;

            // Read image
            if (readImage(&width, &height, &pixels, inputFileName))
                return -1;

            // Get quad tree
            if (getQuadTree(width, pixels, &root, factor))
                return -1;

            // Write quad tree to output file
            writeQuadTree(root, width, outputFileName);

            freeQuadTree(root);
            freePixelMatrix(height, pixels);

            break;
        }
        case FLAG_D: {
            unsigned int size;
            Pixel **pixels;
            QuadTreeNode *root;

            // Read quad tree
            readQuadTree(&root, &size, inputFileName);

            // Get image
            getImage(root, size, &pixels);

            // Write image to .ppm file
            writeImage(pixels, size, outputFileName);

            freeQuadTree(root);
            freePixelMatrix((int) size, pixels);

            break;
        }
    }

    return 0;
}
