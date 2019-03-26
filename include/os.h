#ifndef OS_H
#define OS_H

long OS_GetTick ();
void OS_Sleep (long milisecs);
void OS_Exit (char* function, char* failfunction);
void OS_Init ();
void OS_ProcessEvents ();
void OS_Render ();
void OS_Terminate ();

#endif
