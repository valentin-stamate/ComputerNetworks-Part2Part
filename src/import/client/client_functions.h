/*
** Here are the definitions of the client
*/

#pragma once
#include "../models.h"


void showWelcomeMessage();
void trimString(char*, char);
void getBlocks(char[10][100], char*, int*);
int process(char[10][100], int);

int sendLoginCredentials(int, char[10][100], User*);


