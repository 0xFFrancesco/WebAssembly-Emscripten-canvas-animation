#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <emscripten.h>
#include <inttypes.h>

struct Circle {
    int32_t x;
    int32_t y;
    int32_t radious;
    int32_t red;
    int32_t green;
    int32_t blue;
    int32_t alpha;
};

struct CircleAnimationData {
    float xv;
    float yv;
    int xvinitial;
    int yvinitial;
    int xdir;
    int ydir;
    int xacc;
    int yacc;
    int rinitial;
    int ginitial;
    int binitial;
    int glamount;
    int gldir;
    int radiousinitial;
    int radiousdir;
};

int CIRCLES_NUMBER = 0;
int RADIOUS_MIN = 0;
int RADIOUS_MAX = 0;
int RADIOUS_GLOWING = 0;
int RADIOUS_GLOWING_VELOCITY = 0;
int HUE_MIN = 0;
int HUE_MAX = 0;
int HUE_GLOWING = 0;
int HUE_GLOWING_VELOCITY = 0;
int ALPHA_MIN = 0;
int ALPHA_MAX = 0;
int VELOCITY_INITIAL_MIN = 0;
int VELOCITY_INITIAL_MAX = 0;
int VELOCITY_FINAL_MAX = 0;
int ACCELERATION_MIN = 0;
int ACCELERATION_MAX = 0;
int ACCELERATION_DIVIDER = 0;
int CANVAS_WIDTH = 0;
int CANVAS_HEIGHT = 0;

struct Circle *circles;
struct CircleAnimationData *circlesAnimationData;

int getRand(min, max) {
    return rand() % (max + 1 - min) + min;
}

int getItemSizeInBytes() {
    return sizeof(struct Circle);
}

// void startRendering() {
//     printf("WASM> start rendering...\n");
//     emscripten_run_script("render()");
// }

void initSettings() {
    printf("WASM> init settings...\n");

    srand(time(NULL));

    CIRCLES_NUMBER = emscripten_run_script_int("settings.circlesNumber");
    RADIOUS_MIN = emscripten_run_script_int("settings.sizeMin");
    RADIOUS_MAX = emscripten_run_script_int("settings.sizeMax");
    RADIOUS_GLOWING = emscripten_run_script_int("settings.sizeGlowing");
    RADIOUS_GLOWING_VELOCITY = emscripten_run_script_int("settings.sizeGlowingVelocity");
    HUE_MIN = emscripten_run_script_int("settings.hueMin");
    HUE_MAX = emscripten_run_script_int("settings.hueMax");
    HUE_GLOWING = emscripten_run_script_int("settings.hueGlowing");
    HUE_GLOWING_VELOCITY = emscripten_run_script_int("settings.hueGlowingVelocity");
    ALPHA_MIN = emscripten_run_script_int("settings.alphaMin");
    ALPHA_MAX = emscripten_run_script_int("settings.alphaMax");
    VELOCITY_INITIAL_MIN = emscripten_run_script_int("settings.velocityInitialMin");
    VELOCITY_INITIAL_MAX = emscripten_run_script_int("settings.velocityInitialMax");
    VELOCITY_FINAL_MAX = emscripten_run_script_int("settings.velocityFinalMax");
    ACCELERATION_MIN = emscripten_run_script_int("settings.accelerationMin");
    ACCELERATION_MAX = emscripten_run_script_int("settings.accelerationMax");
    ACCELERATION_DIVIDER = emscripten_run_script_int("settings.accelerationDivider");
    CANVAS_WIDTH = emscripten_run_script_int("settings.canvasWidth");
    CANVAS_HEIGHT = emscripten_run_script_int("settings.canvasHeight");
 
    if (sizeof(circles)) free(circles);
    if (sizeof(circlesAnimationData)) free(circlesAnimationData);
    circles = malloc(CIRCLES_NUMBER * getItemSizeInBytes());
    circlesAnimationData = malloc(CIRCLES_NUMBER * sizeof(struct CircleAnimationData));
}

void generateData() {
    printf("WASM> generate data...\n");
    
    for (int i=0; i<CIRCLES_NUMBER; i++){
        int radious = getRand(RADIOUS_MIN, RADIOUS_MAX);
        int x = CANVAS_WIDTH / 2 + getRand(-30, 30);//getRand(CANVAS_WIDTH) + (radious / 2);
        int y = CANVAS_HEIGHT / 2 + getRand(-30, 30);//getRand(CANVAS_HEIGHT) + (radious / 2);

        circles[i].radious = radious;
        circles[i].x = x;
        circles[i].y = y;
        circles[i].x = x;
        circles[i].red = getRand(HUE_MIN, HUE_MAX);
        circles[i].green = getRand(HUE_MIN, HUE_MAX);
        circles[i].blue = getRand(HUE_MIN, HUE_MAX);
        circles[i].alpha = getRand(ALPHA_MIN, ALPHA_MAX);

        circlesAnimationData[i].xv = getRand(VELOCITY_INITIAL_MIN, VELOCITY_INITIAL_MAX);
        circlesAnimationData[i].yv = getRand(VELOCITY_INITIAL_MIN, VELOCITY_INITIAL_MAX);
        circlesAnimationData[i].xvinitial = circlesAnimationData[i].xv;
        circlesAnimationData[i].yvinitial = circlesAnimationData[i].yv;
        circlesAnimationData[i].xdir = getRand(0, 1);
        circlesAnimationData[i].ydir = getRand(0, 1);
        circlesAnimationData[i].xacc = getRand(ACCELERATION_MIN, ACCELERATION_MAX);
        circlesAnimationData[i].yacc = getRand(ACCELERATION_MIN, ACCELERATION_MAX);
        circlesAnimationData[i].rinitial = circles[i].red;
        circlesAnimationData[i].ginitial = circles[i].green;
        circlesAnimationData[i].binitial = circles[i].blue;
        circlesAnimationData[i].gldir = getRand(0, 1);
        circlesAnimationData[i].radiousinitial = circles[i].radious;
        circlesAnimationData[i].radiousdir = getRand(0, 1);
    }
    //startRendering();
}

