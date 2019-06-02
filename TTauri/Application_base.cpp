// Copyright 2019 Pokitec
// All rights reserved.

#include "Application_base.hpp"
#include "logging.hpp"

namespace TTauri {

using namespace std;

Application_base::Application_base(std::shared_ptr<ApplicationDelegate> delegate) :
    delegate(move(delegate))
{
    initializeLogging();
    LOG_INFO("Starting application.");
}

void Application_base::startingLoop()
{
    if (!loopStarted) {
        loopStarted = true;
        delegate->startingLoop();
    }
}

void Application_base::lastWindowClosed()
{
    delegate->lastWindowClosed();
}

}
