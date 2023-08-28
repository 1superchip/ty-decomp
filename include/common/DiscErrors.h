#ifndef COMMON_DISCERRORS
#define COMMON_DISCERRORS

void DiscErr_SetError(int error);
bool DiscErr_IsError(void);
void DiscErr_ClearError(void);
void DiscErr_DrawErrorString(void);
void DiscErr_DrawString(char* pErrStr);

#endif // COMMON_DISCERRORS
