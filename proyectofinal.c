#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL.h>

#define FILAS 15
#define COLUMNAS 21
#define TAM 32
#define WIDTH (COLUMNAS * TAM)
#define HEIGHT (FILAS * TAM)
#define MAX_FANTASMAS 4
#define TOTAL_NIVELES 3

char mapa[FILAS][COLUMNAS + 1];
int jugadorFila, jugadorCol, inicioJugadorFila, inicioJugadorCol;
int fantasmaFila[MAX_FANTASMAS], fantasmaCol[MAX_FANTASMAS];
int inicioFantasmaFila[MAX_FANTASMAS], inicioFantasmaCol[MAX_FANTASMAS];
int fantasmaDir[MAX_FANTASMAS], totalFantasmas;
int puntos = 0, vidas = 3, pellets = 0, nivel = 1;
bool jugando = true, victoria = false;

char mapasDefault[TOTAL_NIVELES][FILAS][COLUMNAS + 1] = {
    {
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
    },
    {
        "#####################",
        "#P....#.......#....F#",
        "#.##.#.#.###.#.#.##.#",
        "#....#.........#....#",
        "####.###.#.#.###.####",
        "#......#.#.#.#......#",
        "#.####.#.....#.####.#",
        "#........F..........#",
        "#.####.#.....#.####.#",
        "#......#.#.#.#......#",
        "####.###.#.#.###.####",
        "#....#.........#....#",
        "#.##.#.#.###.#.#.##.#",
        "#F....#.......#....F#",
        "#####################"
    },
    {
        "#####################",
        "#P..#.......#......F#",
        "#.#.#.#####.#.####..#",
        "#.#...............#.#",
        "#.#####.###.#####.#.#",
        "#.....#..F..#.......#",
        "#####.#.###.#.#######",
        "#...................#",
        "#######.#.###.#.#####",
        "#.......#.....#.....#",
        "#.#.#####.###.#####.#",
        "#.#...............#.#",
        "#..####.#.#####.#.#.#",
        "#F......#.......#...#",
        "#####################"
    }
};

void cargarMapa(void);
void buscarElementos(void);
void reiniciarPosiciones(void);
void moverJugador(int df, int dc);
void moverFantasmas(void);
void revisarColisiones(void);
void guardarPuntaje(void);
void dibujarCirculo(SDL_Renderer *renderer, int cx, int cy, int radio);
void renderizar(SDL_Renderer *renderer);
void actualizarTitulo(SDL_Window *window);

void cargarMapa(void)
{
    FILE *archivo;
    char nombre[20], linea[100];
    int f, c;

    sprintf(nombre, "nivel%d.txt", nivel);
    archivo = fopen(nombre, "r");

    for (f = 0; f < FILAS; f++) {
        if (archivo != NULL && fgets(linea, sizeof(linea), archivo) != NULL) {
            for (c = 0; c < COLUMNAS; c++) {
                if (linea[c] == '\n' || linea[c] == '\r' || linea[c] == '\0') {
                    mapa[f][c] = ' ';
                } else {
                    mapa[f][c] = linea[c];
                }
            }
        } else {
            for (c = 0; c < COLUMNAS; c++) {
                mapa[f][c] = mapasDefault[nivel - 1][f][c];
            }
        }
        mapa[f][COLUMNAS] = '\0';
    }

    if (archivo != NULL) fclose(archivo);
}

