#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <SDL2/SDL_image.h>

#define WIDTH 800
#define HEIGHT 600

int posX, posY;
int speed =5;

SDL_Rect walkDown[] = {
    { 0, 12, 18, 37},
    {22, 11, 19, 38},
    {45, 11, 18, 38},
};

SDL_Rect walkHorizontal[] = {
    { 1, 55, 18, 37},
    {22, 56, 17, 36},
    {43, 55, 18, 37},
    {65, 56, 18, 36},
    {88, 56, 18, 36},

};

SDL_Rect walkUp[] = {
    { 1, 99, 19, 36},
    {24, 99, 19, 36},
    {47, 98, 21, 37},
    {72, 98, 21, 37},
};

SDL_Rect idle[] = {
    {68, 13, 22, 36},
    {94, 11, 22, 36},    
};


int main(int argc, char **argv)
{
    if(SDL_Init(SDL_INIT_VIDEO)!=0)
    {
        printf("Error: %s\n", SDL_GetError());
        return 1;
    }

    if(!(IMG_Init(IMG_INIT_PNG)&IMG_INIT_PNG)){
        printf("Error: %s\n", IMG_GetError());
        SDL_Quit();        
        return 1;
    }

    bool running = true;
    SDL_Event event;

    SDL_Window *window = SDL_CreateWindow(
        "HOLA MUNDO", // TITULO DE LA VENTA
        SDL_WINDOWPOS_CENTERED, // POSICIÓN DE LA VENTANA EN X
        SDL_WINDOWPOS_CENTERED, // POSICIÓN DE LA VENTANA EN Y
        WIDTH, // ANCHO DE LA VENTANA
        HEIGHT, // ALTO DE LA VENTANA
        0
    );

    if(window == NULL)
    {
        printf("Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_RaiseWindow(window); 

    

    SDL_Renderer *renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    if(renderer == NULL)
    {
        printf("Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture *texture = IMG_LoadTexture(renderer, "moon.png");

    if(texture==NULL){
        printf("Error: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    posX = (WIDTH/2)-25;
    posY = (HEIGHT/2)-25;

    int frame = 0;
    int direction =0;
    // Tamaño de cada sprite
    int frameWidth = 20;
    int frameHeight = 40;
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    unsigned int lastFrameTime = 0;

    while(running)
    {
        while(SDL_PollEvent(&event))
        {
            
            if(event.type == SDL_QUIT){
                running = false;
            }else if(event.type == SDL_KEYDOWN){
                switch (event.key.keysym.sym)
                {
                case SDLK_x:
                    running = false;
                    break;                
                default:
                    break;
                }     
            }
        }

        const unsigned char *keyboard = SDL_GetKeyboardState(NULL);

        bool moving = false;

        // Movimiento
        if(keyboard[SDL_SCANCODE_RIGHT]){
            posX += speed;
            moving = true;
            flip = SDL_FLIP_HORIZONTAL;
            direction = 0;
        }

        if(keyboard[SDL_SCANCODE_LEFT]){
            posX -= speed;
            moving = true;
            flip = SDL_FLIP_NONE;
            direction = 0;
        }

        if(keyboard[SDL_SCANCODE_UP]){
            posY -= speed;
            moving = true;
            direction = 1;
        }

        if(keyboard[SDL_SCANCODE_DOWN]){
            posY += speed;
            moving = true;
            direction = 2;
        }

        // Tiempo actual
        unsigned int currentTime = SDL_GetTicks();

        // Cambiar frame cada 150 ms
        if(currentTime > lastFrameTime + 500 && !moving){            
            frame++;
            lastFrameTime = currentTime;
        }else if(currentTime > lastFrameTime + 150 && moving){
            frame++;
            lastFrameTime = currentTime;
        }
        // Elegir animación
        SDL_Rect src;

        if(moving){
            switch (direction)
            {
            case 0:                
                src = walkHorizontal[frame%5];    
                break;

            case 1:                
                src = walkUp[frame%4];    
                break;

            case 2:                
                src = walkDown[frame%3];    
                break;           
            
            }
            
        }else{            
            src = idle[frame%2];
        }

        // Dónde dibujar
        SDL_Rect dest = {
            posX,
            posY,
            frameWidth * 2,
            frameHeight * 2
        };

        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        SDL_RenderClear(renderer);
        
        SDL_RenderCopyEx(
            renderer,
            texture,
            &src,
            &dest,
            0,
            NULL,
            flip
        );
        
        SDL_RenderPresent(renderer);

        SDL_Delay(16);

    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}