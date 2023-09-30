//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <io.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <Windows.h>
// process.h


#include "TlHelp32.h"
#include "Unit1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------

__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
//--------------------------------------------------------
 HANDLE hMutex;
// atexit(xx);
 hMutex = CreateMutex(0, 0, "xxx"); // Если наш Mutex не найден создаем
// CloseHandle(hMutex); // Закрываем ручку
// exit(0);
// int a;
// a=0;
//--------------------------------------------------------
}
//---------------------------------------------------------------------------

