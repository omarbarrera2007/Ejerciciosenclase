#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL.h>

#define FILAS 15
#define COLUMNAS 21
#define TAM 32
#define ALTO_INFO 50
#define WIDTH (COLUMNAS * TAM)
#define HEIGHT (FILAS * TAM + ALTO_INFO)
#define MAX_FANTASMAS 4

char mapa[FILAS][COLUMNAS + 1];

int jugadorFila = 1;
int jugadorCol = 1;
int inicioJugadorFila = 1;
int inicioJugadorCol = 1;

int fantasmaFila[MAX_FANTASMAS];
int fantasmaCol[MAX_FANTASMAS];
int fantasmaDir[MAX_FANTASMAS];
int inicioFantasmaFila[MAX_FANTASMAS];
int inicioFantasmaCol[MAX_FANTASMAS];
int totalFantasmas = 0;

int puntos = 0;
int vidas = 3;
int pellets = 0;
bool running = true;
bool victoria = false;

/* Este mapa se usa solo si no se encuentra el archivo mapa.txt. */
char mapaDefault[FILAS][COLUMNAS + 1] = {
    "#####################",
    "#P........#........F#",
    "#.###.###.#.###.###.#",
    "#...................#",
    "#.###.#.#####.#.###.#",
    "#.....#...#...#.....#",
    "#####.### # ###.#####",
    "#.........F.........#",
    "#####.### # ###.#####",
    "#.....#...#...#.....#",
    "#.###.#.#####.#.###.#",
    "#...................#",
    "#.###.###.#.###.###.#",
    "#F........#........F#",
    "#####################"
};

void cargarMapa(void);
void buscarPersonajes(void);
void moverJugador(int cambioFila, int cambioCol);
void moverFantasmas(void);
void revisarChoqueConFantasmas(void);
void reiniciarPosiciones(void);
void guardarPuntaje(void);
void dibujarTextoSimple(SDL_Renderer *renderer, int x, int y, int numero);
void renderizar(SDL_Renderer *renderer);

int main(int argc, char **argv)
{
    srand((unsigned int)time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Error al iniciar SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Pac-Man simple",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH,
        HEIGHT,
        0
    );

    if (window == NULL) {
        printf("Error al crear ventana: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    if (renderer == NULL) {
        printf("Error al crear renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    cargarMapa();
    buscarPersonajes();

    SDL_Event event;
    unsigned int ultimoMovimientoFantasma = 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_x:
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                    case SDLK_UP:
                        moverJugador(-1, 0);
                        break;
                    case SDLK_DOWN:
                        moverJugador(1, 0);
                        break;
                    case SDLK_LEFT:
                        moverJugador(0, -1);
                        break;
                    case SDLK_RIGHT:
                        moverJugador(0, 1);
                        break;
                }
            }
        }

        /* Los fantasmas se mueven mas lento que el jugador. */
        if (SDL_GetTicks() > ultimoMovimientoFantasma + 300) {
            moverFantasmas();
            ultimoMovimientoFantasma = SDL_GetTicks();
        }

        revisarChoqueConFantasmas();

        if (vidas <= 0 || pellets <= 0) {
            victoria = (pellets <= 0);
            running = false;
        }

        renderizar(renderer);
        SDL_Delay(16);
    }

    guardarPuntaje();

    printf("Juego terminado.\n");
    if (victoria) {
        printf("Ganaste.\n");
    } else {
        printf("Perdiste.\n");
    }
    printf("Puntaje final: %d\n", puntos);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void cargarMapa(void)
{
    FILE *archivo = fopen("mapa.txt", "r");
    int fila;
    char linea[100];

    if (archivo == NULL) {
        printf("No se encontro mapa.txt, se usara un mapa por defecto.\n");
        for (fila = 0; fila < FILAS; fila++) {
            int col;
            for (col = 0; col < COLUMNAS; col++) {
                mapa[fila][col] = mapaDefault[fila][col];
            }
            mapa[fila][COLUMNAS] = '\0';
        }
        return;
    }

    for (fila = 0; fila < FILAS; fila++) {
        int col;

        if (fgets(linea, sizeof(linea), archivo) == NULL) {
            linea[0] = '\0';
        }

        for (col = 0; col < COLUMNAS; col++) {
            if (linea[col] == '\n' || linea[col] == '\r' || linea[col] == '\0') {
                mapa[fila][col] = ' ';
            } else {
                mapa[fila][col] = linea[col];
            }
        }

        mapa[fila][COLUMNAS] = '\0';
            }

    fclose(archivo);
}

void buscarPersonajes(void)
{
    int fila, col;
    totalFantasmas = 0;
    pellets = 0;

    for (fila = 0; fila < FILAS; fila++) {
        for (col = 0; col < COLUMNAS; col++) {
            if (mapa[fila][col] == 'P') {
                jugadorFila = fila;
                jugadorCol = col;
                inicioJugadorFila = fila;
                inicioJugadorCol = col;
                mapa[fila][col] = ' ';
            } else if (mapa[fila][col] == 'F' && totalFantasmas < MAX_FANTASMAS) {
                fantasmaFila[totalFantasmas] = fila;
                fantasmaCol[totalFantasmas] = col;
                inicioFantasmaFila[totalFantasmas] = fila;
                inicioFantasmaCol[totalFantasmas] = col;
                fantasmaDir[totalFantasmas] = rand() % 4;
                totalFantasmas++;
                mapa[fila][col] = ' ';
            } else if (mapa[fila][col] == '.') {
                pellets++;
            }
        }
    }
}

