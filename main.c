#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

typedef enum { false, true } bool;
typedef struct s_carte
{
    int x,y,Id;
    SDL_Surface* image;
    bool gone;
} Card;
int GAP_BETWEEN_CARDS = 20;
SDL_Rect cardDimensions;
SDL_Renderer *renderer;
SDL_Window *window;
int numbCards;
int PLAYING = 0, WON = 1;
Card *Jeu;
int DELAY = 600;
int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 800;
SDL_Surface* backImage;

Card* createGame(int numberOfCards);
SDL_Window* createWindow(int width, int height);
SDL_Renderer* initRenderer(SDL_Window* window);
SDL_Surface* getCardImage(int card);
void playGame();
int gameLogic(int mouseX, int mouseY);
bool removeCard(Card card);
void unturnCard(Card card);
bool isCardClicked(int mouseX, int mouseY, Card card);

Card* createGame(int numberOfCards)
{
    if(numberOfCards % 2 == 1) return NULL;
    Card *game = malloc(sizeof(Card) * numberOfCards);
    int tab[numberOfCards];
    int i = 0;
    for(i = 0; i < numbCards; i += 2)
    {
        tab[i] = (i/2);
        tab[i+1] = (i/2);
    }

    int u,j,temp;
    for(u = 0; u<numberOfCards; u++)
    {
        j = u+ rand() % (numberOfCards-u);
        temp = tab[u];
        tab[u] = tab[j];
        tab[j] = temp;
    }

    int side = round(sqrt(numberOfCards - 1));
    int offsetX = (WINDOW_WIDTH - (sqrt(numberOfCards) * cardDimensions.w + (sqrt(numberOfCards) - 1) * GAP_BETWEEN_CARDS)) / 2;
    int offsetY = (WINDOW_HEIGHT - (sqrt(numberOfCards) * cardDimensions.h + (sqrt(numberOfCards) - 1) * GAP_BETWEEN_CARDS)) / 2;

    int xTraverse = offsetX;
    int yTraverse = offsetY;
    for(i = 0; i < numbCards; i++)
    {
        game[i].gone = false;
        game[i].x = xTraverse;
        game[i].y = yTraverse;
        game[i].image= getCardImage(tab[i]);
        game[i].Id = tab[i];

        if((i + 1) % side == 0)
        {
            xTraverse = offsetX;
            yTraverse += cardDimensions.h + GAP_BETWEEN_CARDS;
        } else {
            xTraverse += cardDimensions.w + GAP_BETWEEN_CARDS;
        }
    }
    return game;
}

