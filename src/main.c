#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// --- Configurações da Simulação ---
const int WIDTH = 800;
const int HEIGHT = 600;
const int NUM_BALLS = 15;
const float RESTITUTION_COEFFICIENT = 0.85f; // Para ver a energia diminuir
// const float RESTITUTION_COEFFICIENT = 1.0f; // Para ver a energia se conservar

// Configurações de inicialização das bolinhas
const int MIN_BALL_RADIUS = 15;
const int MAX_BALL_RADIUS = 35;
const float VELOCITY_SCALE = 200.0f;

// Variável para controlar a exibição de informações de debug
bool showDebugInfo = true;

// Estrutura para representar uma bola
typedef struct Ball {
    Vector2 position;
    Vector2 velocity;
    int radius;
    float mass;
    Color color;
} Ball;

// --- Protótipos das Funções ---
void InitBalls(Ball balls[], int numBalls);
void UpdateFrame(Ball balls[], int numBalls);
// MODIFICADO: A função de desenho agora recebe a energia cinética
void DrawFrame(Ball balls[], int numBalls, float kineticEnergy);
void CheckBallCollision(Ball *ball1, Ball *ball2);
void CheckWallCollision(Ball *ball);
// NOVO: Protótipo da nossa nova função de cálculo
float CalculateTotalKineticEnergy(Ball balls[], int numBalls);


//==================================================================================
// Função Principal
//==================================================================================
int main(void) {
    InitWindow(WIDTH, HEIGHT, "Simulador de Colisões com Energia Cinética");
    SetTargetFPS(144);

    SetRandomSeed((unsigned int)time(NULL));

    Ball balls[NUM_BALLS];
    InitBalls(balls, NUM_BALLS);

    // Loop principal da simulação
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_R)) InitBalls(balls, NUM_BALLS);
        if (IsKeyPressed(KEY_D)) showDebugInfo = !showDebugInfo;
        
        UpdateFrame(balls, NUM_BALLS);

        // NOVO: Calcula a energia total a cada quadro
        float totalKE = CalculateTotalKineticEnergy(balls, NUM_BALLS);
        
        // MODIFICADO: Passa a energia calculada para a função de desenho
        DrawFrame(balls, NUM_BALLS, totalKE);
    }

    CloseWindow();
    return 0;
}

//==================================================================================
// NOVO: Função para Calcular a Energia Cinética Total
//==================================================================================
float CalculateTotalKineticEnergy(Ball balls[], int numBalls) {
    float totalEnergy = 0.0f;
    for (int i = 0; i < numBalls; i++) {
        // Primeiro, calculamos o quadrado da velocidade (v^2 = vx^2 + vy^2)
        float speedSq = balls[i].velocity.x * balls[i].velocity.x + balls[i].velocity.y * balls[i].velocity.y;
        
        // Agora, usamos a fórmula KE = 0.5 * m * v^2
        float kineticEnergy = 0.5f * balls[i].mass * speedSq;
        
        totalEnergy += kineticEnergy;
    }
    return totalEnergy;
}


//==================================================================================
// Funções de Lógica e Desenho
//==================================================================================

// Função para atualizar o estado da simulação a cada quadro
void UpdateFrame(Ball balls[], int numBalls) {
    float deltaTime = GetFrameTime();

    // Atualiza a posição das bolinhas
    for (int i = 0; i < numBalls; i++) {
        balls[i].position.x += balls[i].velocity.x * deltaTime;
        balls[i].position.y += balls[i].velocity.y * deltaTime;
    }

    // Verifica e resolve todas as colisões
    for (int i = 0; i < numBalls; i++) {
        CheckWallCollision(&balls[i]);
        for (int j = i + 1; j < numBalls; j++) {
            CheckBallCollision(&balls[i], &balls[j]);
        }
    }
}

// MODIFICADO: A função agora recebe um float com a energia
void DrawFrame(Ball balls[], int numBalls, float kineticEnergy) {
    BeginDrawing();
    ClearBackground(BLACK);

    // Desenha as bolinhas
    for (int i = 0; i < numBalls; i++) {
        DrawCircleV(balls[i].position, balls[i].radius, balls[i].color);
        if (showDebugInfo) {
            DrawText(TextFormat("M:%.1f", balls[i].mass), balls[i].position.x - 15, balls[i].position.y - 8, 10, WHITE);
        }
    }
    
    DrawRectangleLines(0, 0, WIDTH, HEIGHT, DARKGRAY);

    // Exibe informações na tela
    DrawText(TextFormat("Bolinhas: %d", NUM_BALLS), 10, 10, 20, RAYWHITE);
    DrawText(TextFormat("Restituição: %.2f", RESTITUTION_COEFFICIENT), 10, 35, 20, RAYWHITE);
    
    // NOVO: Exibe a Energia Cinética Total na tela
    DrawText(TextFormat("Energia Cinética Total: %.0f", kineticEnergy), 10, 60, 20, LIME);

    DrawFPS(WIDTH - 90, 10);
    DrawText("Pressione [R] para reiniciar", WIDTH - 170, 40, 10, GRAY);
    DrawText("Pressione [D] para info", WIDTH - 170, 55, 10, GRAY);

    EndDrawing();
}


