//
// Created by lzz on 2024/3/14.
//

#ifndef ABILITY_RUNTIME_STARTUPINITIALIZER_H
#define ABILITY_RUNTIME_STARTUPINITIALIZER_H


class StartupInitializer {
public:
    StartupInitializer() = default;
    ~StartupInitializer() = default;

    static void Init();

    static void BuildAutoStartupManager();

    static void BuildManualStartupManager();

};


#endif //ABILITY_RUNTIME_STARTUPINITIALIZER_H
