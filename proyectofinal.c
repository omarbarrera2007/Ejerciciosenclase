#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define FILAS 15
#define COLUMNAS 21
#define TAM 32
#define WIDTH (COLUMNAS * TAM)
#define HEIGHT (FILAS * TAM)
#define MAX_FANTASMAS 4
#define TOTAL_NIVELES 3

char mapa[FILAS][COLUMNAS + 1];
int jugadorFila, jugadorCol, inicioJugadorFila, inicioJugadorCol;
int velFila = 0;
int velCol = 0;
int fantasmaFila[MAX_FANTASMAS], fantasmaCol[MAX_FANTASMAS];
int inicioFantasmaFila[MAX_FANTASMAS], inicioFantasmaCol[MAX_FANTASMAS];
int fantasmaDir[MAX_FANTASMAS], totalFantasmas;
int dirFantasma[MAX_FANTASMAS];
int puntos = 0, vidas = 3, pellets = 0, nivel = 1;

int dirJugador = 0;
int framePikachu = 0;
int frameGengar = 0;

Uint32 ultimoFrame = 0;

bool jugando = true, victoria = false;
SDL_Texture *pikachuTexture = NULL;
SDL_Texture *gengarTexture = NULL;

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
                dirFantasma[totalFantasmas] = 0;
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

    if(df == -1) dirJugador = 3;
    if(df == 1)  dirJugador = 0;
    if(dc == -1) dirJugador = 1;
    if(dc == 1)  dirJugador = 2;

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

            if (fantasmaDir[i] == 0) dirFantasma[i] = 3;
            if (fantasmaDir[i] == 1) dirFantasma[i] = 0;
            if (fantasmaDir[i] == 2) dirFantasma[i] = 1;
            if (fantasmaDir[i] == 3) dirFantasma[i] = 2;

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

   SDL_Rect pikachuDestino = {
    jugadorCol * TAM-8,
    jugadorFila * TAM-8,
    48,
    48
};

SDL_Rect pikachuFrame = {
    framePikachu * 64, dirJugador * 64, 64, 64
};


SDL_RenderCopy(renderer,
               pikachuTexture,
               &pikachuFrame,
               &pikachuDestino);

    for (i = 0; i < totalFantasmas; i++) {

    SDL_Rect gengarDestino = {
        fantasmaCol[i] * TAM-8,
        fantasmaFila[i] * TAM-8,
        48,
        48
    };

    SDL_Rect gengarFrame = {
    frameGengar * 48, dirFantasma[i] * 48, 48, 48
};


    SDL_RenderCopy(renderer,
                   gengarTexture,
                   &gengarFrame,
                   &gengarDestino);
}

    SDL_RenderPresent(renderer);
}

void actualizarTitulo(SDL_Window *window)
{
    char titulo[120];
    sprintf(titulo, "Pac-Man | Nivel: %d/3 | Vidas: %d | Puntos: %d", nivel, vidas, puntos);
    SDL_SetWindowTitle(window, titulo);
}
Uint32 ultimoMovimientoJugador = 0;
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

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
    printf("Error SDL_image: %s\n", IMG_GetError());
    return 1;
}

    window = SDL_CreateWindow("Pac-Man", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Surface *temp;

temp = IMG_Load("pikachu.png");

if (temp == NULL) {
    printf("Error cargando pikachu.png: %s\n", IMG_GetError());
} else {
    pikachuTexture = SDL_CreateTextureFromSurface(renderer, temp);
    SDL_FreeSurface(temp);
}

temp = IMG_Load("gengar.png");

if (temp == NULL) {
    printf("Error cargando gengar.png: %s\n", IMG_GetError());
} else {
    gengarTexture = SDL_CreateTextureFromSurface(renderer, temp);
    SDL_FreeSurface(temp);
}

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
                if (event.key.keysym.sym == SDLK_UP) {
                        velFila = -1;
                        velCol = 0;
                }

                if (event.key.keysym.sym == SDLK_DOWN) {
                        velFila = 1;
                        velCol = 0;
                }

                if (event.key.keysym.sym == SDLK_LEFT) {
                            velFila = 0;
                        velCol = -1;
                }

            if (event.key.keysym.sym == SDLK_RIGHT) {
                        velFila = 0;
                        velCol = 1;
            }   
            }
        }
if(SDL_GetTicks() - ultimoFrame > 120)
{
    framePikachu = (framePikachu + 1) % 4;
    frameGengar = (frameGengar + 1) % 3;

    ultimoFrame = SDL_GetTicks();
}
        if (SDL_GetTicks() > ultimoFantasma + 300) {
            moverFantasmas();
            ultimoFantasma = SDL_GetTicks();
        }

        if(SDL_GetTicks() - ultimoMovimientoJugador > 180)
{
    moverJugador(velFila, velCol);
    ultimoMovimientoJugador = SDL_GetTicks();
}

        revisarColisiones();

        if (vidas <= 0)
{
    SDL_ShowSimpleMessageBox(
        SDL_MESSAGEBOX_ERROR,
        "CHEPAMON",
        "Has perdido todas tus vidas.",
        window
    );

    jugando = false;

        } else if (pellets <= 0) {
            if (nivel < TOTAL_NIVELES) {
                nivel++;
                cargarMapa();
                buscarElementos();
                SDL_Delay(500);
            } else {
                victoria = true;

SDL_ShowSimpleMessageBox(
    SDL_MESSAGEBOX_INFORMATION,
    "CHEPAMON",
    "Felicidades, completaste todos los niveles.",
    window
);

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

    SDL_DestroyTexture(pikachuTexture);
    SDL_DestroyTexture(gengarTexture);

    IMG_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}