//---------------------------------------------------------------------------
#pragma hdrstop

#include "Main.h"
#include "Obgect.h"
#include "Internet.h"
#include "LiteModul.h"
#include "ModulTCP.h"
#include "ModuleTimer.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma classgroup "Vcl.Controls.TControl"
#pragma resource "*.dfm"

TDataModuleInternet *DataModuleInternet;

//---------------------------------------------------------------------------
__fastcall TDataModuleInternet::
	TDataModuleInternet(TComponent* Owner):	TDataModule(Owner)
 {
 static int MemLong;

 MemLong = sizeof(struct INP_ARH);
 // память приёма пакетов TCP
 InpData = new TMemoryStream(); InpData->Clear( ); InpData->SetSize( MemLong );
 }
//---------------------------------------------------------------------------

void __fastcall TDataModuleInternet::Connect(TObject *Sender, TCustomWinSocket *Socket)
 {
 TDataModuleTimer *Timer;
 // Socket->Data используется как счётчик тайм-оута для различных функций
 Timer = new TDataModuleTimer(Application);
 Timer->Socket = Socket; Timer->Count = 25;
 Socket->Data = Timer; Timer->Timer->Enabled = true;
 }
//---------------------------------------------------------------------------

void __fastcall TDataModuleInternet::ErrServer(TObject *Sender,
	TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode)
 {
 char mess[100];

 ErrorCode = 0;
 sprintf( mess, "Ошибка интернет соединения %s",
	UtfAsci( 0, Socket->RemoteAddress.c_str() ) ); FormMain->WriteLog( mess );
 Socket->Close( );
 }
//---------------------------------------------------------------------------

void __fastcall TDataModuleInternet::Close(TObject *Sender, TCustomWinSocket *Socket)

 {
 TDataModuleTimer *Timer;

 Timer = (TDataModuleTimer *)Socket->Data;
 Timer->Timer->Enabled = false; Timer->Free( );
 }
//---------------------------------------------------------------------------

void __fastcall TDataModuleInternet::ReadData( TObject *Sender,
											TCustomWinSocket *Socket )
 {
 struct INP_ARH *Data;
 static int count;
 signed char flag;

 // проверка длины минимального пакета
 count = Socket->ReceiveLength(); if( !count ) return; // ошибка приёма
 if( count < sizeof(struct PUBLIC) ) return; // ошибка приёма

 if( count > sizeof(struct INP_ARH) ) InpData->SetSize( count );

 Data = (struct INP_ARH *)InpData->Memory;
 InpData->Position = 0; Socket->ReceiveBuf( InpData->Memory, count );
 if( count > sizeof(struct INP_ARH) ) return;
 Data = (struct INP_ARH *)InpData->Memory;

 // проверка на верность пакета
 // ошибка пакета -> синхронизировать с потерей данных
 if( count > sizeof(struct INP_ARH) ) return;
 if( strncmp( Data->Pbl.Password, "ORION", 5) ) return;
 // проверить на свой терминал
 // -1 - нет данных, 0 - ошибка БД
 // 0x1 - есть обьект (по ID), 0x2 - есть обьект (по номеру)
 // 0x4 - необходима запись параметров
 flag = SqModule->FindDevise( (void *)Data );
 if( !flag ) { Socket->Close( ); return; } // 0 - ошибка БД
 if( flag == -1 ) // -1 - нет данных
	{
	if( Data->Pbl.Command == 3 ) // 3 - ORION
		{ SqModule->WriteAllDevise( (void *)Data ); Socket->Close(); return; }
	}
 //-----------------------------------------------------

 TDataModuleTimer *Timer = (TDataModuleTimer *)Socket->Data;
 Timer->Count = 25;

 // 5 - пакет времени
 if( Data->Pbl.Command == 5 )
	{
	SetTime( &Data->Pbl.Date[0] );
	Socket->SendBuf( (void *)Data, sizeof(struct PUBLIC) );
	}
 else if( Data->Pbl.Command == 1 ) // 1 - данные с архива
	{
	if( flag & (1 | 2) ) // 0x1 - есть обьект (по ID), 2 - (по номеру)
		{
		SqModule->WriteTableData( (void *)Data );
		ModuleTCP->OutTableData( (void *)Data );
		}
	}
 else if( Data->Pbl.Command == 2 ) // 2 - последний пакет (закончить соединение)
	{
	if( flag & (1 | 2) ) // 0x1 - есть обьект (по ID), 2 - (по номеру)
		{
		SqModule->WriteTableData( (void *)Data );
		ModuleTCP->OutTableData( (void *)Data );
		}
	if( (flag & 0x4 ) ) // 0x4 - необходима запись параметров
		{
		void *Out = SqModule->SetCustomizationPac( (void *)Data );
		if( !Out ) Socket->Close( ); // прибор не найден -> дать отбой
		else Socket->SendBuf( Out, sizeof(struct OUT_PARAM_TERMINAL) );
		}
	else Socket->Close( ); // дать отбой
	}
 else if( Data->Pbl.Command == 3 ) // 3 - ORION
	{
	SqModule->WriteAllDevise( (void *)&Data );
	if( flag != 2 ) Socket->Close(); // дать отбой
	else // - необходима запись параметров
		{
		void *Out = SqModule->SetCustomizationPac( (void *)&Data );
		if( !Out ) Socket->Close(); // прибор не найден -> дать отбой
		else
			{
			Socket->SendBuf( Out, sizeof(struct OUT_PARAM_TERMINAL) );
			TDataModuleTimer *Timer = (TDataModuleTimer *)Socket->Data;
			Timer->Count = 25;
			}
		}
	}
 // 4 - пакет потверждения настройки
 else if( Data->Pbl.Command == 4 )
	{
	Socket->Close( ); // дать отбой
	SqModule->OkCustomizationPac( (void *)Data ); // снять флаг настройки
	}
 }
//---------------------------------------------------------------------------

void TDataModuleInternet::SetTime( char *realtime )
 {
 TDateTime Date;
 AnsiString AS;

 Date = Date.CurrentDateTime( ); AS = Date.FormatString( "D" );
 realtime[0] = atoi( AS.c_str() ); AS = Date.FormatString( "M" );
 realtime[1] = atoi( AS.c_str() ); AS = Date.FormatString( "yy" );
 realtime[2] = atoi( AS.c_str() );

 AS = Date.FormatString( "H" ); realtime[3] = atoi( AS.c_str() );
 AS = Date.FormatString( "N" ); realtime[4] = atoi( AS.c_str() );
 AS = Date.FormatString( "S" ); realtime[5] = atoi( AS.c_str() );
 }
//==================================================================

