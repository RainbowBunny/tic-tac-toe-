// SDL library
#include <SDL2/SDL.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_image.h>

// std library
#include <iostream>
#include <vector>
#include <ciso646>
using namespace std;

const int WINDOW_WIDTH  = 900;
const int WINDOW_HEIGHT = 900;
const int CELL_SIZE = 300;
const SDL_Color LINE_COLOR = {128, 128, 128};

enum {
    POTATO_IMAGE = 0,
    X_IMAGE,
    O_IMAGE,
    IMAGE_COUNT
};

SDL_Texture* picture[IMAGE_COUNT];

SDL_Window* gGlobalWindow;
SDL_Renderer* gGlobalRenderer; 

void logSDLError(const string &msg, bool fatal) {
    cout << msg << "Error: " << SDL_GetError() << endl;
    if (fatal) {
        SDL_Quit();
        exit(1);
    }
}

struct Position {
    int x, y;
    Position (int x = 0, int y = 0) : x(x), y(y) {}

    bool isInsideRect(int rectWidth, int rectHeight, int objectX, int objectY) {
        return objectX >= x and objectX < x + rectWidth and objectY >= y and objectY < y + rectHeight;
    }
};

void drawTexture(SDL_Renderer* renderer, SDL_Texture* texture, int cellX, int cellY, int cellWidth, int cellHeight) {
    SDL_Rect srcRect, dstRect;
    dstRect.x = cellX; dstRect.y = cellY; dstRect.w = cellWidth, dstRect.h = cellHeight;

    if (SDL_RenderCopy(renderer, texture, nullptr, &dstRect) < 0) {
        logSDLError("SDL_RenderCopy", true);
    }

}

struct Board {
    int height, width, currentPlayer, top, left;
    SDL_Window* gameWindow;
    SDL_Renderer* gameRenderer;

    vector <vector <Position> > board;
    vector <vector <int> > playedPosition;

    Board (SDL_Window* gameWindow, SDL_Renderer* gameRenderer, int height = 0, int width = 0, int top = 0, int left = 0) : height(height), width(width), gameWindow(gameWindow), gameRenderer(gameRenderer), top(top), left(left) {
        currentPlayer = X_IMAGE;
        board.assign(height, vector <Position> (width));
        playedPosition.assign(height, vector <int> (width));
    };

    void createBoard() {
        int windowX, windowY;
        SDL_GetWindowPosition(gameWindow, &windowX, &windowY);

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                board[i][j] = Position(windowX + CELL_SIZE * i, windowY + CELL_SIZE * j);
            } 
        }
    }

    void renderBoard() {
        int mouseX, mouseY;
        SDL_GetGlobalMouseState(&mouseX, &mouseY);

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                SDL_Texture* texture = nullptr;
                if (board[i][j].isInsideRect(CELL_SIZE, CELL_SIZE, mouseX, mouseY) and !playedPosition[i][j]) {
                    texture = picture[currentPlayer];
                } else if (!playedPosition[i][j]) {
                    texture = picture[POTATO_IMAGE];
                } else {
                    texture = picture[playedPosition[i][j]];
                }

                drawTexture(gameRenderer, texture, i * CELL_SIZE + 1, j * CELL_SIZE + 1, CELL_SIZE - 2, CELL_SIZE - 2);
            }
        }
    }

    void updateGameState() {
        int mouseX, mouseY;
        SDL_GetGlobalMouseState(&mouseX, &mouseY);

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (board[i][j].isInsideRect(CELL_SIZE, CELL_SIZE, mouseX, mouseY) and !playedPosition[i][j]) {
                    playedPosition[i][j] = currentPlayer;
                    currentPlayer = X_IMAGE + O_IMAGE - currentPlayer;
                    renderBoard();
                }
            }
        }
    }
};
 
void init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        logSDLError("SDL_Init", true);
    }

    int imflags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(imflags) & imflags) != imflags) {
        logSDLError("SDL_image", true);
    }
    
    gGlobalWindow = SDL_CreateWindow("Tic Tac Toe", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (gGlobalWindow == nullptr) {
        logSDLError("Cant create window", true);
    }

    gGlobalRenderer = SDL_CreateRenderer(gGlobalWindow, -1, SDL_WINDOW_SHOWN);
    if (gGlobalRenderer == nullptr) {
        logSDLError("Cant get renderer", true);
    }
}

SDL_Texture* loadTexture(string path) {
    SDL_Texture* texture = nullptr;
    SDL_Surface* surface = IMG_Load(path.c_str());

    if (surface == nullptr) {
        logSDLError("Can't load image: " + path, true);
        return nullptr;
    }

    texture = SDL_CreateTextureFromSurface(gGlobalRenderer, surface);

    if (texture == nullptr) {
        logSDLError("Can't create texture from image: " + path, true);
        return nullptr;
    }

    return texture;
}

void drawBoard() {
    for (int i = 0; i <= WINDOW_WIDTH; i += CELL_SIZE) {
        SDL_SetRenderDrawColor(gGlobalRenderer, LINE_COLOR.r, LINE_COLOR.g, LINE_COLOR.b, 0);
        SDL_RenderDrawLine(gGlobalRenderer, 0, i, WINDOW_HEIGHT, i);
    }

    for (int i = 0; i <= WINDOW_HEIGHT; i += CELL_SIZE) {
        SDL_SetRenderDrawColor(gGlobalRenderer, LINE_COLOR.r, LINE_COLOR.g, LINE_COLOR.b, 0);
        SDL_RenderDrawLine(gGlobalRenderer, i, 0, i, WINDOW_WIDTH);
    }
    SDL_RenderPresent(gGlobalRenderer);
}

void loadImage() {
    picture[POTATO_IMAGE] = loadTexture("potato.jpg");
    picture[X_IMAGE]      = loadTexture("letterX.png");
    picture[O_IMAGE]      = loadTexture("letterO.png");
}

int main() {
    init();
    loadImage();

    Board game(gGlobalWindow, gGlobalRenderer, WINDOW_HEIGHT / CELL_SIZE, WINDOW_WIDTH / CELL_SIZE, 0, 0);
    SDL_Event event;

    while (true) {
        game.createBoard();
        game.renderBoard();
        drawBoard();
        // SDL_GetGlobalMouseState(&x, &y);
        // SDL_GetWindowPosition(gGlobalWindow, &z, &t);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_Quit();
                exit(0);
                break;
            }

            if (event.type == SDL_MOUSEBUTTONDOWN) {
                game.updateGameState();
            }
        }
    }
   
}