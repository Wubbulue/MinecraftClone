#ifndef LOGGING_HEADER
#define LOGGING_HEADER

#include <chrono>
#include <utility>
#include <string>
#include <stdio.h>


namespace Logging {

    template<typename F, typename... Args>
    double funcTime(F func, Args&&... args);


    template<typename F, typename... Args>
    void printFuncTime(std::string funcName, F func, Args&&... args);

    auto timeFuncInvocation =
        [](auto&& func, auto&&... params) {
        // get time before function invocation
        const auto& start = high_resolution_clock::now();
        // function invocation using perfect forwarding
        for (auto i = 0; i < 100000/*largeNumber*/; ++i) {
            std::forward<decltype(func)>(func)(std::forward<decltype(params)>(params)...);
        }
        // get time after function invocation
        const auto& stop = high_resolution_clock::now();
        return (stop - start) / 100000/*largeNumber*/;
    };


};

#endif // !LOGGING_HEADER
