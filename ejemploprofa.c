#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <SDL2/SDL_image.h>

#define WIDTH 800
#define HEIGHT 600

int posX, posY;
int speed =10;
int frameWidth =22;
int frameHeight = 40;

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
    bool dirLeft = true;

    while(running)
    {
        while(SDL_PollEvent(&event))
        {
            dirLeft = true;
            if(event.type == SDL_QUIT){
                running = false;
            }else if(event.type == SDL_KEYDOWN){
                switch (event.key.keysym.sym)
                {
                case SDLK_x:
                    running = false;
                    break;

                case SDLK_UP:
                    direction= 2;
                    posY-= speed;
                    frame = (frame+1) % 4;
                    break;

                case SDLK_DOWN:
                    direction= 0;
                    posY += speed;
                    frame = (frame+1) % 3;
                    break;

                case SDLK_LEFT:
                    direction= 1;
                    posX-= speed;
                    frame = (frame+1) % 4;
                    break;

                case SDLK_RIGHT:
                    dirLeft = false;
                    direction = 1;
                    posX += speed;
                    frame = (frame+1) % 4;
                    break;
                
                default:
                    break;
                }     
                    
                
            }
        }

        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255,255,255, 255);

        int framePosX = frame*frameWidth;
        int framePosY = (direction!=2)?direction*frameHeight+10:direction*frameHeight+15;

        SDL_Rect src = { framePosX, framePosY, frameWidth, frameHeight};

        SDL_Rect dest = {posX, posY, 32, 48};

        if(!dirLeft){
            SDL_RenderCopyEx(
                renderer,
                texture,
                &src,
                &dest,
                0,
                NULL,
                SDL_FLIP_HORIZONTAL
            );
        }else{
            SDL_RenderCopy(
                renderer,
                texture,
                &src,
                &dest
            );
        }

        

        //SDL_RenderFillRect(renderer, &square);

        SDL_RenderPresent(renderer);

        SDL_Delay(16);

    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}
