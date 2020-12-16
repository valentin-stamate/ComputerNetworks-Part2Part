/*
** Here are the definitions of the client
*/

#pragma once
#include "../models.h"
#define MAX_NOTIF 50

void showWelcomeMessage(User*);
void trimString(char*, char);
void getBlocks(char[10][255], char*, int*);
int process(char[10][255], int);

void sendLoginCredentials(int, char[10][255], User*);
void getUserCredentials(int, User*);

void getLine(char*, int);

void showNotifications(char[MAX_NOTIF][500], int);
void showNotification(char*);

void pushNotification(char*, char[MAX_NOTIF][500], int*);
void popNotification(char[MAX_NOTIF][500], int*);

void getUsers(int, char[MAX_NOTIF][500], int*, User*, int*);

void showFileStatus(struct stat);
struct stat MyStat(char*, int*);
void MyFind(char*, char[100][100], int*);

void clearNotifications(char [MAX_NOTIF][500], int*);

void printColors();
