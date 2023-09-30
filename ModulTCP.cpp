//---------------------------------------------------------------------------
#include <time.h>
#include <sys\timeb.h>

#pragma hdrstop

#include "Main.h"
#include "Obgect.h"
#include "ModulTCP.h"
#include "LiteModul.h"
#include "Internet.h"
//#include "Modem.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma classgroup "Vcl.Controls.TControl"
#pragma resource "*.dfm"

TModuleTCP *ModuleTCP;
//---------------------------------------------------------------------------

__fastcall TModuleTCP::TModuleTCP(TComponent* Owner) : TDataModule(Owner)
 {
 // память приёма пакетов TCP
 TCPStream = new TMemoryStream(); TCPStream->Clear( );
 TCPStream->SetSize(0x8000);
 ServerTCP->Active = true; ProcesFlag &= ~0x80;
 }
//---------------------------------------------------------------------------

void __fastcall TModuleTCP::NewConnect(TObject *Sender, TCustomWinSocket *Socket)
 {
 char mess[100];
 TMemoryStream *St;
 struct SOSCEDSTREAM *Struct;

 Struct = new (struct SOSCEDSTREAM);
 memset( Struct, 0, sizeof(struct SOSCEDSTREAM) );
 St = new TMemoryStream( ); St->SetSize( 0x8000 ); Struct->InStream = St;
 St = new TMemoryStream( ); St->SetSize( 0x10000 ); Struct->OutStream = St;
 Socket->Data = Struct;
 sprintf( mess, "Соединение с клиентом %s",
	UtfAsci( 0, Socket->RemoteAddress.c_str() ) ); FormMain->WriteLog( mess );
 }
//---------------------------------------------------------------------------

void __fastcall TModuleTCP::CloseConnect(TObject *Sender, TCustomWinSocket *Socket)
 {
 char mess[100];
 TMemoryStream *St;
 struct SOSCEDSTREAM *Struct;

 Struct = (struct SOSCEDSTREAM *)Socket->Data;
 St = (TMemoryStream *)Struct->InStream; delete St;// St->Free( );
 St = (TMemoryStream *)Struct->OutStream; delete St;// St->Free( );
 sprintf( mess, "Разъединение с клиентом %s",
	UtfAsci( 0, Socket->RemoteAddress.c_str() ) ); FormMain->WriteLog( mess );
 }

//**************************************************************************
//******************************** SERVER TCP ******************************
//**************************************************************************

void TModuleTCP::OutTableData( void *buf )  //????????????????????????
 {
 struct INP_ARH *InputData;
 TDateTime Date;
 static struct
	{
	struct COM Comand; // общая для всех команд
	struct OUT_ARHIV Arhiv; // Таблица архивных данных
	} OutData;

 InputData = (struct INP_ARH *)buf;
 // подготовка шапки запроса
 strcpy( OutData.Comand.Password, "ORION" );
 strcpy( OutData.Comand.Command, "Data" );
 OutData.Comand.Name[0] = 0; OutData.Comand.Fun = 0;
 OutData.Comand.PackedLong = sizeof(struct OUT_ARHIV);

 // подготовка данных
 strcpy( OutData.Arhiv.ID, InputData->Pbl.ID );
 strcpy( OutData.Arhiv.Code, InputData->Pbl.Number );

 // время измерения данных
 OutData.Arhiv.TimerOut = (double)EncodeDate( InputData->Pbl.Date[2] + 2000,
							InputData->Pbl.Date[1], InputData->Pbl.Date[0] );
 OutData.Arhiv.TimerOut += (double)EncodeTime( InputData->Pbl.Date[3],
							InputData->Pbl.Date[4],	InputData->Pbl.Date[5], 0 );
 // время получения данных
 Date = Date.CurrentDateTime( ); OutData.Arhiv.TimerInp = Date.Val;
 // CSQ
 OutData.Arhiv.constCSQ = InputData->Arh.constCSQ;
 // Флаг настроек датчиков
 OutData.Arhiv.FlagSetup.FlagDevise = InputData->Arh.FlagSetup.FlagDevise;
 // Флаг настроек дверей
 OutData.Arhiv.FlagSetup.FlagBit = InputData->Arh.FlagSetup.FlagBit;
 // Флаг ошибок датчиков
 OutData.Arhiv.FlagError.FlagDevise = InputData->Arh.FlagError.FlagDevise;
 // Флаг ошибок дверей
 OutData.Arhiv.FlagError.FlagBit = InputData->Arh.FlagError.FlagBit;
 // данные датчиков
 memcpy( &OutData.Arhiv.Sen, &InputData->Arh.Sen, sizeof(struct SEN) * 16 );

 SetAllMess( (void *)&OutData, sizeof(OUT_ARHIV) ); // размер без шапки
 }

void TModuleTCP::SetAllMess( void *data, int size ) // размер без шапки
 {
 size += sizeof(struct COM);
 for( int a = 0; a < ServerTCP->Socket->ActiveConnections; a++ )
		{ ServerTCP->Socket->Connections[a]->SendBuf( data, size );	}
 }
