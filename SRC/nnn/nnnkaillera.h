/*
struct kailleraInfos{
  char *appName;
  char *gameList;
  int (WINAPI *gameCallback)(char *game, int player, int numplayers);
  void (WINAPI *chatReceivedCallback)(char *nick, char *text);
  void (WINAPI *clientDroppedCallback)(char *nick, int playernb);
  void (WINAPI *moreInfosCallback)(char *gamename);
};
*/
#include "kailleraclient.h"

BOOL nnnkailleraInit();
void nnnkailleraShutdown();
void nnnkailleraSelectServerDialog(HWND );
int nnnkailleraModifyPlayValues(void *, int );
void nnnkailleraSetInfos(kailleraInfos *);
void nnnkailleraEndGame();
void nnnkailleraChatSend(char *str);
