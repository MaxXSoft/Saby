#ifndef SABY_SEMA_H_
#define SABY_SEMA_H_

#include "symbol.h"

inline EnvPtr MakeEnvironment(EnvPtr outer) {
    return std::make_shared<Environment>(outer);
}

#endif // SABY_SEMA_H_