//---------------------------------------------------------------------------

void __fastcall TModuleTCP::ErrServer(TObject *Sender,
	TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode)
										 { ErrorCode = 0; Socket->Close(); }
//---------------------------------------------------------------------------

void __fastcall TModuleTCP::SocketRead(TObject *Sender,
									TCustomWinSocket *Socket)
 {
 const char cCom[15][10] =
	{
	{ "Test" },  // 0 - Тест
	{ "Server" }, // 1 - запрос данных сервера
	{ "Devise" }, // 2 - запрос данных устройств
	{ "OldData" }, // 3 - получение текущих данных
	{ "SetDev" }, // 4 - запись (удаление) данных устройства
	{ "Ring" },  // 5 - сделать звонок для вызова

	{ "InitArh" }, // 6 - диапазон архива
	{ "DataArh" }, // 7 - данные архива устройств

	{ "NewPar" },  // 8 - получить параметры ожидающие настройки
	{ "DispArh" }, // 9 - данные архива диспетчера
	{ "Conect" }, // 10 - вход в программу пользователя
	{ "Exit" }, // 11 - выход из программы пользователя
	{ "Mess" },  // 12 - любые приходящие данные
	{ "" },  // 13 - включить WEB сервер
	{ "" }  // 14 - выключить WEB сервер
//	{ "OnWEB" },  // 13 - включить WEB сервер
//	{ "OffWEB" }  // 14 - выключить WEB сервер
	};

 struct SOSCEDSTREAM *SocketStream;
 TMemoryStream *InStream, *OutStream;
 static struct COM Com; // общая для всех команд
 int count;
 unsigned char command;

 // проверка длины минимального пакета
 count = Socket->ReceiveLength(); if( !count ) return; // ошибка приёма
 SocketStream = (SOSCEDSTREAM *)Socket->Data;
 InStream = (TMemoryStream *)SocketStream->InStream;
 OutStream = (TMemoryStream *)SocketStream->OutStream;

 TCPStream->Position = 0;
 Socket->ReceiveBuf( TCPStream->Memory, count ); TCPStream->Position = 0;
 if( (SocketStream->ProcesFlag & 0x80) ) // 0x80 - ожидание больших даных (OldData)
	{
	InStream->CopyFrom( TCPStream, count );
	SocketStream->CountPac -= count;	// счётчик приёма больших данных
	// окончание приёма
	if( SocketStream->CountPac <= 0 ) SocketStream->ProcesFlag &= ~0x80;
	else return;
	}
 else
	{
	// проверка на верность длины пакета
	if (count < sizeof(struct COM)) return;
	TCPStream->Read( (void *)&Com, sizeof(struct COM) );
	count -= sizeof(struct COM); InStream->Position = 0;
	// проверка на верность шапки пакета
	if( strncmp(Com.Password, "ORION", 5) ) return;
	if( count ) InStream->CopyFrom( TCPStream, count );
	// получены данные-> проверка на верность длины данных пакета
	if( Com.PackedLong > count ) // много данных
		{
		SocketStream->ProcesFlag |= 0x80;
		SocketStream->CountPac = Com.PackedLong - count;
		return;
		}
	}

 // поиск коматды
 for( command = 0; command < 15; command++ )
	{ if( !strcmp( Com.Command, &cCom[command][0]) ) break; }
 switch( command )
	{
	case 0: // Test
		{
		Com.Fun |= 0x40; Socket->SendBuf( (void *)&Com, sizeof(struct COM) );
		break;
		}
	case 1: // "Server" запрос данных сервера
		{
		if( !Com.Fun ) SetParamServer( Com.Name, Socket ); // чтение
		else // запись
			{
			char IP[20]; UtfAsci( IP, Socket->RemoteAddress.c_str() );
			WriteParamServer( IP, Com.Name, Socket );
			}
		break;
		}
	case 2: // "Devise" запрос данных устройств (поток)
		{ SetAllDevise( Socket ); break; }
	case 3: // "OldData" - получение текущих данных (поток)
		{ SetOldData( Socket ); break; }
	case 4: // "SetDev" запись (удаление) данных устройства
		{
		// данные прибора для записи или удаления (struct TERMINALINFO)
		// необходимо получить из InStream
		struct
			{
			struct COM Pbl; // общая для всех команд
			struct TERMINALINFO Terminal; // информация о приборе
			} Data;

		memset( &Data, 0, sizeof(Data) ); InStream->Position = 0;
		InStream->Read( (void *)&Data.Terminal, sizeof(struct TERMINALINFO) );
		// --- шапка пакета потверждения команды ---
		strcpy( (char *)&Data.Pbl.Password, "ORION" );
		strcpy( (char *)&Data.Pbl.Command, "SetDev" );
		strncpy( (char *)&Data.Pbl.Name, (char *)&Data.Terminal.NameInsert, 50 );
		Data.Pbl.Fun = Com.Fun;
		Data.Pbl.PackedLong = sizeof(struct TERMINALINFO);
		UtfAsci( Data.Terminal.HostInsert, Socket->RemoteAddress.c_str() );
		if( !Com.Fun ) // запись
			{
			if( (SqModule->WriteTableDevise( &Data.Terminal )) < 0 )
													Data.Pbl.Fun = 0x80;
			}
		else // удаление
			{
			if( (SqModule->DeleteDevise( &Data.Terminal )) < 0 )
													Data.Pbl.Fun = 0x81;
			}
		SetAllMess( (void *)&Data, sizeof(struct TERMINALINFO) ); // размер без шапки
		break;
		}
	case 5: // "Ring" сделать звонок для вызова
		{
		struct DEV DevID; // информация о приборе
		InStream->Position = 0;
		InStream->Read( (void *)&DevID, sizeof(struct DEV) );
		SqModule->CommandRing( DevID.Code );
		break;
		}
	case 6: // "InitArh" диапазон архива
		{ SetArh( Com.Name, Socket ); break; }
	case 7: // "DataArh" данные архива устройств (поток)
		{ SetArhData( Com.Name, Socket ); break; }
	case 8: // "NewPar" получить параметры ожидающие настройки
		{ GetNewParam( Socket ); break; }
	case 9: break; // "ArhDisp" данные архива диспетчера
		{ break; }
	case 10: // "Conect" вход в программу пользователя
		{ break; }
	case 11: // "Exit" выход из программы пользователя
		{ break; }
	case 12: // "Mess" любые приходящие данные
		{ break; }
/*
	case 13: // "OnWEB" включить WEB сервер
		{
		STARTUPINFO si = { sizeof(si) };
		PROCESS_INFORMATION pi;
		bool err;
		char *Name = "spider.exe";
		err = CreateProcess(NULL, Name,
			NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
		Com.Fun = 0;
		if( err == true ) { Com.Fun |= 0x43; ReWriteParamServer( 1 ); }
		SetAllMess( (void *)&Com, 0 ); // размер без шапки
		break;
		}
*/
/*
	case 14: // "OffWEB" выключить WEB сервер
		{
		Com.Fun |= 0x40; Socket->SendBuf( (void *)&Com, sizeof(struct COM) );
		break;
		}
*/
	default: return; // нет такой команды
	}
 }
 //---------------------------------------------------------------------------