void accelerate(struct CircleAnimationData *animData){
    //ACCELLERATION
    int velocityFinalMaxX = VELOCITY_FINAL_MAX - animData->xvinitial;
    if (animData->xv <= velocityFinalMaxX) {
        float xaccel = animData->xacc / (float)ACCELERATION_DIVIDER;
        float newXVelocity = animData->xv + xaccel;
        animData->xv = newXVelocity > velocityFinalMaxX ? velocityFinalMaxX : newXVelocity;
    }

    int velocityFinalMaxY = VELOCITY_FINAL_MAX - animData->yvinitial;
    if (animData->yv <= velocityFinalMaxY) {
        float yaccel = animData->yacc / (float)ACCELERATION_DIVIDER;
        float newYVelocity = animData->yv + yaccel;
        animData->yv = newYVelocity > velocityFinalMaxY ? velocityFinalMaxY : newYVelocity;
    }
}

int getGlowingValue(struct CircleAnimationData *animData, int *dir, int initialValue, int currentValue, int allowDirChange, int glowingDelta, int velocity) {
    if (*dir == 1) {
        int newValue = currentValue + velocity;
        if (newValue >= initialValue + glowingDelta) {
            newValue = initialValue + glowingDelta;
            if (allowDirChange == 1) {
                *dir = 0;
            }
        }
        return newValue;
    } else {
        int newValue = currentValue - velocity;
        if (newValue <= initialValue - glowingDelta || newValue < 0) {
            newValue = initialValue - glowingDelta > 0 ? initialValue - glowingDelta : 0;
            if (allowDirChange == 1) {
                *dir = 1;
            }
        }
        return newValue;
    }
}

void glowing(struct CircleAnimationData *animData, struct Circle *circleItem){
    
    int newR = getGlowingValue(animData, &animData->gldir, animData->rinitial, circleItem->red, 0, HUE_GLOWING, HUE_GLOWING_VELOCITY);
    int newG = getGlowingValue(animData, &animData->gldir, animData->ginitial, circleItem->green, 0, HUE_GLOWING, HUE_GLOWING_VELOCITY);
    int newB = getGlowingValue(animData, &animData->gldir, animData->binitial, circleItem->blue, 1, HUE_GLOWING, HUE_GLOWING_VELOCITY);

    circleItem->red = newR;
    circleItem->green = newG;
    circleItem->blue = newB;

    int newRadious = getGlowingValue(animData, &animData->radiousdir, animData->radiousinitial, circleItem->radious, 1, RADIOUS_GLOWING, RADIOUS_GLOWING_VELOCITY);

    circleItem->radious =newRadious;

}

void handleCollisionX(struct CircleAnimationData *animData, struct Circle *circleItem){
    //COLLISON BORDER DETECT X
    if (animData->xdir == 1) {
        if ((circleItem->x + animData->xv + circleItem->radious) >= CANVAS_WIDTH) {
            animData->xdir = 0;
            animData->ydir = getRand(0, 1);
            circleItem->x = CANVAS_WIDTH - circleItem->radious; //- abs(CANVAS_WIDTH - circleItem->x - circleItem->radious - animData->xv);
        } else {
            circleItem->x += animData->xv;
        }
    } else {
        if ((circleItem->x - animData->xv - circleItem->radious) <= 0) {
            animData->xdir = 1;
            animData->ydir = getRand(0, 1);
            circleItem->x = circleItem->radious; //abs(circleItem->x - circleItem->radious - animData->xv);
        } else {
            circleItem->x -= animData->xv;
        }
    }
}

void handleCollisionY(struct CircleAnimationData *animData, struct Circle *circleItem){
    //COLLISON BORDER DETECT Y
    if (animData->ydir == 1) {
        if ((circleItem->y + animData->yv + circleItem->radious) >= CANVAS_HEIGHT) {
            animData->ydir = 0;
            animData->xdir = getRand(0, 1);
            circleItem->y = CANVAS_HEIGHT - circleItem->radious; //- abs(CANVAS_HEIGHT - circleItem->y - circleItem->radious - animData->yv);
        } else {
            circleItem->y += animData->yv;
        }
    } else {
        if ((circleItem->y - animData->yv - circleItem->radious) <= 0) {
            animData->ydir = 1;
            animData->xdir = getRand(0, 1);
            circleItem->y = circleItem->radious; //abs(circleItem->y - circleItem->radious - animData->yv);
        } else {
            circleItem->y -= animData->yv;
        }
    }
}

void updateData() {

    for (int i=0; i<CIRCLES_NUMBER; i++){
        accelerate(&circlesAnimationData[i]);
        glowing(&circlesAnimationData[i], &circles[i]);
        handleCollisionX(&circlesAnimationData[i], &circles[i]);
        handleCollisionY(&circlesAnimationData[i], &circles[i]);
    }

}

struct Circle *getCirclesMemoryPointerInBytes() {
    return circles;
}
