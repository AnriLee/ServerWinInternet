//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
#include <dirent.h>
#include <dir.h>
#include <time.h>
#include <sys\timeb.h>
//---------------------------------------------------------------------------
#include <windows.h>
#include <tlhelp32.h>
#include <sys/stat.h>
//---------------------------------------------------------------------------

#include "Main.h"
#include "Obgect.h"
#include "LiteModul.h"
#include "Internet.h"
#include "ModulTCP.h"

STARTUPINFO si = { sizeof(si) };
PROCESS_INFORMATION pi;
//char *NameWEB = "spider.exe";
char *NameWEB = "Project1.exe";

struct SERVERINFO Server; // настройка сервера
short ProcesFlag = 0;

//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"

 TFormMain *FormMain;
//---------------------------------------------------------------------------

char *UtfAsci( char *str, wchar_t *sourse )
 {
 char buf[200];
 AnsiString AS;

 AS = sourse; strcpy( buf, AS.c_str() );
 if ( str ) strcpy( str, buf );
 return( buf );
 }
//---------------------------------------------------------------------------

// фунции определения промежутка времени (ms)
unsigned int StartControlTime( void )
 {
 unsigned int StartClock;
 SYSTEMTIME st;

 GetLocalTime(&st);
 StartClock = st.wSecond * 1000; StartClock += st.wMilliseconds;
 return( StartClock );
 }

unsigned int GetControlTime( unsigned int OldTime )
 {
 unsigned int Clock;
 unsigned char OldSec;
 unsigned int mSec;
 SYSTEMTIME st;

 GetLocalTime(&st);
 OldSec = OldTime / 1000; mSec = OldTime % 1000; Clock = 0;
 if( st.wSecond != OldSec )
	{
	if( st.wSecond < OldSec )
		{ Clock = 60 - OldSec; Clock += st.wSecond; Clock *= 1000; }
	else { Clock = st.wSecond - OldSec; Clock *= 1000; }
	}
 Clock += st.wMilliseconds; Clock -= mSec;
 return( Clock );
 }
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

__fastcall TFormMain::TFormMain(TComponent* Owner) : TForm(Owner)
 {
 char *str;

 str = GetCommandLine( );
 if( str[0] == '"' ) str++; strncpy( Patch, str, 500 );
 str = strrchr( Patch, '\\' ); str++; *str = 0;
 memset( (void *)&Server, 0, sizeof( struct SERVERINFO ) );
 }
//---------------------------------------------------------------------------

void __fastcall TFormMain::Start(TObject *Sender)
 {
 ProcesFlag = 0;
 SqModule->ReadTableServis( (void *)&Server );
 DataModuleInternet->ServerInternet->Port = Server.IntPortServer;
 DataModuleInternet->ServerInternet->Active = true;
 OpenLog( );
 // включить WEB сервер
// CreateProcess(NULL, NameWEB, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
// void * hMutex = OpenMutex(MUTEX_ALL_ACCESS, 0, "xxx"); // Открываем Mutex
// unsigned int PID = ShellExecute( "Project1.exe" );
// void * hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
//																   false, PID );
// BOOL x = TerminateProcess( hProcess, 0 );

// CloseHandle(hMutex);
// BOOL x = TerminateProcess( hMutex, 0 );
//--------------------------------------------------------
//int nnn = ProcessExists(NameWEB);

//		Run(NameWEB);
//		WinExists("notepad.exe");
//		WinClose(NameWEB, "");

// BOOL x = TerminateProcess( hMutex, 0 );
// CloseHandle(hMutex); // Закрываем ручку
//SendMessage(hMutex,WM_DESTROY,0,0);
//  CloseHandle( pi.hProcess );
// hMutex = OpenMutex(MUTEX_ALL_ACCESS, 0, "xxx"); // Открываем Mutex
//--------------------------------------------------------
//	Process [] proc = Process.GetProcessesByName("utorrent");
	//proc[0].Kill();
//HWND hwnd;
// hwnd = FindWindow(NULL,"NameWEB");
// SendMessage(hwnd,WM_DESTROY,0,0);
 }

void __fastcall TFormMain::Stop(TObject *Sender, TCloseAction &Action)
 {
 int a;

 for( a = 0; a < ModuleTCP->ServerTCP->Socket->ActiveConnections; a++ )
						ModuleTCP->ServerTCP->Socket->Connections[a]->Close( );
 ModuleTCP->ServerTCP->Close( );
 CloseLog( );
// BOOL x = TerminateProcess( pi.hProcess, 0 );

 }
//---------------------------------------------------------------------------

void TFormMain::OpenLog( void )
 {
 char str[200];

 sprintf( str, "%sServerLog.log", Patch );
 OutMess = 0; OutMess = fopen( str, "a+t" ); WriteLog( "старт службы" );
 }

void TFormMain::CloseLog( void )
 {
 ModuleTCP->ServerTCP->Active = false;
 if( OutMess ) { WriteLog( "стоп службы" ); fclose( OutMess ); }
 }

void TFormMain::WriteLog( char *mess )
 {
 time_t t;
 struct tm *Time;
 char str[50];

 if( !OutMess ) return;
 time(&t); Time = localtime(&t);

 sprintf( str, "%2.2u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u ",
			Time->tm_mday, Time->tm_mon+1, Time->tm_year-100,
						Time->tm_hour, Time->tm_min, Time->tm_sec );
 fputs( str, OutMess ); fputs( mess, OutMess ); fputc( '\n', OutMess );
 fflush( OutMess );
 }
//---------------------------------------------------------------------------

void __fastcall TFormMain::HandleTimer(TObject *Sender)
 {
 DWORD result = WaitForSingleObject(pi.hProcess, 0);
 if( result ) return;
 TerminateProcess( pi.hProcess, 0 ); CloseHandle( pi.hProcess );
 CreateProcess(NULL, NameWEB, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
 }
//---------------------------------------------------------------------------

