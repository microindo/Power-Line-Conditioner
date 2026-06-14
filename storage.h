#ifndef STORAGE_H
#define STORAGE_H

#include <Preferences.h>
#include "config.h"

extern AppConfig appConfig;

void loadConfig();
void saveConfig();
void resetConfig();
void printConfig();

#endif