SDL_Window* createWindow(int width, int height)
{
    SDL_Surface* surface = SDL_LoadBMP("imageicone.bmp");
    SDL_Window* newWindow = SDL_CreateWindow("Memory", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
    SDL_SetWindowIcon(newWindow, surface);
    if (newWindow == NULL)
    {
        printf("Window could not be initiated\n");
        exit(-1);
    }
    printf("Window init success\n");
    return newWindow;
}

SDL_Renderer* initRenderer(SDL_Window* window)
{
    SDL_Renderer* renderer = SDL_CreateRenderer(window , -1, SDL_RENDERER_ACCELERATED);
    if (renderer==NULL)
    {
        printf("Renderer init error\n");
        exit(-1);
    }
    printf("Renderer init success\n");
    return renderer;
}

SDL_Surface* getCardImage(int card)
{
    SDL_Surface* cardImage;
    char fileName[10];
    int cardNum = card + 1;
    sprintf(fileName, "carte%d.bmp", cardNum);
    cardImage = SDL_LoadBMP(fileName);
    return cardImage;
}

SDL_Rect createRect(int x, int y, int width, int height)
{
    SDL_Rect rect;
    rect.x = x; /*largeur renderer/2 - largeur image/2 pour la placé au centre*/
    rect.y = y;/*hauteur renderer/2 - hauteur image/2 pour la placé au centre*/ /* 0 et 0 */
    rect.w = width;/* largeur de l'image*/
    rect.h = height;/*hauteur*/
    return rect;
}

void playGame()
{
    SDL_Event ev;
    int gameState = PLAYING;
    printf("Game started\n");
    while(gameState == PLAYING)
    {
        while(SDL_PollEvent(&ev))
        {
            if(ev.type == SDL_QUIT)
            {
                return;
            }
            else if(ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT)
            {
                gameState = gameLogic(ev.button.x, ev.button.y);
            }
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool isCardClicked(int mouseX, int mouseY, Card card)
{
    return ((mouseX >= card.x) && (mouseX <= card.x + cardDimensions.w) && (mouseY >= card.y) && (mouseY <= card.y + cardDimensions.h) && !card.gone);
}

void unturnCard(Card card)
{
    SDL_Rect DestR;
    DestR.x = card.x;
    DestR.y = card.y;
    DestR.w = cardDimensions.w;
    DestR.h = cardDimensions.h;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, backImage);
    SDL_RenderCopy(renderer, texture, &cardDimensions, &DestR);
    SDL_RenderPresent(renderer);
}

bool removeCard(Card card)
{
    SDL_Surface *image = SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, image);
    SDL_Rect DestR;
    DestR.x = card.x;
    DestR.y = card.y;
    DestR.w = cardDimensions.w;
    DestR.h = cardDimensions.h;
    SDL_RenderFillRect(renderer,&DestR);
    SDL_RenderCopy(renderer, texture, &cardDimensions, &DestR);
    SDL_RenderPresent(renderer);
    return true;
}

void turnCard(Card card)
{
    SDL_Rect DestR;
    DestR.x = card.x;
    DestR.y = card.y;
    DestR.w = cardDimensions.w;
    DestR.h = cardDimensions.h;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, card.image);
    SDL_RenderCopy(renderer, texture, &cardDimensions, &DestR);
    SDL_RenderPresent(renderer);
}

int pairsFound;
bool visibleCard;
Card currentVisibleCard;
int currentVisibleCardIndex;
int gameLogic(int mouseX, int mouseY)
{
    int i;
    for(i = 0; i < numbCards; i++)
    {
        Card *card = &Jeu[i];
        if(i == currentVisibleCardIndex) continue;
        if(isCardClicked(mouseX, mouseY, *(card)))
        {
            turnCard(*(card));
            SDL_Delay(DELAY);
            if(visibleCard == false)
            {
                currentVisibleCard = *(card);
                currentVisibleCardIndex = i;
                visibleCard = true;
            }
            else
            {
                if(card->Id == currentVisibleCard.Id)  //It's a pair!
                {
                    pairsFound += 1;
                    printf("Found %d pairs!\n", pairsFound);
                    currentVisibleCard.gone = removeCard(currentVisibleCard);
                    card->gone = removeCard(*(card));
                    visibleCard = false;
                    if(pairsFound == (numbCards/2)){
                            printf("YOUPI YOU WON");
                            SDL_Delay(DELAY);
                            return WON;
                    }
                }
                else     //It's not a pair
                {
                    unturnCard(currentVisibleCard);
                    unturnCard(*(card));
                    visibleCard = false;
                    currentVisibleCardIndex = -1;
                }
            }
            break;
        }
    }
    return PLAYING; //Still not won you loser
}


void initBoard()
{
    SDL_Texture* texture;
    SDL_Rect DestR;
    int i;
    for(i = 0; i < numbCards; i++)
    {
        DestR.x = Jeu[i].x;
        DestR.y = Jeu[i].y;
        DestR.w = cardDimensions.w;
        DestR.h = cardDimensions.h;
        texture = SDL_CreateTextureFromSurface(renderer, backImage);
        SDL_RenderCopy(renderer, texture, &cardDimensions, &DestR);
        SDL_RenderPresent(renderer); //Force new rendering
    }
}

int main(int argc, char *argv[])
{
    numbCards = -1;
    printf("With how many cards do you want to play Memory?\n");
    while(numbCards % 2 == 1 || numbCards < 4 || numbCards > 16){
        printf("Please enter an even number between 4 and 16:\n\t");
        scanf("%d", &numbCards);
        if(numbCards % 2 == 1) printf("%d is not an even number.\n", numbCards);
        if(numbCards < 4) printf("%d is smaller than 4.\n", numbCards);
        if(numbCards > 16) printf("%d is greater than 16.\n", numbCards);
    }

    window = createWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
    renderer = initRenderer(window);

    backImage = SDL_LoadBMP("doscarte.bmp");
    cardDimensions = createRect(0, 0, 101, 146);
    srand(time(NULL)); // initialisation de rand
    currentVisibleCardIndex = -1;

    Jeu = createGame(numbCards);
    printf("Game Created\n");

    pairsFound = 0;
    visibleCard = false;

    initBoard();
    printf("Board init success\n");

    playGame();
}
