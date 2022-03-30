/**
 * Main.cpp skeleton
 * Contributors:
 *      * Arthur Sonzogni
 * Licence:
 *      * MIT
 */

#include "MyApplication.hpp"

int main(int argc, const char* argv[]) {
    MyApplication app(argv[1], 1280, 960);
    app.run();
    return 0;
}