/**************************************/
/****** Обработчик приёма данных ******/
/**************************************/

// фунции определения промежутка времени (ms)
void TModuleTCP::StartControlTime( unsigned int *ControlTime )
 {
 SYSTEMTIME st;

 GetLocalTime(&st);
 *ControlTime = st.wMilliseconds; *ControlTime += (st.wSecond * 1000);
 *ControlTime += st.wMinute * 1000 * 1000 * 60;
 *ControlTime += st.wHour * 1000 * 1000 * 60 * 60;
 *ControlTime += st.wMonth * 1000 * 1000 * 60 * 60 * 12;
 }

// возращает время в милисекундах
unsigned int TModuleTCP::GetControlTime( unsigned int *ControlTime )
 {
 unsigned int Clock, RetClock;
 SYSTEMTIME st;

 GetLocalTime(&st);
 Clock = st.wMilliseconds;
 Clock += (st.wSecond * 1000); Clock += st.wMinute * 1000 * 1000 * 60;
 Clock += st.wHour * 1000 * 1000 * 60 * 60;
 Clock += st.wMonth * 1000 * 1000 * 60 * 60 * 12;
 RetClock = (Clock - *ControlTime);
 return( RetClock );
 }

//---------------------------------------------------------------------------
//-------------------- Команды ответов на запросы ---------------------------
//---------------------------------------------------------------------------

void TModuleTCP::SetParamServer( char * User, TCustomWinSocket *Socket )
 {
 STARTUPINFO si = { sizeof(si) };
 PROCESS_INFORMATION pi;
 char str[400];
 static struct
	{
	struct COM Pbl; // общая для всех команд
	struct SERVERINFO server; // настройка сервера
	} Data;

 strcpy( (char *)&Data.Pbl.Password, "ORION" );
 strcpy( (char *)&Data.Pbl.Command, "Server" );
 strcpy( (char *)&Data.Pbl.Name, User );
 Data.Pbl.Fun = 0; Data.Pbl.PackedLong = sizeof(struct SERVERINFO); // длина пакета
 memcpy( (void *)&Data.server, (void *)&Server, sizeof(struct SERVERINFO) );

 //    unsigned char OnOffWEB; // 0 - выключить, 1 - включить, 3 - работает
//--------------------------------------------------------
//STARTUPINFO si = { sizeof(si) };
//PROCESS_INFORMATION pi;
//char *xxx = "spider.exe";
//CreateProcess(NULL, xxx, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
//BOOL DestroyWindow(
//  [in] HWND hWnd
// OpenProcessByName("firefox.exe", PROCESS_VM_READ);
//CloseHandle(hFire);
//  ProcessVector processVector;
//  getProcessInformation( processVector );

//DWORD procID = 0;
//procID = GetProcessID("имя процесса.exe"); // имя процесса можно найти в диспетчере задач
//	if (!procID) {
		// Если procID дает результат 0 и процесс не найден выполняем код который вам нужен
//	}
	// в случае успеха продолжаем код//--------------------------------------------------------
/*
 if( !Server.OnOffWEB )
	{

	}
 else if( (Server.OnOffWEB & 1) )
	{
	if( !(Server.OnOffWEB & 2) )
		{
		sprintf( str, "%sServerWinWEB.exe", Patch );
		CreateProcess( NULL, str, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
		}
	}
*/
 Socket->SendBuf( (void *)&Data, sizeof(Data) );
 }