void buscarElementos(void)
{
    int f, c;

    totalFantasmas = 0;
    pellets = 0;

    for (f = 0; f < FILAS; f++) {
        for (c = 0; c < COLUMNAS; c++) {
            if (mapa[f][c] == 'P') {
                jugadorFila = inicioJugadorFila = f;
                jugadorCol = inicioJugadorCol = c;
                mapa[f][c] = ' ';
            } else if (mapa[f][c] == 'F' && totalFantasmas < MAX_FANTASMAS) {
                fantasmaFila[totalFantasmas] = inicioFantasmaFila[totalFantasmas] = f;
                fantasmaCol[totalFantasmas] = inicioFantasmaCol[totalFantasmas] = c;
                fantasmaDir[totalFantasmas] = rand() % 4;
                totalFantasmas++;
                mapa[f][c] = ' ';
            } else if (mapa[f][c] == '.') {
                pellets++;
            }
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

void moverJugador(int df, int dc)
{
    int nf = jugadorFila + df;
    int nc = jugadorCol + dc;

    if (nf < 0 || nf >= FILAS || nc < 0 || nc >= COLUMNAS) return;
    if (mapa[nf][nc] == '#') return;

    jugadorFila = nf;
    jugadorCol = nc;

    if (mapa[nf][nc] == '.') {
        puntos += 10;
        pellets--;
        mapa[nf][nc] = ' ';
    }
}

void moverFantasmas(void)
{
    int i, intento;

    for (i = 0; i < totalFantasmas; i++) {
        if (rand() % 10 < 7) {
            if (jugadorCol < fantasmaCol[i]) fantasmaDir[i] = 2;
            else if (jugadorCol > fantasmaCol[i]) fantasmaDir[i] = 3;
            else if (jugadorFila < fantasmaFila[i]) fantasmaDir[i] = 0;
            else if (jugadorFila > fantasmaFila[i]) fantasmaDir[i] = 1;
        } else {
            fantasmaDir[i] = rand() % 4;
        }

        for (intento = 0; intento < 4; intento++) {
            int nf = fantasmaFila[i];
            int nc = fantasmaCol[i];

            if (fantasmaDir[i] == 0) nf--;
            if (fantasmaDir[i] == 1) nf++;
            if (fantasmaDir[i] == 2) nc--;
            if (fantasmaDir[i] == 3) nc++;

            if (nf >= 0 && nf < FILAS && nc >= 0 && nc < COLUMNAS && mapa[nf][nc] != '#') {
                fantasmaFila[i] = nf;
                fantasmaCol[i] = nc;
                break;
            }

            fantasmaDir[i] = rand() % 4;
        }
    }
}

void revisarColisiones(void)
{
    int i;

    for (i = 0; i < totalFantasmas; i++) {
        if (jugadorFila == fantasmaFila[i] && jugadorCol == fantasmaCol[i]) {
            vidas--;
            reiniciarPosiciones();
            SDL_Delay(400);
            return;
        }
    }
}

void guardarPuntaje(void)
{
    FILE *archivo = fopen("puntajes.txt", "a");

    if (archivo != NULL) {
        fprintf(archivo, "Puntaje: %d, nivel alcanzado: %d, vidas: %d\n", puntos, nivel, vidas);
        fclose(archivo);
    }
}

void dibujarCirculo(SDL_Renderer *renderer, int cx, int cy, int radio)
{
    int x, y;

    for (y = -radio; y <= radio; y++) {
        for (x = -radio; x <= radio; x++) {
            if (x * x + y * y <= radio * radio) {
                SDL_RenderDrawPoint(renderer, cx + x, cy + y);
            }
        }
    }
}

void renderizar(SDL_Renderer *renderer)
{
    int f, c, i;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (f = 0; f < FILAS; f++) {
        for (c = 0; c < COLUMNAS; c++) {
            SDL_Rect cuadro = {c * TAM, f * TAM, TAM, TAM};

            if (mapa[f][c] == '#') {
                SDL_SetRenderDrawColor(renderer, 0, 60, 200, 255);
                SDL_RenderFillRect(renderer, &cuadro);
            } else if (mapa[f][c] == '.') {
                SDL_Rect pellet = {c * TAM + 13, f * TAM + 13, 6, 6};
                SDL_SetRenderDrawColor(renderer, 255, 220, 120, 255);
                SDL_RenderFillRect(renderer, &pellet);
            }
        }
    }

    SDL_SetRenderDrawColor(renderer, 255, 230, 0, 255);
    dibujarCirculo(renderer, jugadorCol * TAM + TAM / 2, jugadorFila * TAM + TAM / 2, 12);

    SDL_SetRenderDrawColor(renderer, 255, 60, 90, 255);
    for (i = 0; i < totalFantasmas; i++) {
        SDL_Rect fantasma = {fantasmaCol[i] * TAM + 5, fantasmaFila[i] * TAM + 5, TAM - 10, TAM - 10};
        SDL_RenderFillRect(renderer, &fantasma);
    }

    SDL_RenderPresent(renderer);
}

void actualizarTitulo(SDL_Window *window)
{
    char titulo[120];
    sprintf(titulo, "Pac-Man | Nivel: %d/3 | Vidas: %d | Puntos: %d", nivel, vidas, puntos);
    SDL_SetWindowTitle(window, titulo);
}

int main(int argc, char **argv)
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;
    unsigned int ultimoFantasma = 0;

    srand((unsigned int)time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Error SDL: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Pac-Man", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (window == NULL || renderer == NULL) {
        printf("Error al crear ventana o renderer: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    cargarMapa();
    buscarElementos();

    while (jugando) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) jugando = false;

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_x) jugando = false;
                if (event.key.keysym.sym == SDLK_UP) moverJugador(-1, 0);
                if (event.key.keysym.sym == SDLK_DOWN) moverJugador(1, 0);
                if (event.key.keysym.sym == SDLK_LEFT) moverJugador(0, -1);
                if (event.key.keysym.sym == SDLK_RIGHT) moverJugador(0, 1);
            }
        }

        if (SDL_GetTicks() > ultimoFantasma + 300) {
            moverFantasmas();
            ultimoFantasma = SDL_GetTicks();
        }

        revisarColisiones();

        if (vidas <= 0) {
            jugando = false;
        } else if (pellets <= 0) {
            if (nivel < TOTAL_NIVELES) {
                nivel++;
                cargarMapa();
                buscarElementos();
                SDL_Delay(500);
            } else {
                victoria = true;
                jugando = false;
            }
        }

        actualizarTitulo(window);
        renderizar(renderer);
        SDL_Delay(16);
    }

    guardarPuntaje();

    if (victoria) {
        printf("Ganaste\n");
    } else {
        printf("Perdiste\n");
    }

    printf("Puntaje final: %d\n", puntos);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}