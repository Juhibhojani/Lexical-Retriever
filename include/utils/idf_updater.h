#pragma once
#include "../models/idf_table.h"

// Function that will run inside pthread
void* idf_updater_thread(void* arg);