void TModuleTCP::WriteParamServer( char * IP,
			char * User, TCustomWinSocket *Socket )
 {
 struct SOSCEDSTREAM *SocketStream;
 TMemoryStream *InStream;
 struct
	{
	struct COM Pbl; // общая для всех команд
	struct SERVERINFO server; // настройка сервера
	} Data;

 SocketStream = (SOSCEDSTREAM *)Socket->Data;
 InStream = (TMemoryStream *)SocketStream->InStream;
 InStream->Position = 0;
// получение данных
 InStream->Read( (void *)&Data.server, sizeof(struct SERVERINFO) );
 strcpy( Data.server.HostInsert, IP );

 Data.Pbl.Fun = SqModule->WriteTableServis( (void *)&Data.server );
 // Проверка настроек БД (новые данные записаннны в Server)
// if( Server.Base.DBFlag ) ModuleMySQL->SetConectDB( );
// else ModuleMySQL->CloseDB( ); // окончание работы с БД
 // ответка
 SqModule->ReadTableServis( (void *)&Data.server );
 strcpy( (char *)&Data.Pbl.Password, "ORION" );
 strcpy( (char *)&Data.Pbl.Command, "Server" );
 strcpy( (char *)&Data.Pbl.Name, User );
 Data.Pbl.PackedLong = sizeof(struct SERVERINFO); // длина пакета
 SetAllMess( (void *)&Data, sizeof(struct SERVERINFO) ); // размер без шапки
 // перезапуск
 DataModuleInternet->ServerInternet->Active = false;
 DataModuleInternet->ServerInternet->Port = Server.IntPortServer;
 DataModuleInternet->ServerInternet->Active = true;
 }

void TModuleTCP::GetNewParam( TCustomWinSocket *Socket )
 {
 struct SOSCEDSTREAM *SocketStream;
 TMemoryStream *InStream, *OutStream;
 struct DEV terminal;
 void *Out;
 struct
	{
	struct COM Pbl; // общая для всех команд
	struct NEWPAR NewData;
	} Data;

 SocketStream = (SOSCEDSTREAM *)Socket->Data;
 InStream = (TMemoryStream *)SocketStream->InStream;
 InStream->Position = 0;
 // получение данных
 InStream->Read( (void *)&terminal, sizeof(struct DEV) );
 Out = SqModule->GetNewParam( (void *)&terminal );

 strcpy( (char *)&Data.Pbl.Password, "ORION" );
 strcpy( (char *)&Data.Pbl.Command, "NewPar" );
 Data.Pbl.Name[0] = 0; Data.Pbl.Fun = 0;
 Data.Pbl.PackedLong = sizeof(struct NEWPAR);
 memcpy( &Data.NewData, Out, sizeof(struct NEWPAR) );
 Socket->SendBuf( (void *)&Data, sizeof(Data) );
 }

