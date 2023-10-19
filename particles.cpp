#include <Windows.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#pragma comment(lib, "winmm.lib")

#define NUM_PARTICLES 10000
#define NUM_FRAMES 10

uint64_t _qpc_frequency = 0;

// nanoseconds
int64_t time_now() {
    LARGE_INTEGER counter = {};
    QueryPerformanceCounter(&counter);
    return int64_t((1e9 * (uint64_t)counter.QuadPart) / _qpc_frequency);
}

struct Vec3 {
    float x;
    float y;
    float z;
};

struct Particle {
    Vec3 pos;
    Vec3 vel;
    float rad;
};

Particle particles[NUM_PARTICLES];

uint32_t hash(uint32_t x) {
    x = ((x >> 16) ^ x) * (uint32_t)0x45d9f3b;
    x = ((x >> 16) ^ x) * (uint32_t)0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

int main() {
    timeBeginPeriod(1);
    QueryPerformanceFrequency((LARGE_INTEGER*)&_qpc_frequency);

    for(int i = 0; i < NUM_PARTICLES; i++) {
        uint32_t h = hash(i);
        Particle p = {};
        p.rad = (float)(50 + (i % 100)) / 100.0f;
        p.pos = {
            (float)(hash(h * 1) % 10000) / 100.0f,
            (float)(hash(h * 7) % 10000) / 100.0f,
            (float)(hash(i * 13) % 10000) / 100.0f,
        };
        particles[i] = p;
    }
    
    int64_t start = time_now();

    for(int frame = 0; frame < NUM_FRAMES; frame++) {
        // printf("frame %i\n", frame);
        
        float delta = 0.01f + (float)(hash(frame) % 100) / 10000.0f;
        for(int i = 0; i < NUM_PARTICLES; i++) {
            Particle p = particles[i];

            p.vel.y -= delta * 9.81f;

            p.vel = {
                p.vel.x / (1.0f + delta * 3.5f),
                p.vel.y / (1.0f + delta * 3.5f),
                p.vel.z / (1.0f + delta * 3.5f),
            };

            p.pos = {
                p.pos.x + p.vel.x * delta,
                p.pos.y + p.vel.y * delta,
                p.pos.z + p.vel.z * delta,
            };

            particles[i] = p;
        }

        for(int i = 0; i < NUM_PARTICLES; i++) {
            Particle pi = particles[i];
            for(int j = i + 1; j < NUM_PARTICLES; j++) {
                Particle pj = particles[j];

                Vec3 i_to_j = {
                    pj.pos.x - pi.pos.x,
                    pj.pos.y - pi.pos.y,
                    pj.pos.z - pi.pos.z,
                };

                float dist = sqrt(i_to_j.x * i_to_j.x) + sqrt(i_to_j.y * i_to_j.y) + sqrt(i_to_j.z * i_to_j.z);

                if (dist < 1e-9f) {
                    continue;
                }

                float overlap = dist - (pi.rad + pj.rad);

                if (overlap > 0.0f) {
                    Vec3 i_to_j_norm = {
                        i_to_j.x / dist,
                        i_to_j.y / dist,
                        i_to_j.z / dist,
                    };

                    pi.vel = {
                        pi.vel.x - i_to_j.x * delta,
                        pi.vel.y - i_to_j.y * delta,
                        pi.vel.z - i_to_j.z * delta,
                    };

                    pj.vel = {
                        pj.vel.x + i_to_j.x * delta,
                        pj.vel.y + i_to_j.y * delta,
                        pj.vel.z + i_to_j.z * delta,
                    };
                }
            }

            particles[i] = pi;
        }
    }

    int64_t end = time_now();
    printf("time: %f ms\n", (float)(end - start) / 1e6f);

    uint32_t validation = 0;
    for (int i = 0; i < NUM_PARTICLES; i++) {
        Particle p = particles[i];
        validation ^= *(uint32_t*)(&p.pos.x);
        validation ^= *(uint32_t*)(&p.pos.y);
        validation ^= *(uint32_t*)(&p.pos.z);
        validation ^= *(uint32_t*)(&p.vel.x);
        validation ^= *(uint32_t*)(&p.vel.y);
        validation ^= *(uint32_t*)(&p.vel.z);
        validation ^= *(uint32_t*)(&p.rad);
    }

    printf("validation code: %x\n", validation);
}