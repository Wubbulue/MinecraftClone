#include "Logging.h"

//TODO: these don't work with member functions
namespace Logging {

    template<typename F, typename... Args>
    double funcTime(F func, Args&&... args) {
        std::chrono::high_resolution_clock::time_point TimeVar t1 = timeNow();
        func(std::forward<Args>(args)...);
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - t1).count();
    }

    template<typename F, typename... Args>
    void printFuncTime(std::string funcName, F func, Args&&... args) {
        std::chrono::high_resolution_clock::time_point TimeVar t1 = timeNow();
        func(std::forward<Args>(args)...);
        double duration = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - t1).count();
        printf("Function %s took %lf milisecs", funcName, duration);

    }
};

