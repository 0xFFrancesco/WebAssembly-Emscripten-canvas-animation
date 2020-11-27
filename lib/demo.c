#include <emscripten.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

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
    float xVelovity;
    float yVelocity;
    int xVelocityInitial;
    int yVelocityInitial;
    int xDirection;
    int yDirection;
    int xAcceleration;
    int yAcceleration;
    int redInitial;
    int greenInitial;
    int blueInitial;
    int hueGLowingAmount;
    int hueGlowgingDirection;
    int radiousInitial;
    int radiousDirection;
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

int getRandInRange(min, max) {
    return rand() % (max + 1 - min) + min;
}

int getItemSizeInBytes() {
    return sizeof(struct Circle);
}

struct Circle *getCirclesMemoryPointerInBytes() {
    return circles;
}

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
        circles[i].radious = getRandInRange(RADIOUS_MIN, RADIOUS_MAX);;
        circles[i].x =  CANVAS_WIDTH / 2 + getRandInRange(-30, 30); //getRand(radious, CANVAS_WIDTH - radious);
        circles[i].y = CANVAS_HEIGHT / 2 + getRandInRange(-30, 30); //getRand(radious, CANVAS_HEIGHT - radious);
        circles[i].red = getRandInRange(HUE_MIN, HUE_MAX);
        circles[i].green = getRandInRange(HUE_MIN, HUE_MAX);
        circles[i].blue = getRandInRange(HUE_MIN, HUE_MAX);
        circles[i].alpha = getRandInRange(ALPHA_MIN, ALPHA_MAX);

        circlesAnimationData[i].xVelovity = getRandInRange(VELOCITY_INITIAL_MIN, VELOCITY_INITIAL_MAX);
        circlesAnimationData[i].yVelocity = getRandInRange(VELOCITY_INITIAL_MIN, VELOCITY_INITIAL_MAX);
        circlesAnimationData[i].xVelocityInitial = circlesAnimationData[i].xVelovity;
        circlesAnimationData[i].yVelocityInitial = circlesAnimationData[i].yVelocity;
        circlesAnimationData[i].xDirection = getRandInRange(0, 1);
        circlesAnimationData[i].yDirection = getRandInRange(0, 1);
        circlesAnimationData[i].xAcceleration = getRandInRange(ACCELERATION_MIN, ACCELERATION_MAX);
        circlesAnimationData[i].yAcceleration = getRandInRange(ACCELERATION_MIN, ACCELERATION_MAX);
        circlesAnimationData[i].redInitial = circles[i].red;
        circlesAnimationData[i].greenInitial = circles[i].green;
        circlesAnimationData[i].blueInitial = circles[i].blue;
        circlesAnimationData[i].hueGlowgingDirection = getRandInRange(0, 1);
        circlesAnimationData[i].radiousInitial = circles[i].radious;
        circlesAnimationData[i].radiousDirection = getRandInRange(0, 1);
    }
}

void computeAcceleration(struct CircleAnimationData *animData){
    int velocityFinalMaxX = VELOCITY_FINAL_MAX - animData->xVelocityInitial;
    if (animData->xVelovity <= velocityFinalMaxX) {
        float xAcceleration = animData->xAcceleration / (float)ACCELERATION_DIVIDER;
        float newXVelocity = animData->xVelovity + xAcceleration;
        animData->xVelovity = newXVelocity > velocityFinalMaxX ? velocityFinalMaxX : newXVelocity;
    }

    int velocityFinalMaxY = VELOCITY_FINAL_MAX - animData->yVelocityInitial;
    if (animData->yVelocity <= velocityFinalMaxY) {
        float yAcceleration = animData->yAcceleration / (float)ACCELERATION_DIVIDER;
        float newYVelocity = animData->yVelocity + yAcceleration;
        animData->yVelocity = newYVelocity > velocityFinalMaxY ? velocityFinalMaxY : newYVelocity;
    }
}