void moverJugador(int cambioFila, int cambioCol)
{
    int nuevaFila = jugadorFila + cambioFila;
    int nuevaCol = jugadorCol + cambioCol;

    if (nuevaFila < 0 || nuevaFila >= FILAS || nuevaCol < 0 || nuevaCol >= COLUMNAS) {
        return;
    }

    if (mapa[nuevaFila][nuevaCol] == '#') {
        return;
    }

    jugadorFila = nuevaFila;
    jugadorCol = nuevaCol;

    if (mapa[jugadorFila][jugadorCol] == '.') {
        puntos += 10;
        pellets--;
        mapa[jugadorFila][jugadorCol] = ' ';
    }
}

void moverFantasmas(void)
{
    int i;

    for (i = 0; i < totalFantasmas; i++) {
        int intento;
        int nuevaFila = fantasmaFila[i];
        int nuevaCol = fantasmaCol[i];

        /* A veces persigue al jugador y a veces cambia al azar. */
        if (rand() % 10 < 7) {
            if (jugadorCol < fantasmaCol[i]) {
                fantasmaDir[i] = 2;
            } else if (jugadorCol > fantasmaCol[i]) {
                fantasmaDir[i] = 3;
            } else if (jugadorFila < fantasmaFila[i]) {
                fantasmaDir[i] = 0;
            } else if (jugadorFila > fantasmaFila[i]) {
                fantasmaDir[i] = 1;
            }
        } else {
            fantasmaDir[i] = rand() % 4;
        }

        for (intento = 0; intento < 4; intento++) {
            nuevaFila = fantasmaFila[i];
            nuevaCol = fantasmaCol[i];

            if (fantasmaDir[i] == 0) {
                nuevaFila--;
            } else if (fantasmaDir[i] == 1) {
                nuevaFila++;
            } else if (fantasmaDir[i] == 2) {
                nuevaCol--;
            } else if (fantasmaDir[i] == 3) {
                nuevaCol++;
            }

            if (nuevaFila >= 0 && nuevaFila < FILAS &&
                nuevaCol >= 0 && nuevaCol < COLUMNAS &&
                mapa[nuevaFila][nuevaCol] != '#') {
                fantasmaFila[i] = nuevaFila;
                fantasmaCol[i] = nuevaCol;
                break;
            }
    
            fantasmaDir[i] = rand() % 4;
        }
    }
}

void revisarChoqueConFantasmas(void)
{
    int i;

    for (i = 0; i < totalFantasmas; i++) {
        if (jugadorFila == fantasmaFila[i] && jugadorCol == fantasmaCol[i]) {
            vidas--;
            printf("Te atrapo un fantasma. Vidas: %d\n", vidas);
            reiniciarPosiciones();
            SDL_Delay(500);
            return;
        }
    }
}

void reiniciarPosiciones(void)
{
    int i;

    jugadorFila = inicioJugadorFila;
    jugadorCol = inicioJugadorCol;

    for (i = 0; i < totalFantasmas; i++) {
        fantasmaFila[i] = inicioFantasmaFila[i];
        fantasmaCol[i] = inicioFantasmaCol[i];
    }
}

void guardarPuntaje(void)
{
    FILE *archivo = fopen("puntajes.txt", "a");

    if (archivo == NULL) {
        printf("No se pudo guardar el puntaje.\n");
        return;
    }

    fprintf(archivo, "Puntaje: %d - Vidas restantes: %d\n", puntos, vidas);
    fclose(archivo);
}

void dibujarTextoSimple(SDL_Renderer *renderer, int x, int y, int numero)
{
    /* Numeros muy simples hechos con rectangulos para no usar SDL_ttf. */
    SDL_Rect r = {x, y, 8, 18};
    int i;

    for (i = 0; i < numero; i++) {
        r.x = x + i * 12;
        SDL_RenderFillRect(renderer, &r);
    }
}

void renderizar(SDL_Renderer *renderer)
{
    int fila, col, i;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (fila = 0; fila < FILAS; fila++) {
        for (col = 0; col < COLUMNAS; col++) {
            SDL_Rect cuadro = {col * TAM, fila * TAM + ALTO_INFO, TAM, TAM};

            if (mapa[fila][col] == '#') {
                SDL_SetRenderDrawColor(renderer, 0, 60, 200, 255);
                SDL_RenderFillRect(renderer, &cuadro);
            } else {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderFillRect(renderer, &cuadro);
            }

            if (mapa[fila][col] == '.') {
                SDL_Rect pellet = {
                    col * TAM + TAM / 2 - 4,
                    fila * TAM + ALTO_INFO + TAM / 2 - 4,
                    8,
                    8
                };
                SDL_SetRenderDrawColor(renderer, 255, 220, 120, 255);
                SDL_RenderFillRect(renderer, &pellet);
            }
        }
    }

    SDL_Rect jugador = {
        jugadorCol * TAM + 4,
        jugadorFila * TAM + ALTO_INFO + 4,
        TAM - 8,
        TAM - 8
    };
        SDL_SetRenderDrawColor(renderer, 255, 60, 90, 255);
        SDL_RenderFillRect(renderer, &fantasma);
    }

    /* Barra superior: puntos con bloques verdes y vidas con bloques rojos. */
    SDL_SetRenderDrawColor(renderer, 40, 200, 80, 255);
    dibujarTextoSimple(renderer, 15, 15, puntos / 10);

    SDL_SetRenderDrawColor(renderer, 220, 40, 40, 255);
    dibujarTextoSimple(renderer, WIDTH - 90, 15, vidas);

    SDL_RenderPresent(renderer);
}
