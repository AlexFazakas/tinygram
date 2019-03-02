#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <math.h>

#define GRAYSCALE 53

/* @author: Alexandru Fazakas */

/* Structura de imagine pe care am folosit-o */

typedef struct {
    char type[4];
    int width;
    int height;
    int max;
    unsigned char* grid;
} image;

/* "Matricele" folosite pentru implementarea filtrelor */

float smooth_matrix[] = {1.0 / 9, 1.0 / 9, 1.0 / 9,
                         1.0 / 9, 1.0 / 9, 1.0 / 9,
                         1.0 / 9, 1.0 / 9, 1.0 / 9};
float blur_matrix[] = {1.0 / 16, 2.0 / 16, 1.0 / 16,
                       2.0 / 16, 4.0 / 16, 2.0 / 16,
                       1.0 / 16, 2.0 / 16, 1.0 / 16};
float sharpen_matrix[] = {0, -2.0 / 3, 0,
                          -2.0 / 3, 11.0 / 3, -2.0 / 3,
                          0, -2.0 / 3, 0};
float mean_matrix[] = {-1.0, -1.0, -1.0,
                       -1.0, 9.0, -1.0,
                       -1.0, -1.0, -1.0};
float emboss_matrix[] = {0, 1.0, 0,
                         0, 0, 0,
                         0, -1.0, 0};

/* Functie pentru citirea input-ului in functie de tipul imaginii
 * Aceasta functie este apelata de fiecare proces, citind in paralel imaginea
 */

void read_input(char *file_name, image *initial_image) {


    FILE *input_file = fopen(file_name, "rb");

    fscanf(input_file, "%s", initial_image->type);
    fscanf(input_file, "%i%i%i\n", &initial_image->width,
                                   &initial_image->height,
                                   &initial_image->max);

    if (initial_image->type[1] == GRAYSCALE) {
        initial_image->grid = malloc(initial_image->width *
                                     initial_image->height *
                                     sizeof(unsigned char));
    fread(initial_image->grid,
          sizeof(char),
          initial_image->width * initial_image->height,
          input_file);
    } else {
        initial_image->grid = malloc(3 * initial_image->width *
                                     initial_image->height *
                                     sizeof(unsigned char));
        fread(initial_image->grid,
              sizeof(char),
              3 * initial_image->width * initial_image->height,
              input_file);
    }
    fclose(input_file);
}

/* Functie care se ocupa de scrierea imaginii rezultate intr-un fisier dat
 * ca argument. Aceasta este apelata doar de procesul cu rank 0.
 */

void write_output(char *file_name, image *image) {
  FILE *output_file = fopen(file_name, "wb");

  fprintf(output_file, "%s\n", image->type);
  fprintf(output_file, "%d %d\n", image->width, image->height);
  fprintf(output_file, "%d\n", image->max);

  if (image->type[1] == GRAYSCALE) {
    fwrite(image->grid,
           sizeof(unsigned char),
           image->width * image->height,
           output_file);
  } else {
    fwrite(image->grid,
           sizeof(unsigned char),
           3 * image->width * image->height,
           output_file);
  }

  free(image->grid);
  free(image);
  fclose(output_file);
}

/* O functie simpla care intoarce un id numeric corespunzator fiecarui filtru */

int get_filter_id(char *filter) {
    if (strcmp(filter, "smooth") == 0) {
        return 0;
    }
    if (strcmp(filter, "blur") == 0) {
        return 1;
    }
    if (strcmp(filter, "sharpen") == 0) {
        return 2;
    }
    if (strcmp(filter, "mean") == 0) {
        return 3;
    }
    if (strcmp(filter, "emboss") == 0) {
        return 4;
    }
    return -1;
}

/* Filter_value calculeaza valoarea noului pixel de care avem nevoie in functie
 * de punctul curent, tipul imaginii si id-ul filtrului pe care il folosim. */