int getGlowingValue(struct CircleAnimationData *animData, int *direction, int initialValue, int currentValue, int allowDirectionChange, int glowingAmount, int velocity) {
    if (*direction == 1) {
        int newValue = currentValue + velocity;
        if (newValue >= initialValue + glowingAmount) {
            newValue = initialValue + glowingAmount;
            if (allowDirectionChange == 1) {
                *direction = 0;
            }
        }
        return newValue;
    } else {
        int newValue = currentValue - velocity;
        if (newValue <= initialValue - glowingAmount || newValue < 0) {
            newValue = initialValue - glowingAmount > 0 ? initialValue - glowingAmount : 0;
            if (allowDirectionChange == 1) {
                *direction = 1;
            }
        }
        return newValue;
    }
}

void computeGlowing(struct CircleAnimationData *animData, struct Circle *circleItem){
    
    int newRed = getGlowingValue(animData, &animData->hueGlowgingDirection, animData->redInitial, circleItem->red, 0, HUE_GLOWING, HUE_GLOWING_VELOCITY);
    int newGreen = getGlowingValue(animData, &animData->hueGlowgingDirection, animData->greenInitial, circleItem->green, 0, HUE_GLOWING, HUE_GLOWING_VELOCITY);
    int newBlue = getGlowingValue(animData, &animData->hueGlowgingDirection, animData->blueInitial, circleItem->blue, 1, HUE_GLOWING, HUE_GLOWING_VELOCITY);

    circleItem->red = newRed;
    circleItem->green = newGreen;
    circleItem->blue = newBlue;

    int newRadious = getGlowingValue(animData, &animData->radiousDirection, animData->radiousInitial, circleItem->radious, 1, RADIOUS_GLOWING, RADIOUS_GLOWING_VELOCITY);

    circleItem->radious = newRadious;

}

void handleCollisionX(struct CircleAnimationData *animData, struct Circle *circleItem){
    //COLLISON BORDER DETECT X
    if (animData->xDirection == 1) {
        if ((circleItem->x + animData->xVelovity + circleItem->radious) >= CANVAS_WIDTH) {
            animData->xDirection = 0;
            animData->yDirection = getRandInRange(0, 1);
            circleItem->x = CANVAS_WIDTH - circleItem->radious; //- abs(CANVAS_WIDTH - circleItem->x - circleItem->radious - animData->xv);
        } else {
            circleItem->x += animData->xVelovity;
        }
    } else {
        if ((circleItem->x - animData->xVelovity - circleItem->radious) <= 0) {
            animData->xDirection = 1;
            animData->yDirection = getRandInRange(0, 1);
            circleItem->x = circleItem->radious; //abs(circleItem->x - circleItem->radious - animData->xv);
        } else {
            circleItem->x -= animData->xVelovity;
        }
    }
}

void handleCollisionY(struct CircleAnimationData *animData, struct Circle *circleItem){
    //COLLISON BORDER DETECT Y
    if (animData->yDirection == 1) {
        if ((circleItem->y + animData->yVelocity + circleItem->radious) >= CANVAS_HEIGHT) {
            animData->yDirection = 0;
            animData->xDirection = getRandInRange(0, 1);
            circleItem->y = CANVAS_HEIGHT - circleItem->radious; //- abs(CANVAS_HEIGHT - circleItem->y - circleItem->radious - animData->yv);
        } else {
            circleItem->y += animData->yVelocity;
        }
    } else {
        if ((circleItem->y - animData->yVelocity - circleItem->radious) <= 0) {
            animData->yDirection = 1;
            animData->xDirection = getRandInRange(0, 1);
            circleItem->y = circleItem->radious; //abs(circleItem->y - circleItem->radious - animData->yv);
        } else {
            circleItem->y -= animData->yVelocity;
        }
    }
}

void updateData() {
    for (int i=0; i<CIRCLES_NUMBER; i++){
        computeAcceleration(&circlesAnimationData[i]);
        computeGlowing(&circlesAnimationData[i], &circles[i]);
        handleCollisionX(&circlesAnimationData[i], &circles[i]);
        handleCollisionY(&circlesAnimationData[i], &circles[i]);
    }
}