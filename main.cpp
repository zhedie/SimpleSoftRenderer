#include "application.h"
#include <assimp/Importer.hpp>

extern Application* createApplication();
bool g_ApplicationRunning = true;

int main(int argc, char **argv) {
    while (g_ApplicationRunning) {
        Application *app = createApplication();
        app->run();
        delete app;
    }
    return 0;
}