unsigned char filter_value(image *initial_image, int row, int col, int filter_id) {
    float result = 0;

    int width = initial_image->type[1] == GRAYSCALE ? initial_image->width :
                                                      3 * initial_image->width;
    int offset = initial_image->type[1] == GRAYSCALE ? 1 : 3;

    /* Width si offset sunt folositi pentru a usura inmultirea celor doua matrice
     * reprezentate sub forma de vectori. */

    switch(filter_id) {
        case 0:
                result += smooth_matrix[0] *
                            initial_image->grid[(row - 1) * width + col - offset];
                result += smooth_matrix[1] *
                            initial_image->grid[(row - 1) * width + col];
                result += smooth_matrix[2] *
                            initial_image->grid[(row - 1) * width + col + offset];
                result += smooth_matrix[3] *
                            initial_image->grid[row * width + col - offset];
                result += smooth_matrix[4] *
                            initial_image->grid[row * width + col];
                result += smooth_matrix[5] *
                            initial_image->grid[row * width + col + offset];
                result += smooth_matrix[6] *
                            initial_image->grid[(row + 1) * width + col - offset];
                result += smooth_matrix[7] *
                            initial_image->grid[(row + 1) * width + col];
                result += smooth_matrix[8] *
                            initial_image->grid[(row + 1) * width + col + offset];
            break;
        case 1:
                result += blur_matrix[0] *
                            initial_image->grid[(row - 1) * width + col - offset];
                result += blur_matrix[1] *
                            initial_image->grid[(row - 1) * width + col];
                result += blur_matrix[2] *
                            initial_image->grid[(row - 1) * width + col + offset];
                result += blur_matrix[3] *
                            initial_image->grid[row * width + col - offset];
                result += blur_matrix[4] *
                            initial_image->grid[row * width + col];
                result += blur_matrix[5] *
                            initial_image->grid[row * width + col + offset];
                result += blur_matrix[6] *
                            initial_image->grid[(row + 1) * width + col - offset];
                result += blur_matrix[7] *
                            initial_image->grid[(row + 1) * width + col];
                result += blur_matrix[8] *
                            initial_image->grid[(row + 1) * width + col + offset];
            break;
        case 2:
                result += sharpen_matrix[0] *
                            initial_image->grid[(row - 1) * width + col - offset];
                result += sharpen_matrix[1] *
                            initial_image->grid[(row - 1) * width + col];
                result += sharpen_matrix[2] *
                            initial_image->grid[(row - 1) * width + col + offset];
                result += sharpen_matrix[3] *
                            initial_image->grid[row * width + col - offset];
                result += sharpen_matrix[4] *
                            initial_image->grid[row * width + col];
                result += sharpen_matrix[5] *
                            initial_image->grid[row * width + col + offset];
                result += sharpen_matrix[6] *
                            initial_image->grid[(row + 1) * width + col - offset];
                result += sharpen_matrix[7] *
                            initial_image->grid[(row + 1) * width + col];
                result += sharpen_matrix[8] *
                            initial_image->grid[(row + 1) * width + col + offset];
            break;
        case 3:
                result += mean_matrix[0] *
                            initial_image->grid[(row - 1) * width + col - offset];
                result += mean_matrix[1] *
                            initial_image->grid[(row - 1) * width + col];
                result += mean_matrix[2] *
                            initial_image->grid[(row - 1) * width + col + offset];
                result += mean_matrix[3] *
                            initial_image->grid[row * width + col - offset];
                result += mean_matrix[4] *
                            initial_image->grid[row * width + col];
                result += mean_matrix[5] *
                            initial_image->grid[row * width + col + offset];
                result += mean_matrix[6] *
                            initial_image->grid[(row + 1) * width + col - offset];
                result += mean_matrix[7] *
                            initial_image->grid[(row + 1) * width + col];
                result += mean_matrix[8] *
                            initial_image->grid[(row + 1) * width + col + offset];
            break;
        case 4:
                result += emboss_matrix[0] *
                            initial_image->grid[(row - 1) * width + col - offset];
                result += emboss_matrix[1] *
                            initial_image->grid[(row - 1) * width + col];
                result += emboss_matrix[2] *
                            initial_image->grid[(row - 1) * width + col + offset];
                result += emboss_matrix[3] *
                            initial_image->grid[row * width + col - offset];
                result += emboss_matrix[4] *
                            initial_image->grid[row * width + col];
                result += emboss_matrix[5] *
                            initial_image->grid[row * width + col + offset];
                result += emboss_matrix[6] *
                            initial_image->grid[(row + 1) * width + col - offset];
                result += emboss_matrix[7] *
                            initial_image->grid[(row + 1) * width + col];
                result += emboss_matrix[8] *
                            initial_image->grid[(row + 1) * width + col + offset];
            break;
    }

    return (unsigned char) result;
}

/* Functie care aplica un filtru pe o imagine data in mod serial.
 * Foloseste o a doua imagine in care scrie rezultatele si din care le
 * copiaza la final pentru ca un punct calculat sa nu afecteze vecinii.
 */
void apply_filter_serial(image *initial_image, char *filter) {
    int filter_id = get_filter_id(filter);
    int height = initial_image->height;
    int width = initial_image->type[1] == GRAYSCALE ? initial_image->width :
                                                      3 * initial_image->width;
    int offset = initial_image->type[1] == GRAYSCALE ?  1 : 3;
    image *new_image = malloc(sizeof(image));
    new_image->grid = malloc(height * width);
    
    for (int row = 1; row < height - 1; ++row) {
        for (int col = offset; col < width - offset; ++col) {
            int current_position = row * width + col;

            new_image->grid[current_position] = filter_value(initial_image,
                                                             row,
                                                             col,
                                                             filter_id);
            }
        }

    for (int row = 1; row < height - 1; ++row) {
        for (int col = offset; col < width - offset; ++col) {
            int current_position = row * width + col;

            initial_image->grid[current_position] =
                                new_image->grid[current_position];
        }
    }

    free(new_image->grid);
    free(new_image);
}

/* Functie care aplica un filter in mod paralel. */