void TModuleTCP::SetAllDevise( TCustomWinSocket *Socket )
 {
 struct COM Pbl; // общая для всех команд
 struct TERMINALINFO Devise; // информация о приборе
 AnsiString AS;
 TByteDynArray blob;
 TDateTime Date;
 int count;
 struct SOSCEDSTREAM *SocketStream;
 TMemoryStream *Stream;

 SocketStream = (SOSCEDSTREAM *)Socket->Data;
 Stream = (TMemoryStream *)SocketStream->OutStream;

 count = sizeof(struct COM);
 strcpy( (char *)&Pbl.Password, "ORION" );
 strcpy( (char *)&Pbl.Command, "Devise" );
 Pbl.Name[0] = 0; Pbl.Fun = 0; Pbl.PackedLong = 0;
 Stream->Position = 0; Stream->Write( (void *)&Pbl, sizeof(struct COM) );

 SqModule->LiteQuery->SQL->Clear();
 SqModule->LiteQuery->SQL->Add( "select * from `TableDevise`" );
 try { SqModule->LiteQuery->Open(); }
 catch (...) { goto ex; }
 if( !SqModule->LiteQuery->RecordCount ) { goto ex; }

 blob.set_length(sizeof(struct PARAMETRS_TERMINAL)); // длина пакета
 SqModule->LiteQuery->First(); // установить первую запись
 for( ; SqModule->LiteQuery->Eof == false; SqModule->LiteQuery->Next() )
	{
	memset( (void *)&Devise, 0, sizeof(struct TERMINALINFO) );
	Devise.TabNo = SqModule->LiteQuery->Fields->FieldByName("TabNo")->AsInteger;
	AS = SqModule->LiteQuery->Fields->FieldByName("Identifier")->AsString;
	strcpy( Devise.ID, AS.c_str() );
	AS = SqModule->LiteQuery->Fields->FieldByName("Code")->AsString;
	strcpy( Devise.Code, AS.c_str() );
	Devise.VerDevise = SqModule->LiteQuery->Fields->
							FieldByName("VerDevise")->AsInteger;
	Devise.SaveFlag =
		SqModule->LiteQuery->Fields->FieldByName("SaveFlag")->AsInteger;

	Date = SqModule->LiteQuery->Fields->
				FieldByName("TimerInsert")->AsDateTime;
	Devise.TimerInsert = Date.Val;

	AS = SqModule->LiteQuery->Fields->FieldByName("HostInsert")->AsString;
	strcpy( Devise.HostInsert, AS.c_str() );
	AS = SqModule->LiteQuery->Fields->FieldByName("NameInsert")->AsString;
	strcpy( Devise.NameInsert, AS.c_str() );

	Devise.TypeDevise = SqModule->LiteQuery->Fields->FieldByName("TypeDevise")->AsInteger;
	AS = SqModule->LiteQuery->Fields->FieldByName("Name")->AsString;
	strcpy( Devise.Name, AS.c_str() );
	AS = SqModule->LiteQuery->Fields->FieldByName("Region")->AsString;
	strcpy( Devise.Region, AS.c_str() );
	AS = SqModule->LiteQuery->Fields->FieldByName("DistrictCity")->AsString;
	strcpy( Devise.DistrictCity, AS.c_str() );
	AS = SqModule->LiteQuery->Fields->FieldByName("Addres")->AsString;
	strcpy( Devise.Addres, AS.c_str() );
	Devise.KordinateDevise.Lat =
		SqModule->LiteQuery->Fields->FieldByName("Lat")->AsFloat;
	Devise.KordinateDevise.Lon =
		SqModule->LiteQuery->Fields->FieldByName("Long")->AsFloat;

	memset( &Devise.Parametrs, 0, sizeof(struct PARAMETRS_TERMINAL) );
	blob = SqModule->LiteQuery->Fields->FieldByName("Parametrs")->AsBytes;
	memcpy( (void *)&Devise.Parametrs, (void *)&blob[0],
						sizeof(struct PARAMETRS_TERMINAL) );

	Stream->Write( (void *)&Devise, sizeof(struct TERMINALINFO) );
	count += sizeof(struct TERMINALINFO);
	Pbl.PackedLong += sizeof(struct TERMINALINFO);
	}
ex:
 Stream->Position = 0; Stream->Write( (void *)&Pbl, sizeof(struct COM) );
 Socket->SendBuf( Stream->Memory, count );
 }