//==================================================================================
// Funções de Inicialização e Física (sem alterações)
//==================================================================================

void InitBalls(Ball balls[], int numBalls) {
    for (int i = 0; i < numBalls; i++) {
        balls[i].radius = GetRandomValue(MIN_BALL_RADIUS, MAX_BALL_RADIUS);
        balls[i].mass = (float)balls[i].radius / 2.0f;

        bool positionFound = false;
        int attempts = 0;
        while (!positionFound && attempts < 100) {
            balls[i].position = (Vector2){
                (float)GetRandomValue(balls[i].radius, WIDTH - balls[i].radius),
                (float)GetRandomValue(balls[i].radius, HEIGHT - balls[i].radius)
            };
            
            positionFound = true;
            for (int j = 0; j < i; j++) {
                float distSq = (balls[i].position.x - balls[j].position.x) * (balls[i].position.x - balls[j].position.x) +
                               (balls[i].position.y - balls[j].position.y) * (balls[i].position.y - balls[j].position.y);
                float min_dist = (float)(balls[i].radius + balls[j].radius);
                
                if (distSq < min_dist * min_dist) {
                    positionFound = false;
                    break;
                }
            }
            attempts++;
        }
        
        balls[i].velocity = (Vector2){
            (float)GetRandomValue(-VELOCITY_SCALE, VELOCITY_SCALE),
            (float)GetRandomValue(-VELOCITY_SCALE, VELOCITY_SCALE)
        };

        balls[i].color = (Color){ (unsigned char)GetRandomValue(100, 255), (unsigned char)GetRandomValue(100, 255), (unsigned char)GetRandomValue(100, 255), 255 };
    }
}

void CheckWallCollision(Ball *ball) {
    if (ball->position.x - ball->radius <= 0) {
        ball->position.x = ball->radius;
        ball->velocity.x *= -RESTITUTION_COEFFICIENT;
    } else if (ball->position.x + ball->radius >= WIDTH) {
        ball->position.x = WIDTH - ball->radius;
        ball->velocity.x *= -RESTITUTION_COEFFICIENT;
    }

    if (ball->position.y - ball->radius <= 0) {
        ball->position.y = ball->radius;
        ball->velocity.y *= -RESTITUTION_COEFFICIENT;
    } else if (ball->position.y + ball->radius >= HEIGHT) {
        ball->position.y = HEIGHT - ball->radius;
        ball->velocity.y *= -RESTITUTION_COEFFICIENT;
    }
}

void CheckBallCollision(Ball *b1, Ball *b2) {
    float dx = b2->position.x - b1->position.x;
    float dy = b2->position.y - b1->position.y;
    float distSq = dx * dx + dy * dy;
    float min_dist = (float)(b1->radius + b2->radius);

    if (distSq < min_dist * min_dist && distSq > 0) {
        float distance = sqrtf(distSq);
        
        float nx = dx / distance;
        float ny = dy / distance;

        float overlap = 0.5f * (min_dist - distance);
        b1->position.x -= overlap * nx;
        b1->position.y -= overlap * ny;
        b2->position.x += overlap * nx;
        b2->position.y += overlap * ny;
        
        Vector2 relativeVelocity = { b2->velocity.x - b1->velocity.x, b2->velocity.y - b1->velocity.y };
        float velocityAlongNormal = relativeVelocity.x * nx + relativeVelocity.y * ny;
        
        if (velocityAlongNormal > 0) return;
        
        float impulse = -(1.0f + RESTITUTION_COEFFICIENT) * velocityAlongNormal / (1.0f / b1->mass + 1.0f / b2->mass);
        
        b1->velocity.x -= impulse * nx / b1->mass;
        b1->velocity.y -= impulse * ny / b1->mass;
        b2->velocity.x += impulse * nx / b2->mass;
        b2->velocity.y += impulse * ny / b2->mass;
    }
}