void apply_filter_parallel(image *initial_image,
                           char *filter,
                           int start[],
                           int end[],
                           int threads,
                           int rank) {
    int filter_id = get_filter_id(filter);
    int height = initial_image->height;
    int width = initial_image->type[1] == GRAYSCALE ? initial_image->width :
                                                      3 * initial_image->width;
    int offset = initial_image->type[1] == GRAYSCALE ?  1 : 3;

    /* Inainte de a aplica filtrele, avem nevoie de niste informatii
     * de la celelalte procese.
     * Primul proces va avea nevoie de prima linie de la urmatorul proces,
     * ultimul proces va avea nevoie de ultima linie a procesului cu rank-ul
     * anterior.
     * Oricare alt proces are nevoie de ultima linie de la procesul anterior
     * si de prima linie de la urmatorul proces ca rank.
     */

    if (rank != 0) {
        MPI_Send(initial_image->grid + start[rank] * width,
                 width,
                 MPI_UNSIGNED_CHAR,
                 rank - 1,
                 0,
                 MPI_COMM_WORLD);
    }
    if (rank != threads - 1) {
        MPI_Recv(initial_image->grid + end[rank] * width,
                 width,
                 MPI_UNSIGNED_CHAR,
                 rank + 1,
                 0,
                 MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
    }
    if (rank != threads - 1) {
        MPI_Send(initial_image->grid + (end[rank] - 1) * width,
                 width,
                 MPI_UNSIGNED_CHAR,
                 rank + 1,
                 0,
                 MPI_COMM_WORLD);
    }
    if (rank != 0) {
        MPI_Recv(initial_image->grid + (start[rank] - 1) * width,
                 width,
                 MPI_UNSIGNED_CHAR,
                 rank - 1,
                 0, 
                 MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
    }

    image *new_image = malloc(sizeof(image));
    new_image->grid = malloc(height * width);
    
    for (int row = start[rank]; row < end[rank]; ++row) {
        for (int col = offset; col < width - offset; ++col) {
            int current_position = row * width + col;

            new_image->grid[current_position] = filter_value(initial_image,
                                                             row,
                                                             col,
                                                             filter_id);
            }
        }

    for (int row = start[rank]; row < end[rank]; ++row) {
        for (int col = offset; col < width - offset; ++col) {
            int current_position = row * width + col;

            initial_image->grid[current_position] =
                                new_image->grid[current_position];
        }
    }

    free(new_image->grid);
    free(new_image);

}

/* Functie care aplica toate filtrele peste o imagine, serial. */

void do_serial(image *initial_image, int argc, char **argv) {
    for (int i = 3; i < argc; ++i) {
        apply_filter_serial(initial_image, argv[i]);
    }
}

/* Functie care aplica toate filtrele peste imagine in mod paralel, apoi
 * compune imaginea la loc din fiecare bucata procesata de procese diferite.
 */

void do_parallel(image *initial_image,
                 int argc,
                 char **argv,
                 int start[],
                 int end[],
                 int threads,
                 int rank) {
    for (int i = 3; i < argc; ++i) {
        apply_filter_parallel(initial_image, argv[i], start, end, threads, rank);
    }
    int width = initial_image->type[1] == GRAYSCALE ? initial_image->width :
                                                      3 * initial_image->width;
    /* Strangem imaginea finala la procesul cu rank 0: */
    if (rank == 0) {
        for (int i = 1; i < threads; ++i) {
            MPI_Recv(initial_image->grid + start[i] * width,
                     (end[i] - start[i] + 1) * width,
                     MPI_UNSIGNED_CHAR,
                     i,
                     0,
                     MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);
        }
    /* Iar orice alt proces trimite partea procesata de el catre 0: */
    } else {
        MPI_Send(initial_image->grid + start[rank] * width,
                 (end[rank] - start[rank] + 1) * width,
                 MPI_UNSIGNED_CHAR,
                 0,
                 0,
                 MPI_COMM_WORLD);
    }
}

int main(int argc, char **argv) {
    int rank, threads;
    image *initial_image = malloc (sizeof(image));

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &threads);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    read_input(argv[1], initial_image);

    if (threads == 1) { 
        do_serial(initial_image, argc, argv);
    } else {

/* Vectori in care am tinut minte linia de inceput si de final pentru partea
 * pe care o proceseaza procesul cu un anumit rank. */

        int start[threads];
        int end[threads];

/* Impartirea imaginii in functie de numarul de procese e similara cu formula
 * din laborator, cu mentiunea ca nu am mai folosit un minim pentru a fi
 * sigur ca este procesata intreaga imagine. */

        for (int i = 0; i < threads; ++i) {
            if (i == 0) {
                start[i] = 1;
            } else {
                start[i] = ceil((double) initial_image->height / threads) * i;
            }
            if (i == threads - 1) {
                end[i] = initial_image->height - 1;
            } else {
                end[i] = ceil((double) initial_image->height / threads) * (i + 1);
            }

        }
        do_parallel(initial_image, argc, argv, start, end, threads, rank);
    }

    if (rank == 0) {
        write_output(argv[2], initial_image);
    }

    MPI_Finalize();
    return 0;
}