// данные каких приборов необходимо получить в InStream
void TModuleTCP::SetOldData( TCustomWinSocket *Socket )
 {
 int AllCount, count;
 char str[100];
 struct DEV Key;
 AnsiString AS;
 struct COM Pbl; // общая для всех команд
 struct OUT_ARHIV Data;// Таблица архивных данных
 TDateTime Date;
 int Long;
 struct SOSCEDSTREAM *SocketStream;
 TMemoryStream *InStream, *OutStream;

 SocketStream = (SOSCEDSTREAM *)Socket->Data;
 InStream = (TMemoryStream *)SocketStream->InStream;
 OutStream = (TMemoryStream *)SocketStream->OutStream;

 Long = sizeof(struct COM);
 strcpy( (char *)&Pbl.Password, "ORION" );
 strcpy( (char *)&Pbl.Command, "OldData" );
 Pbl.Name[0] = 0; Pbl.Fun = 0; Pbl.PackedLong = 0;
 AllCount = InStream->Position;
 InStream->Position = 0; OutStream->Position = 0;
 OutStream->Write( (void *)&Pbl, sizeof(struct COM) );

 for( ; AllCount; )
	{
	count = InStream->Read( (void *)&Key, sizeof(struct DEV) );
	if( count < sizeof(struct DEV) ) break; // больше нет данных
	AllCount -= sizeof(struct DEV);
	// Постоянные данные
	SqModule->LiteQuery->SQL->Clear();
	SqModule->LiteQuery->SQL->Add( "select * from `TableDataGRP`" );
	if( Key.ID[0] ) sprintf( str, " where `Identifier` = '%s';", Key.ID );
	else sprintf( str, " where `Code` = '%s';", Key.Code );
	SqModule->LiteQuery->SQL->Add( str );
	try { SqModule->LiteQuery->Open(); }
	catch (...) { goto ex; }
	// устройство не найдено
	if( SqModule->LiteQuery->Eof == true ) continue;
	SqModule->LiteQuery->Last(); // указатель в конец
	AS = SqModule->LiteQuery->Fields->FieldByName("Identifier")->AsString;
	strcpy( Data.ID, AS.c_str() );
	AS = SqModule->LiteQuery->Fields->FieldByName("Code")->AsString;
	strcpy( Data.Code, AS.c_str() );

	Date = SqModule->LiteQuery->Fields->
				FieldByName("TimerOut")->AsDateTime;
	Data.TimerOut = Date.Val;
	Date = SqModule->LiteQuery->Fields->
				FieldByName("TimerInp")->AsDateTime;
	Data.TimerInp = Date.Val;

	Data.constCSQ = SqModule->LiteQuery->Fields->DataSet->
							FieldByName("constCSQ")->AsInteger;
	Data.FlagSetup.FlagDevise = SqModule->LiteQuery->Fields->DataSet->
								FieldByName("SensorSetup")->AsInteger;
	Data.FlagSetup.FlagBit = SqModule->LiteQuery->Fields->DataSet->
								FieldByName("BitSetup")->AsInteger;
	Data.FlagError.FlagDevise = SqModule->LiteQuery->Fields->DataSet->
								FieldByName("SensorError")->AsInteger;
	Data.FlagError.FlagBit = SqModule->LiteQuery->Fields->DataSet->
								FieldByName("BitError")->AsInteger;
	Data.Sen[0].data = SqModule->LiteQuery->Fields->DataSet->FieldByName("AK")->AsFloat;
	Data.Sen[0].flag = SqModule->LiteQuery->Fields->DataSet->FieldByName("FlAK")->AsInteger;
	Data.Sen[1].data = SqModule->LiteQuery->Fields->DataSet->FieldByName("P_1")->AsFloat;
	Data.Sen[1].flag = SqModule->LiteQuery->Fields->DataSet->FieldByName("FlP_1")->AsInteger;
	Data.Sen[2].data = SqModule->LiteQuery->Fields->DataSet->FieldByName("P_3")->AsFloat;
	Data.Sen[2].flag = SqModule->LiteQuery->Fields->DataSet->FieldByName("FlP_3")->AsInteger;
	// 3 - Загазованность (0x8)
	Data.Sen[3].data = SqModule->LiteQuery->Fields->DataSet->FieldByName("CnHm_1")->AsFloat;
	Data.Sen[3].flag = SqModule->LiteQuery->Fields->DataSet->FieldByName("FlCnHm_1")->AsInteger;
	// 4 - ПСК 1 (0x10)
	Data.Sen[4].data = SqModule->LiteQuery->Fields->DataSet->FieldByName("PSK_1")->AsFloat;
	Data.Sen[4].flag = SqModule->LiteQuery->Fields->DataSet->FieldByName("FlPSK_1")->AsInteger;
	// 5 - Температура 1 (0x20)
	Data.Sen[5].data = SqModule->LiteQuery->Fields->DataSet->FieldByName("T_1")->AsFloat;
	Data.Sen[5].flag = SqModule->LiteQuery->Fields->DataSet->FieldByName("FlT_1")->AsInteger;
	// 6 - Температура 2 (0x40)
	Data.Sen[6].data = SqModule->LiteQuery->Fields->DataSet->FieldByName("T_2")->AsFloat;
	Data.Sen[6].flag = SqModule->LiteQuery->Fields->DataSet->FieldByName("FlT_2")->AsInteger;
	// 7 - потанциал катодной защиты (0x80)
	Data.Sen[7].data = SqModule->LiteQuery->Fields->DataSet->FieldByName("Ektz")->AsFloat;
	Data.Sen[7].flag = SqModule->LiteQuery->Fields->DataSet->FieldByName("FlEktz")->AsInteger;
	// P_2  Давление Р2 (0x100)
	Data.Sen[8].data = SqModule->LiteQuery->Fields->DataSet->FieldByName("P_2")->AsFloat;
	Data.Sen[8].flag = SqModule->LiteQuery->Fields->DataSet->FieldByName("FlP_2")->AsInteger;
	// 9 - ПСК 2 (0x200)
	Data.Sen[9].data = SqModule->LiteQuery->Fields->DataSet->FieldByName("PSK_2")->AsFloat;
	Data.Sen[9].flag = SqModule->LiteQuery->Fields->DataSet->FieldByName("FlPSK_2")->AsInteger;
	// Rez_AK  Внешний акумулятор (0x400)
	Data.Sen[10].data = SqModule->LiteQuery->Fields->DataSet->FieldByName("Rez_AK")->AsFloat;
	Data.Sen[10].flag = SqModule->LiteQuery->Fields->DataSet->FieldByName("FlRez_AK")->AsInteger;
	// ток катодной защиты (0x1000)
	Data.Sen[11].data = SqModule->LiteQuery->Fields->DataSet->FieldByName("IKZ")->AsFloat;
	Data.Sen[11].flag = SqModule->LiteQuery->Fields->DataSet->FieldByName("FlIKZ")->AsInteger;
	// напряжение катодной защиты (0x2000)
	Data.Sen[12].data = SqModule->LiteQuery->Fields->DataSet->FieldByName("UKZ")->AsFloat;
	Data.Sen[12].flag = SqModule->LiteQuery->Fields->DataSet->FieldByName("FlUKZ")->AsInteger;

	Data.Sen[13].data = 0; Data.Sen[13].flag = 0;
	Data.Sen[14].data = 0; Data.Sen[14].flag = 0;
	Data.Sen[15].data = 0; Data.Sen[15].flag = 0;

	OutStream->Write( (void *)&Data, sizeof(struct OUT_ARHIV) );
	Pbl.PackedLong += sizeof(struct OUT_ARHIV);
	Long += sizeof(struct OUT_ARHIV);
	}
ex:
 OutStream->Position = 0; OutStream->Write( (void *)&Pbl, sizeof(struct COM) );
 Socket->SendBuf( OutStream->Memory, Long );
 return;
 }
