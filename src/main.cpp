#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdbool.h>

// Dimensões da TV HD no PS3
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

// Estado do Pou
struct PouState {
    float hunger;   // 0 a 100
    float health;   // 0 a 100
    float fun;      // 0 a 100
    int coins;
};

// Posição do cursor do DualShock 3
float cursorX = SCREEN_WIDTH / 2.0f;
float cursorY = SCREEN_HEIGHT / 2.0f;
const float CURSOR_SPEED = 7.0f;

enum GameState { STATE_MENU, STATE_GAME };
GameState currentState = STATE_MENU;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) < 0) {
        printf("Erro ao inicializar SDL: %s\n", SDL_GetError());
        return 1;
    }

    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    SDL_Window* window = SDL_CreateWindow("Pou PS3 Edition",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Carrega suporte a Gamepad (DualShock 3 no PS3)
    SDL_GameController* controller = NULL;
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            controller = SDL_GameControllerOpen(i);
            break;
        }
    }

    PouState pou = { 80.0f, 100.0f, 60.0f, 150 };
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Leitura do Analógico do DualShock 3
        if (controller) {
            Sint16 axisX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
            Sint16 axisY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);

            // Deadzone para evitar drift
            if (abs(axisX) > 8000) cursorX += (axisX / 32767.0f) * CURSOR_SPEED;
            if (abs(axisY) > 8000) cursorY += (axisY / 32767.0f) * CURSOR_SPEED;

            // Limite da tela
            if (cursorX < 0) cursorX = 0;
            if (cursorX > SCREEN_WIDTH) cursorX = SCREEN_WIDTH;
            if (cursorY < 0) cursorY = 0;
            if (cursorY > SCREEN_HEIGHT) cursorY = SCREEN_HEIGHT;

            // Pressionar o botão CROSS (X) do PS3
            if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A)) {
                if (currentState == STATE_MENU) {
                    currentState = STATE_GAME;
                } else if (currentState == STATE_GAME) {
                    // Se clicar perto do Pou, alimenta o Pou
                    if (abs(cursorX - SCREEN_WIDTH/2) < 100 && abs(cursorY - SCREEN_HEIGHT/2) < 100) {
                        pou.hunger += 10.0f;
                        if (pou.hunger > 100.0f) pou.hunger = 100.0f;
                        pou.coins += 5;
                    }
                }
            }

            // Botão START sai do jogo
            if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START)) {
                running = false;
            }
        }

        // --- RENDERIZAÇÃO ---
        // Fundo estilo quarto do Pou (Amarelo suave)
        SDL_SetRenderDrawColor(renderer, 255, 243, 176, 255);
        SDL_RenderClear(renderer);

        if (currentState == STATE_MENU) {
            // Desenha Menu Principal em Português
            // Corpo do Pou no centro
            SDL_SetRenderDrawColor(renderer, 180, 120, 60, 255);
            SDL_Rect pouBody = { SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT/2 - 60, 160, 140 };
            SDL_RenderFillRect(renderer, &pouBody);

            // Olhos
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_Rect eyeL = { SCREEN_WIDTH/2 - 50, SCREEN_HEIGHT/2 - 30, 30, 40 };
            SDL_Rect eyeR = { SCREEN_WIDTH/2 + 20, SCREEN_HEIGHT/2 - 30, 30, 40 };
            SDL_RenderFillRect(renderer, &eyeL);
            SDL_RenderFillRect(renderer, &eyeR);
        } 
        else if (currentState == STATE_GAME) {
            // Desenha o Pou Interativo
            SDL_SetRenderDrawColor(renderer, 180, 120, 60, 255);
            SDL_Rect pouBody = { SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 - 80, 200, 180 };
            SDL_RenderFillRect(renderer, &pouBody);

            // Barra de Fome (Verde)
            SDL_SetRenderDrawColor(renderer, 50, 205, 50, 255);
            SDL_Rect barHunger = { 50, 40, (int)(pou.hunger * 2), 25 };
            SDL_RenderFillRect(renderer, &barHunger);
        }

        // Desenha o Cursor (Mãozinha/Círculo Vermelho guiado pelo analógico)
        SDL_SetRenderDrawColor(renderer, 235, 50, 50, 255);
        SDL_Rect cursorRect = { (int)cursorX - 10, (int)cursorY - 10, 20, 20 };
        SDL_RenderFillRect(renderer, &cursorRect);

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    if (controller) SDL_GameControllerClose(controller);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();

    return 0;
}