//---------------------------------------------------------------------------

void TModuleTCP::SetArh( char *Name, TCustomWinSocket *Socket )
 {
 TDateTime Date;
 char str[100];
 static struct OUTARH Data;
 struct SOSCEDSTREAM *SocketStream;
 TMemoryStream *InStream;

 SocketStream = (SOSCEDSTREAM *)Socket->Data;
 InStream = (TMemoryStream *)SocketStream->InStream;
 InStream->Position = 0;
 InStream->Read( (void *)&Data.Param, sizeof(struct INITARH) );
//868404041758090
 // --- шапка пакета потверждения команды ---
 strcpy( (char *)&Data.Pbl.Password, "ORION" );
 strcpy( (char *)&Data.Pbl.Command, "InitArh" );
 strcpy( Data.Pbl.Name, Name ); Data.Pbl.Fun = 0x80;
 Data.Pbl.PackedLong = sizeof(struct INITARH);
 Data.Param.StartTime = 0; Data.Param.EngTime = 0;

 SqModule->LiteQuery->SQL->Clear();
 SqModule->LiteQuery->SQL->Add( "select * from `TableDataGRP`" );
 if( Data.Param.ID[0] ) sprintf( str, " where `Identifier` = '%s';", Data.Param.ID );
 else sprintf( str, " where `Code` = '%s';", Data.Param.Code );
 SqModule->LiteQuery->SQL->Add( str );
 try { SqModule->LiteQuery->Open(); }
 catch (...)
	{ Socket->SendBuf( (void *)&Data, sizeof(struct OUTARH) ); return; }
 Data.Pbl.Fun = 0;
 if( SqModule->LiteQuery->Eof == true )
	{ Socket->SendBuf( (void *)&Data, sizeof(struct OUTARH) ); return; }
 SqModule->LiteQuery->First(); // указатель в начало
 Date = SqModule->LiteQuery->Fields->DataSet->
								FieldByName("TimerOut")->AsDateTime;
 Data.Param.StartTime = Date.Val;
 SqModule->LiteQuery->Last(); // указатель в конец
 Date = SqModule->LiteQuery->Fields->DataSet->
								FieldByName("TimerOut")->AsDateTime;
 Data.Param.EngTime = Date.Val;
 Socket->SendBuf( (void *)&Data, sizeof(struct OUTARH) );
 }
//---------------------------------------------------------------------------

void TModuleTCP::SetArhData( char *Name, TCustomWinSocket *Socket )
 {
 TDateTime Date;
 char str[100];
 struct OUTARH Out;
 struct DATAARH Data; // архивные данные
 int count;
 struct SOSCEDSTREAM *SocketStream;
 TMemoryStream *InStream, *OutStream;

 SocketStream = (SOSCEDSTREAM *)Socket->Data;
 InStream = (TMemoryStream *)SocketStream->InStream;
 OutStream = (TMemoryStream *)SocketStream->OutStream;
 InStream->Position = 0; OutStream->Position = 0;

 InStream->Read( (void *)&Out.Param, sizeof(struct INITARH) );
 // --- шапка пакета потверждения команды ---
 strcpy( (char *)&Out.Pbl.Password, "ORION" );
 strcpy( (char *)&Out.Pbl.Command, "DataArh" );
 strcpy( Out.Pbl.Name, Name ); Out.Pbl.Fun = 0x80; // ошибка
 Out.Pbl.PackedLong = sizeof(struct INITARH); // информация о приборе
 count = sizeof(struct OUTARH); // информация о приборе
 // --- информация о архиве ---
 OutStream->Write( (void *)&Out, sizeof(OUTARH) );

 SqModule->LiteQuery->SQL->Clear();
 SqModule->LiteQuery->SQL->Add( "select * from `TableDataGRP`" );
 if( Out.Param.ID[0] ) sprintf( str, " where `Identifier` = '%s';", Out.Param.ID );
 else sprintf( str, " where `Code` = '%s';", Out.Param.Code );
 SqModule->LiteQuery->SQL->Add( str );
 try { SqModule->LiteQuery->Open(); }
 catch (...) { Socket->SendBuf( OutStream->Memory, count ); return; }
 Out.Pbl.Fun = 0;
 if( SqModule->LiteQuery->Eof == true )
	{ Socket->SendBuf( OutStream->Memory, count ); return; }

 // --- данные архива ---
 SqModule->LiteQuery->First(); // установить первую запись
 for( ; SqModule->LiteQuery->Eof == false; SqModule->LiteQuery->Next() )
	{
	Date = SqModule->LiteQuery->Fields->DataSet->
								FieldByName("TimerOut")->AsDateTime;
	if( Date.Val < Out.Param.StartTime ) continue;
	if( Date.Val > Out.Param.EngTime ) break;
	Data.TimerOut = Date.Val;
	Date = SqModule->LiteQuery->Fields->DataSet->
								FieldByName("TimerInp")->AsDateTime;
	Data.TimerInp = Date.Val;
	Data.constCSQ = SqModule->LiteQuery->Fields->DataSet->
									FieldByName("constCSQ")->AsInteger;
	Data.FlagSetup.FlagDevise = SqModule->LiteQuery->Fields->DataSet->
									FieldByName("SensorSetup")->AsInteger;
	Data.FlagSetup.FlagBit = SqModule->LiteQuery->Fields->DataSet->
									FieldByName("BitSetup")->AsInteger;
	Data.FlagError.FlagDevise = SqModule->LiteQuery->Fields->DataSet->
									FieldByName("SensorError")->AsInteger;
	Data.FlagError.FlagBit = SqModule->LiteQuery->Fields->DataSet->
									FieldByName("BitError")->AsInteger;
	Data.Sen[0].data =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("AK")->AsFloat;
	Data.Sen[0].flag =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("FlAK")->AsInteger;
	Data.Sen[1].data =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("P_1")->AsFloat;
	Data.Sen[1].flag =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("FlP_1")->AsInteger;
	Data.Sen[2].data =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("P_3")->AsFloat;
	Data.Sen[2].flag =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("FlP_3")->AsInteger;
	// 3 - Загазованность (0x8)
	Data.Sen[3].data =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("CnHm_1")->AsFloat;
	Data.Sen[3].flag =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("FlCnHm_1")->AsInteger;
	// 4 - ПСК 1 (0x10)
	Data.Sen[4].data =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("PSK_1")->AsFloat;
	Data.Sen[4].flag =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("FlPSK_1")->AsInteger;
	// 5 - Температура 1 (0x20)
	Data.Sen[5].data =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("T_1")->AsFloat;
	Data.Sen[5].flag =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("FlT_1")->AsInteger;
	// 6 - Температура 2 (0x40)
	Data.Sen[6].data =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("T_2")->AsFloat;
	Data.Sen[6].flag =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("FlT_2")->AsInteger;
	// 7 - потанциал катодной защиты (0x80)
	Data.Sen[7].data =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("Ektz")->AsFloat;
	Data.Sen[7].flag =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("FlEktz")->AsInteger;
	// P_2  Давление Р2 (0x100)
	Data.Sen[8].data =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("P_2")->AsFloat;
	Data.Sen[8].flag =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("FlP_2")->AsInteger;
	// 9 - ПСК 2 (0x200)
	Data.Sen[9].data =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("PSK_2")->AsFloat;
	Data.Sen[9].flag =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("FlPSK_2")->AsInteger;
	// Rez_AK  Внешний акумулятор (0x400)
	Data.Sen[10].data =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("Rez_AK")->AsFloat;
	Data.Sen[10].flag =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("FlRez_AK")->AsInteger;
	// ток катодной защиты (0x1000)
	Data.Sen[11].data =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("IKZ")->AsFloat;
	Data.Sen[11].flag =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("FlIKZ")->AsInteger;
	// напряжение катодной защиты (0x2000)
	Data.Sen[12].data =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("UKZ")->AsFloat;
	Data.Sen[12].flag =
		SqModule->LiteQuery->Fields->DataSet->FieldByName("FlUKZ")->AsInteger;
	Data.Sen[13].data = 0; Data.Sen[13].flag = 0;
	Data.Sen[14].data = 0; Data.Sen[14].flag = 0;
	Data.Sen[15].data = 0; Data.Sen[15].flag = 0;

	OutStream->Write( (void *)&Data, sizeof(struct DATAARH) );
	Out.Pbl.PackedLong += sizeof(struct DATAARH);
	count += sizeof(struct DATAARH);
	}

 OutStream->Position = 0; OutStream->Write( (void *)&Out.Pbl, sizeof(struct COM) );
 Socket->SendBuf( OutStream->Memory, count );
 }
//---------------------------------------------------------------------------


