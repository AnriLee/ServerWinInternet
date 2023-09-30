#pragma hdrstop
#include <stdio.h>
#include <time.h>
#include <sys\timeb.h>

#include "Main.h"
#include "Obgect.h"
#include "LiteModul.h"
#include "Internet.h"
#include "ModulTCP.h"
//#include "Modem.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma classgroup "Vcl.Controls.TControl"
#pragma resource "*.dfm"

#define TimerControl 20
//-------------- Host ------------------
#define SetHOST "37.57.78.213" // по умолчанию (ORION)
//#define SetHOST "77.122.84.42" // Volia
//#define SetHOST "185.200.061.167" // Кобеляки
//-------------- Port ------------------
#define SetPort 49050  // Orion

extern void WriteTelegram( void *buf );
TSqModule *SqModule;

//---------------------------------------------------------------------------

__fastcall TSqModule::TSqModule(TComponent* Owner) : TDataModule(Owner)
 {
 LiteConnection->Connected = true;
 OpenTableServis( ); ReadTableServis( (void *)&Server );
 OpenTableDevise( ); OpenTableData( );
 OpenTableWaitRing( ); // ожидание звонка
 OpenTableAllDevise( ); // Создание таблицы AllDevise
 ControlData( );
 }

//==========================================
//============ НАСТРОЙКА ТАБЛИЦ ============
//==========================================

void TSqModule::OpenTableServis( void )
 {
 struct SERVERINFO server; // настройка сервера

 LiteQuery->SQL->Clear( );
 LiteQuery->SQL->Add( "select * from `TableServis`" );
 try { LiteQuery->Open(); }
 catch (...)
	{
	LiteQuery->SQL->Clear();
	LiteQuery->SQL->Add( "CREATE TABLE IF NOT EXISTS `TableServis` (" );
	// IP или Host сервера
	LiteQuery->SQL->Add( "HostServer varchar," );
	// Порт сервера для настройки обьектов (external внешний)
	LiteQuery->SQL->Add( "ExtPortServer integer," );
	// Локальный порт сервера (interior внутренний)
	LiteQuery->SQL->Add( "IntPortServer integer," );
	// Пароль администратора
	LiteQuery->SQL->Add( "AdminPassword varchar," );
	// время хранения данных
	LiteQuery->SQL->Add( "TimerSaveDB integer,"); // в днях
	LiteQuery->SQL->Add( "OnOffWEB integer,"); // 0 - выключить, 1 - включить, 3 - работает
	// IP или Host сервера изменения данных
	LiteQuery->SQL->Add( "HostInsert varchar," );
	// имя пользователя изменения данных
	LiteQuery->SQL->Add( "NameInsert varchar," );
	// время изменения данных
	LiteQuery->SQL->Add( "TimerInsert datatime);" );
	try { LiteQuery->Execute(); }
	catch (...) { return; }
	// ========== настройки сервера ==========
	strcpy( server.HostServer, SetHOST );
	server.ExtPortServer = SetPort;
	server.IntPortServer = SetPort;
	strcpy( server.AdminPassword, "admin" );
	server.TimerSaveDB = 21;
	server.HostInsert[0] = 0; server.NameInsert[0] = 0;
	WriteTableServis( (void *)&server );
	}
 }

void TSqModule::OpenTableDevise( void )
 {
 LiteQuery->SQL->Clear();
 LiteQuery->SQL->Add( "CREATE TABLE IF NOT EXISTS `TableDevise` (" );
 LiteQuery->SQL->Add( "TabNo INTEGER PRIMARY KEY AUTOINCREMENT, " );
 LiteQuery->SQL->Add( "Identifier varchar," ); // ID прибора
 LiteQuery->SQL->Add( "Code varchar," ); // Номер прибора (выбор пользователя)
 LiteQuery->SQL->Add( "VerDevise integer default 0," ); // Версия ПО (версия устройства)
 LiteQuery->SQL->Add( "TimerInsert datatime,"); // время изменения данных
 LiteQuery->SQL->Add( "HostInsert varchar," ); // IP или Host сервера изменения данных
 LiteQuery->SQL->Add( "NameInsert varchar," ); // имя пользователя изменения данных
 LiteQuery->SQL->Add( "SaveFlag integer default 0,"); // флаг изменения данных
 LiteQuery->SQL->Add( "TypeDevise integer default 64,");
 LiteQuery->SQL->Add( "Name varchar default ''," );
 LiteQuery->SQL->Add( "Region varchar default ''," ); // область
 LiteQuery->SQL->Add( "DistrictCity varchar default ''," ); // район
 LiteQuery->SQL->Add( "Addres varchar default ''," );
 LiteQuery->SQL->Add( "Long float(10,4) default 0," );
 LiteQuery->SQL->Add( "Lat float(10,4) default 0," );
 LiteQuery->SQL->Add( "Parametrs blob);" ); // параметры приьора (BLOB)
 try { LiteQuery->Execute(); }
 catch (...) { }
 // Создание таблицы TableDeviseWait
 LiteQuery->SQL->Clear();
 LiteQuery->SQL->Add( "CREATE TABLE IF NOT EXISTS `TableDeviseWait` (" );
 LiteQuery->SQL->Add( "Identifier varchar not null," );
 LiteQuery->SQL->Add( "Code varchar not null," );
 LiteQuery->SQL->Add( "TypeDevise integer default 0,");
 LiteQuery->SQL->Add( "Parametrs BLOB);" ); // параметры приьора (BLOB)
 try { LiteQuery->Execute(); }
 catch (...) { return; }
 }

void TSqModule::OpenTableWaitRing( void ) // ожидание звонка
 {
 LiteQuery->SQL->Clear();
 LiteQuery->SQL->Add( "CREATE TABLE IF NOT EXISTS `TableWaitRing` (" );
 LiteQuery->SQL->Add( "Code varchar," );
 LiteQuery->SQL->Add( "Count integer default 1);");
 try { LiteQuery->Execute(); }
 catch (...) { return; }
 }

void TSqModule::OpenTableAllDevise( void ) // Создание таблицы AllDevise  (3 команда)
 {
 LiteQuery->SQL->Clear();
 LiteQuery->SQL->Add( "CREATE TABLE IF NOT EXISTS `TableAllDevise` (" );
 LiteQuery->SQL->Add( "Identifier varchar primari key," );
 LiteQuery->SQL->Add( "Code varchar not null," );
 LiteQuery->SQL->Add( "TimerInsert datatime,"); // время записи данных
 LiteQuery->SQL->Add( "TypeDevise integer default 0,");
 LiteQuery->SQL->Add( "VerDevise integer default 0," ); // Версия ПО (версия устройства)
 LiteQuery->SQL->Add( "HostServer varchar," ); // адрес сервера
 LiteQuery->SQL->Add( "PortServer integer default 0," ); // порт сервера
 LiteQuery->SQL->Add( "Long float(10,4) default 0," );
 LiteQuery->SQL->Add( "Lat float(10,4) default 0," );
 LiteQuery->SQL->Add( "SensorSetup integer default 1,"); // флаг настроек датчиков
 LiteQuery->SQL->Add( "BitSetup integer default 0,"); // флаг настроек битовых полей
 LiteQuery->SQL->Add( "TimerArh integer default 0,"); // таймер архивации данных (в мин.)
 LiteQuery->SQL->Add( "TimerCon integer default 0,"); // таймер выхода на связь (в мин.)
 LiteQuery->SQL->Add( "lPowerTimer integer default 0,"); // период обработки мало мощных датчиков (в мин.)
 LiteQuery->SQL->Add( "hPowerTimer integer default 0,"); // период обработки мощных датчиков (в мин.)
 LiteQuery->SQL->Add( "Sensor BLOB," ); // данные датчиков (BLOB)
 LiteQuery->SQL->Add( "DopPor integer default 0);");

 try { LiteQuery->Execute(); }
 catch (...) { return; }
 }

void TSqModule::OpenTableData( void )
 {
 LiteQuery->SQL->Clear();
 LiteQuery->SQL->Add( "CREATE TABLE IF NOT EXISTS `TableDataGRP` (" );

 LiteQuery->SQL->Add( "Identifier varchar default ' '," );
 LiteQuery->SQL->Add( "Code varchar default ' '," );
 LiteQuery->SQL->Add( "`TimerOut` datatime,"); // время измерения данных
 LiteQuery->SQL->Add( "`TimerInp` datatime,"); // время получения данных
 LiteQuery->SQL->Add( "`constCSQ` integer default 0,"); // уровень связи
 LiteQuery->SQL->Add( "`SensorSetup` integer default 1,"); // флаг настроек датчиков
 LiteQuery->SQL->Add( "`BitSetup` integer default 0,"); // флаг настроек битовых полей
 LiteQuery->SQL->Add( "`SensorError` integer default 0,"); // флаг ошибок датчиков
 LiteQuery->SQL->Add( "`BitError` integer default 0,"); // флаг ошибок битовых полей

 // --- датчики ---
 // 1 - Давление Р1 (0x2)
 LiteQuery->SQL->Add( "`P_1` float(10,2) default 0," );
 LiteQuery->SQL->Add( "`FlP_1` integer default 0," );
 // 8 - Давление Р2 (0x100)
 LiteQuery->SQL->Add( "`P_2` float(10,2) default 0," );
 LiteQuery->SQL->Add( "`FlP_2` integer default 0," );
 // 2 - Давление Р3 (0x4)
 LiteQuery->SQL->Add( "`P_3` float(10,2) default 0," );
 LiteQuery->SQL->Add( "`FlP_3` integer default 0," );
 // 4 - ПСК 1 (0x10)
 LiteQuery->SQL->Add( "`PSK_1` float(10,2) default 0," );
 LiteQuery->SQL->Add( "`FlPSK_1` integer default 0," );
 // 9 - ПСК 2 (0x200)
 LiteQuery->SQL->Add( "`PSK_2` float(10,2) default 0," );
 LiteQuery->SQL->Add( "`FlPSK_2` integer default 0," );
 // 3 - Загазованность (0x8)
 LiteQuery->SQL->Add( "`CnHm_1` float(10,2) default 0," );
 LiteQuery->SQL->Add( "`FlCnHm_1` integer default 0," );
 // 5 - Температура 1 (0x20)
 LiteQuery->SQL->Add( "`T_1` float(10,2) default 0," );
 LiteQuery->SQL->Add( "`FlT_1` integer default 0," );
 // 6 - Температура 2 (0x40)
 LiteQuery->SQL->Add( "`T_2` float(10,2) default 0," );
 LiteQuery->SQL->Add( "`FlT_2` integer default 0," );
 // 7 - потанциал катодной защиты (0x80)
 LiteQuery->SQL->Add( "`Ektz` float(10,2) default 0," );
 LiteQuery->SQL->Add( "`FlEktz` integer default 0," );
 // 11 - ток катодной защиты (0x1000)
 LiteQuery->SQL->Add( "`IKZ` float(10,2) default 0," );
 LiteQuery->SQL->Add( "`FlIKZ` integer default 0," );
 // 12 - напряжение катодной защиты (0x2000)
 LiteQuery->SQL->Add( "`UKZ` float(10,2) default 0," );
 LiteQuery->SQL->Add( "`FlUKZ` integer default 0," );
 // ~220
 LiteQuery->SQL->Add( "`U_220` varchar(10) default ' '," );
 // 0 - Внутрений акумулятор (0x1)
 LiteQuery->SQL->Add( "`AK` float(10,2) default 0," );
 LiteQuery->SQL->Add( "`FlAK` integer default 0," );
 // 10 - Внешний акумулятор (0x400)
 LiteQuery->SQL->Add( "`Rez_AK` float(10,2) default 0," );
 LiteQuery->SQL->Add( "`FlRez_AK` integer default 0," );

 // --- битовые поля ---
 LiteQuery->SQL->Add( "`PZK_1` varchar(10) default ' '," ); // 0x02 - ПЗК 1
 LiteQuery->SQL->Add( "`PZK_2` varchar(10) default ' '," ); // 0x200 - ПЗК 2
 LiteQuery->SQL->Add( "`DV_1` varchar(10) default ' '," ); // 0x04 - Дв. 1
 LiteQuery->SQL->Add( "`DV_2` varchar(10) default ' '," ); // 0x08 - Дв. 2
 LiteQuery->SQL->Add( "`DV_3` varchar(10) default ' '," ); // 0x10 - Дв. 3
 LiteQuery->SQL->Add( "`DV_4` varchar(10) default ' ');" ); // 0x20 - Дв. 4

 try { LiteQuery->Execute(); }
 catch (...) { return; }
 }

//=======================================
//============ ЗАПИСЬ ТАБЛИЦ ============
//=======================================

int TSqModule::WriteTableServis( void *buf )
 {
 struct SERVERINFO *Data; // настройка сервера
 TDateTime Date;
 char str[50];

 LiteQuery->SQL->Clear();
 LiteQuery->SQL->Add( "select * from `TableServis`" );
 try { LiteQuery->Open( ); }
 catch (...) { FormMain->WriteLog( "Ошибка записи TableServis" ); return 0x81; }

 Data = (struct SERVERINFO *)buf;
 LiteQuery->Edit();
 LiteQuery->Fields->DataSet->FieldByName("HostServer")->AsString = Data->HostServer;
 LiteQuery->Fields->DataSet->FieldByName("ExtPortServer")->AsInteger = Data->ExtPortServer;
 LiteQuery->Fields->DataSet->FieldByName("IntPortServer")->AsInteger = Data->IntPortServer;
 LiteQuery->Fields->DataSet->FieldByName("AdminPassword")->AsString = Data->AdminPassword;
 LiteQuery->Fields->DataSet->FieldByName("TimerSaveDB")->AsInteger = Data->TimerSaveDB;
// LiteQuery->Fields->DataSet->FieldByName("OnOffWEB")->AsInteger = Data->OnOffWEB;
 LiteQuery->Fields->DataSet->FieldByName("HostInsert")->AsString = Data->HostInsert;
 LiteQuery->Fields->DataSet->FieldByName("NameInsert")->AsString = Data->NameInsert;
 Date = Date.CurrentDateTime( );
 LiteQuery->Fields->DataSet->FieldByName("TimerInsert")->AsDateTime = Date;
 try { LiteQuery->Post( ); }
 catch (...) { FormMain->WriteLog( "Ошибка записи TableServis" ); return 0x81; }
 memcpy( (void *)&Server, Data, sizeof( struct SERVERINFO ) );
 Server.TimerInsert = Date.Val;
// ModuleMySQL->WriteTableServis( );
 return 1;
 }

int TSqModule::WriteTableDevise( void *buf )
 {
 struct TERMINALINFO *NewData; // информация о приборе
 union SETDEV OldParametrs;
 TByteDynArray blob;
 char str[100];
 TDateTime Date;
 char er;

 NewData = (struct TERMINALINFO *)buf;
 // прибор был в работе (ID=.....) -> поиск ведём по ID (уникальное)
 if( NewData->ID[0] )
	{
	LiteQuery->SQL->Clear( );
	LiteQuery->SQL->Add( "select * from `TableDevise`" );
	sprintf( str, " where `Identifier` = '%s';", NewData->ID );
	LiteQuery->SQL->Add( str );
	try { LiteQuery->Open( ); }
	catch (...) { return -1; } // ошибка БД
	if( LiteQuery->Eof == false ) goto EndSearch; // прибор найден -> конец поиска
	}
 // прибор небыл в работе (нет данных в базе) -> стоит поискать по номеру
 // поиск по номеру ... прибор небыл в работе или новое устройство
 if( NewData->Code[0] )
	{
	LiteQuery->SQL->Clear( );
	LiteQuery->SQL->Add( "select * from `TableDevise`" );
	sprintf( str, " where `Code` = '%s';", NewData->Code );
	LiteQuery->SQL->Add( str );
	try { LiteQuery->Open( ); }
	catch (...) { return -1; } // ошибка БД
	}
 else return -1; // ошибка

EndSearch:
 NewData->SaveFlag = 1; blob.set_length(sizeof(union SETDEV));
 LiteQuery->Edit( );
 // прибор небыл в работе или новый (нет данных в базе)
 // записать параметры прибора
 if( LiteQuery->Eof == true )
	{
	memcpy( &blob[0], &NewData->Parametrs, sizeof(union SETDEV) );
	LiteQuery->Fields->FieldByName("Parametrs")->AsBytes = blob;
	}
 // сохраняем старые параметры
 blob =	LiteQuery->Fields->FieldByName("Parametrs")->AsBytes;
 memcpy( &OldParametrs, &blob[0], sizeof(union SETDEV) );
 // Запись новых значений
 LiteQuery->Fields->DataSet->FieldByName("Identifier")->AsString = &NewData->ID[0];
 // Номер прибора (выбор пользователя)
 LiteQuery->Fields->DataSet->FieldByName("Code")->AsString = &NewData->Code[0];
 // Версия ПО
 LiteQuery->Fields->DataSet->FieldByName("VerDevise")->AsInteger = NewData->VerDevise;

 Date = Date.CurrentDateTime( );
 LiteQuery->Fields->DataSet->FieldByName("TimerInsert")->AsDateTime = Date;

 LiteQuery->Fields->DataSet->FieldByName("HostInsert")->AsString =
													&NewData->HostInsert[0];
 // имя пользователя изменения данных
 LiteQuery->Fields->DataSet->FieldByName("NameInsert")->AsString =
													&NewData->NameInsert[0];
 LiteQuery->Fields->DataSet->FieldByName("SaveFlag")->AsInteger =
													NewData->SaveFlag;

 LiteQuery->Fields->DataSet->FieldByName("TypeDevise")->AsInteger
												= NewData->TypeDevise;
 LiteQuery->Fields->DataSet->FieldByName("Name")->AsString = &NewData->Name[0];
 LiteQuery->Fields->DataSet->FieldByName("Region")->AsString = &NewData->Region[0];
 LiteQuery->Fields->DataSet->FieldByName("DistrictCity")->AsString
												= &NewData->DistrictCity[0];
 LiteQuery->Fields->DataSet->FieldByName("Addres")->AsString = &NewData->Addres[0];
 LiteQuery->Fields->DataSet->FieldByName("Lat")->AsFloat =
										NewData->KordinateDevise.Lat;
 LiteQuery->Fields->DataSet->FieldByName("Long")->AsFloat =
										NewData->KordinateDevise.Lon;
 try { LiteQuery->Post(); }
 catch (...) { return -1; } // ошибка БД
 // найти табличный номер устройства
 NewData->TabNo = LiteQuery->Fields->DataSet->FieldByName("TabNo")->AsInteger;

 // запись в таблицу ожидания настройки
 LiteQuery->SQL->Clear( );
 LiteQuery->SQL->Add( "select * from `TableDeviseWait`" );
 if( NewData->ID[0] ) // устройство было в работе может поменялась карточка
	{
	sprintf( str, " where `Identifier` = '%s';", NewData->ID );
	LiteQuery->SQL->Add( str );
	}
 if( NewData->Code[0] ) // новое устройство -> ID нет
	{
	sprintf( str, " where `Code` = '%s';", NewData->Code );
	LiteQuery->SQL->Add( str );
	}
 try { LiteQuery->Open( ); }
 catch (...) { return -1; } // ошибка БД

 LiteQuery->Edit();
 LiteQuery->Fields->DataSet->FieldByName("Identifier")->AsString = NewData->ID;
 LiteQuery->Fields->DataSet->FieldByName("Code")->AsString = NewData->Code;
 LiteQuery->Fields->DataSet->FieldByName("TypeDevise")->AsInteger =
														NewData->TypeDevise;
 memcpy( &blob[0], &NewData->Parametrs, sizeof(union SETDEV) );
 LiteQuery->Fields->FieldByName("Parametrs")->AsBytes = blob;

 try { LiteQuery->Post( ); }
 catch (...) { return -1; }
 // Возвращаем старые параметры
 memcpy( &NewData->Parametrs, &OldParametrs, sizeof(union SETDEV) );
 er = WriteTableWaitRing( NewData->Code );// Запись в таблицу ожиданий
 if( er == 1 ) ContinueRing( ); // продолжение вызовов
// ModuleMySQL->WriteTableDevise( buf );
 return 0;
 }

void TSqModule::WriteTableData( void *buf )
 {
 struct INP_ARH *InpData; // Входящие данные
 TDateTime Date;
 AnsiString AS;
 char str[100];
 unsigned int FlagDevise, FlagBit;

 InpData = (struct INP_ARH *)buf;
 FlagDevise = InpData->Arh.FlagSetup.FlagDevise;
 FlagBit = InpData->Arh.FlagSetup.FlagBit;

 // Постоянные данные
 LiteQuery->SQL->Clear( );
 LiteQuery->SQL->Add( "insert into `TableDataGRP`" );
 LiteQuery->SQL->Add( " (`Code`, `Identifier`, `TimerOut`, `TimerInp`," );
 LiteQuery->SQL->Add( " `constCSQ`," );
 LiteQuery->SQL->Add( " `SensorSetup`, `BitSetup`," );
 LiteQuery->SQL->Add( " `SensorError`, `BitError`" );
 // --- датчики ---
 if( (FlagDevise & 2) ) // 1 - Давление Р1 (0x2)
			LiteQuery->SQL->Add( ",`P_1`,`FlP_1`" );
 if( (FlagDevise & 0x100) ) // 8 - Давление Р2 (0x100)
			LiteQuery->SQL->Add( ",`P_2`,`FlP_2`" );
 if( (FlagDevise & 4) ) // 2 - Давление Р3 (0x4)
			LiteQuery->SQL->Add( ",`P_3`, `FlP_3`" );

 if( (FlagDevise & 0x10) ) // 4 - ПСК 1 (0x10)
			LiteQuery->SQL->Add( ",`PSK_1`,`FlPSK_1`" );
 if( (FlagDevise & 0x200) ) // 9 - ПСК 2 (0x200)
			LiteQuery->SQL->Add( ",`PSK_2`,`FlPSK_2`" );

 if( (FlagDevise & 0x8) ) // 3 - Загазованность (0x8)
			LiteQuery->SQL->Add( ",`CnHm_1`,`FlCnHm_1`" );

 if( (FlagDevise & 0x20) ) // 5 - Температура 1 (0x20)
			LiteQuery->SQL->Add( ",`T_1`,`FlT_1`" );
 if( (FlagDevise & 0x40) ) // 6 - Температура 2 (0x40)
			LiteQuery->SQL->Add( ",`T_2`,`FlT_2`" );

 if( (FlagDevise & 0x80) ) // 7 - потанциал катодной защиты (0x80)
			LiteQuery->SQL->Add( ",`Ektz`,`FlEktz`" );
 if( (FlagDevise & 0x800) ) // ток катодной защиты (0x1000)
			LiteQuery->SQL->Add( ",`IKZ`,`FlIKZ`" );
 if( (FlagDevise & 0x1000) ) // напряжение катодной защиты (0x2000)
			LiteQuery->SQL->Add( ", `UKZ`,`FlUKZ`" );

 if( (FlagBit & 0x8000) ) // 0x8000 - обработчик ~220 V
			LiteQuery->SQL->Add( ",`U_220`" );
 else LiteQuery->SQL->Add( ",`Rez_AK`,`FlRez_AK`" );

 LiteQuery->SQL->Add( ",`AK`,`FlAK`" ); // 0 - Внутрений акумулятор (0x1)

 // --- битовые поля ---
 if( (FlagBit & 0x2) ) LiteQuery->SQL->Add( ",`PZK_1`" ); // 2 - PZK1
 if( (FlagBit & 0x200) ) LiteQuery->SQL->Add( ",`PZK_2`" ); // 1 - PZK2
 if( (FlagBit & 0x4) ) LiteQuery->SQL->Add( ",`DV_1`" ); // 4 - DV1
 if( (FlagBit & 0x8) ) LiteQuery->SQL->Add( ",`DV_2`" ); // 8 - DV2
 if( (FlagBit & 0x10) ) LiteQuery->SQL->Add( ",`DV_3`" ); // 10 - DV3
 if( (FlagBit & 0x20) ) LiteQuery->SQL->Add( ",`DV_4`" ); // 20 - DV4
 LiteQuery->SQL->Add( ") VALUES (" );
 // Постоянные данные
 // `Code`, `ID`, `TimerOut`, `TimerInp`, `constCSQ`,
 // `SensorSetup`, `BitSetup`, `SensorError`, `BitError`
 sprintf( str, "'%s', ", InpData->Pbl.Number ); LiteQuery->SQL->Add( str );
 sprintf( str, "'%s', ", InpData->Pbl.ID ); LiteQuery->SQL->Add( str );

 // время измерения данных
 sprintf( str, "'%0.2u.%0.2u.20%0.2u %0.2u:%0.2u:%u',",
	InpData->Pbl.Date[0], InpData->Pbl.Date[1], InpData->Pbl.Date[2],
		InpData->Pbl.Date[3], InpData->Pbl.Date[4], InpData->Pbl.Date[5] );
 LiteQuery->SQL->Add( str );
 // время получения данных
 Date = Date.CurrentDateTime( ); AS = Date.FormatString( "dd.mm.yyyy hh:nn:S" );
 sprintf( str, "'%s',", AS.c_str() ); LiteQuery->SQL->Add( str );

 // уровень связи
 sprintf( str, "%u, ", InpData->Arh.constCSQ ); LiteQuery->SQL->Add( str );
 // флаг настроек датчиков
 sprintf( str, "%u, ", InpData->Arh.FlagSetup.FlagDevise ); LiteQuery->SQL->Add( str );
 // флаг настроек битовых полей
 sprintf( str, "%u, ", InpData->Arh.FlagSetup.FlagBit ); LiteQuery->SQL->Add( str );
 // флаг ошибок датчиков
 sprintf( str, "%u, ", InpData->Arh.FlagError.FlagDevise ); LiteQuery->SQL->Add( str );
 // флаг ошибок битовых полей
 sprintf( str, "%u ", InpData->Arh.FlagError.FlagBit ); LiteQuery->SQL->Add( str );
 // --- датчики ---
 if( (FlagDevise & 2) ) // 1 - Давление Р1 (0x2)
	{
	sprintf( str, ", %.2f, %u", InpData->Arh.Sen[1].data,
									InpData->Arh.Sen[1].flag );
	LiteQuery->SQL->Add( str );
	}
 if( (FlagDevise & 0x100) ) // 8 - Давление Р2 (0x100)
	{
	sprintf( str, ", %.2f, %u", InpData->Arh.Sen[8].data,
									InpData->Arh.Sen[8].flag );
	LiteQuery->SQL->Add( str );
	}
 if( (FlagDevise & 4) ) // 2 - Давление Р3 (0x4)
	{
	sprintf( str, ", %.2f, %u", InpData->Arh.Sen[2].data,
									InpData->Arh.Sen[2].flag );
	LiteQuery->SQL->Add( str );
	}
 if( (FlagDevise & 0x10) ) // 4 - ПСК 1 (0x10)
	{
	sprintf( str, ", %3.0f, %u", InpData->Arh.Sen[4].data,
									InpData->Arh.Sen[4].flag );
	LiteQuery->SQL->Add( str );
	}
 if( (FlagDevise & 0x200) ) // 9 - ПСК 2 (0x200)
	{
	sprintf( str, ", %3.0f, %u", InpData->Arh.Sen[9].data,
									InpData->Arh.Sen[9].flag );
	LiteQuery->SQL->Add( str );
	}
 if( (FlagDevise & 0x8) ) // 3 - Загазованность (0x8)
	{
	sprintf( str, ", %.2f, %u", InpData->Arh.Sen[3].data,
									InpData->Arh.Sen[3].flag );
	LiteQuery->SQL->Add( str );
	}
 if( (FlagDevise & 0x20) ) // 5 - Температура 1 (0x20)
	{
	sprintf( str, ", %3.0f, %u", InpData->Arh.Sen[5].data,
									InpData->Arh.Sen[5].flag );
	LiteQuery->SQL->Add( str );
	}
 if( (FlagDevise & 0x40) ) // 6 - Температура 2 (0x40)
	{
	sprintf( str, ", %3.0f, %u", InpData->Arh.Sen[6].data,
									InpData->Arh.Sen[6].flag );
	LiteQuery->SQL->Add( str );
	}
 if( (FlagDevise&0x80) ) // 7 - потанциал катодной защиты (0x80)
	{
	LiteQuery->SQL->Add( ", " );
	sprintf( str, "%.2f", InpData->Arh.Sen[7].data );
	LiteQuery->SQL->Add( str );
	sprintf( str, ",%u", InpData->Arh.Sen[7].flag ); LiteQuery->SQL->Add( str );
	}
 if( (FlagDevise&0x800) ) // 11 - ток катодной защиты (0x1000)
	{
	LiteQuery->SQL->Add( ", " );
	sprintf( str, "%.2f", InpData->Arh.Sen[11].data );
	LiteQuery->SQL->Add( str );
	sprintf( str, ",%u", InpData->Arh.Sen[11].flag ); LiteQuery->SQL->Add( str );
	}
 if( (FlagDevise&0x1000) ) // 12 - напряжение катодной защиты (0x2000)
	{
	LiteQuery->SQL->Add( ", " );
	sprintf( str, "%.2f", InpData->Arh.Sen[12].data );
	LiteQuery->SQL->Add( str );
	sprintf( str, ",%u", InpData->Arh.Sen[12].flag ); LiteQuery->SQL->Add( str );
	}
 // Напряжение питания
 if( (FlagBit & 0x8000) ) // 0x8000 - обработчик ~220 V
	{
	LiteQuery->SQL->Add( ", " );
	if( !(InpData->Arh.FlagError.FlagBit & 0x8000) ) LiteQuery->SQL->Add( "'Норма'" );
	else LiteQuery->SQL->Add( "'Авария'" );
	}
 else // 10 - Внешний акумулятор (0x400)
	{
	LiteQuery->SQL->Add( ", " );
	sprintf( str, "%.2f", InpData->Arh.Sen[10].data ); LiteQuery->SQL->Add( str );
	sprintf( str, ",%u", InpData->Arh.Sen[10].flag ); LiteQuery->SQL->Add( str );
	}
 // 0 - Внутрений акумулятор (0x1)
 LiteQuery->SQL->Add( ", " );
 sprintf( str, "%.2f", InpData->Arh.Sen[0].data ); LiteQuery->SQL->Add( str );
 sprintf( str, ",%u", InpData->Arh.Sen[0].flag ); LiteQuery->SQL->Add( str );
 // --- битовые поля ---
 if( (FlagBit & 0x2) ) // ПЗК 1
	{
	LiteQuery->SQL->Add( ", " );
	if( (InpData->Arh.FlagError.FlagBit & 2) ) LiteQuery->SQL->Add( "'Закр.'" );
	else LiteQuery->SQL->Add( "'Откр.'" );
	}
 if( (FlagBit & 0x200) ) // ПЗК 2
	{
	LiteQuery->SQL->Add( ", " );
	if( (InpData->Arh.FlagError.FlagBit & 0x1) ) LiteQuery->SQL->Add( "'Закр.'" );
	else LiteQuery->SQL->Add( "'Откр.'" );
	}
 if( (FlagBit & 4) ) // Дверь № 1  (регуляторная)
	{
	LiteQuery->SQL->Add( ", " );
	if( !(InpData->Arh.FlagError.FlagBit & 4) ) LiteQuery->SQL->Add( "'Закр.'" );
	else LiteQuery->SQL->Add( "'Откр.'" );
	}
 if( (FlagBit & 8) ) // Дверь № 2  (топочная)
	{
	LiteQuery->SQL->Add( ", " );
	if( !(InpData->Arh.FlagError.FlagBit & 8) ) LiteQuery->SQL->Add( "'Закр.'" );
	else LiteQuery->SQL->Add( "'Откр.'" );
	}
 if( (FlagBit & 0x10) ) // Дверь № 3  (щитовая)
	{
	LiteQuery->SQL->Add( ", " );
	if( !(InpData->Arh.FlagError.FlagBit & 0x10) ) LiteQuery->SQL->Add( "'Закр.'" );
	else LiteQuery->SQL->Add( "'Откр.'" );
	}
 if( (FlagBit & 0x20) ) // Дверь № 4
	{
	LiteQuery->SQL->Add( ", " );
	if( !(InpData->Arh.FlagError.FlagBit & 0x20) ) LiteQuery->SQL->Add( "'Закр.'" );
	else LiteQuery->SQL->Add( "'Откр.'" );
	}
 LiteQuery->SQL->Add( ");" );

 try { LiteQuery->Execute( ); }
 catch (...) { return; }
 WriteTelegram( buf );
// ModuleMySQL->WriteTableData( buf );
 }
//-------------------------------------------------------------
//-------------------------------------------------------------

FILE *OutTell;

void OpenTel( void )
 {
 char str[200];

 sprintf( str, "%sTelegram.txt", FormMain->Patch ); OutTell = fopen( str, "a+t" );
 }

void OutTel( char *mess )
 { fputs( mess, OutTell ); fputc( '\n', OutTell ); fflush( OutTell ); }

void CloseTel( void ) { if( OutTell ) fclose( OutTell ); }

void WriteTelegram( void *buf )
 {
 struct INP_ARH *InpData; // Входящие данные
 TDateTime Date;
 AnsiString AS;
 char str[100];
 unsigned int FlagDevise, FlagBit;

 InpData = (struct INP_ARH *)buf;
 OpenTel( ); if( !OutTell ) return;
 // прибор был в работе (ID=.....) -> поиск ведём по ID (уникальное)
 if( InpData->Pbl.ID[0] )
	{
	SqModule->LiteQuery->SQL->Clear( );
	SqModule->LiteQuery->SQL->Add( "select * from `TableDevise`" );
	sprintf( str, " where `Identifier` = '%s';", InpData->Pbl.ID );
	SqModule->LiteQuery->SQL->Add( str );
	try { SqModule->LiteQuery->Open( ); }
	catch (...) { return; } // ошибка БД
	if( SqModule->LiteQuery->Eof == false ) goto EndSearch; // прибор найден -> конец поиска
	}
 // прибор небыл в работе (нет данных в базе) -> стоит поискать по номеру
 // поиск по номеру ... прибор небыл в работе или новое устройство
 if( InpData->Pbl.Number[0] )
	{
	SqModule->LiteQuery->SQL->Clear( );
	SqModule->LiteQuery->SQL->Add( "select * from `TableDevise`" );
	sprintf( str, " where `Code` = '%s';", InpData->Pbl.Number );
	SqModule->LiteQuery->SQL->Add( str );
	try { SqModule->LiteQuery->Open( ); }
	catch (...) { return; } // ошибка БД
	}
 else return; // ошибка
EndSearch:
 AS = SqModule->LiteQuery->Fields->DataSet->FieldByName("Name")->AsString;
 OutTel( AS.c_str() );
 AS = SqModule->LiteQuery->Fields->DataSet->FieldByName("Addres")->AsString;
 OutTel( AS.c_str() );
 // время
 sprintf( str, "%u-%u-%u  %u:%u:%u",
	InpData->Pbl.Date[0], InpData->Pbl.Date[1], InpData->Pbl.Date[2],
		InpData->Pbl.Date[3], InpData->Pbl.Date[4], InpData->Pbl.Date[5] );
 OutTel( str );

 // Pkz
 sprintf( str, "Потенціал, B  %0.2f", InpData->Arh.Sen[7].data );
 if( !InpData->Arh.Sen[7].flag ) strcat( str, " Норма" );
 else strcat( str, " Авария" );
 OutTel( str );
 // Ukz
 sprintf( str, "Напруга, B  %0.2f", InpData->Arh.Sen[10].data );
 if( !InpData->Arh.Sen[10].flag ) strcat( str, " Норма" );
 else strcat( str, "Авария" );
 OutTel( str );
 // Irz
 sprintf( str, "Струм, A  %0.2f", InpData->Arh.Sen[11].data );
 if( !InpData->Arh.Sen[11].flag ) strcat( str, " Норма" );
 else strcat( str, " Авария" );
 OutTel( str );
 // T1
 sprintf( str, "Т1, град  %0.1f", InpData->Arh.Sen[5].data );
 if( !InpData->Arh.Sen[5].flag ) strcat( str, " Норма" );
 else strcat( str, " Авария" );
 OutTel( str );
 // T2
 sprintf( str, "Т2, град  %0.1f", InpData->Arh.Sen[6].data );
 if( !InpData->Arh.Sen[6].flag ) strcat( str, " Норма" );
 else strcat( str, " Авария" );
 OutTel( str );
 // Ak
 sprintf( str, "Акум, B  %0.2f", InpData->Arh.Sen[0].data );
 if( !InpData->Arh.Sen[0].flag ) strcat( str, " Норма" );
 else strcat( str, " Авария" );
 OutTel( str );

 // Напряжение питания
 sprintf( str, "220В  " );
 if( !(InpData->Arh.FlagError.FlagBit & 0x8000) ) strcat( str, " Норма" );
 else strcat( str, " Авария" );
 OutTel( str );
 // охрана
 sprintf( str, "Охорона  " );
 if( !(InpData->Arh.FlagError.FlagBit & 0x4) ) strcat( str, " Норма" );
 else strcat( str, " Авария" );
 OutTel( str );
 // силовой блок (дверь 4 - 0x20)
 sprintf( str, "Силовий  " );
 if( !(InpData->Arh.FlagError.FlagBit & 0x20) ) strcat( str, " Норма" );
 else strcat( str, " Авария" );
 OutTel( str );
 CloseTel( );
 }
//-------------------------------------------------------------
//-------------------------------------------------------------

void TSqModule::CommandRing( char *Number )
 {
 char er;

 if( (ProcesFlag & 3) != 3 ) { MessRing( Number, -1 ); return; } // ошибка настроек FTDI
 er = WriteTableWaitRing( Number );// Запись в таблицу ожиданий
 MessRing( Number, er ); // ответка
 ContinueRing( ); // продолжение вызовов
 }
//----------------------------------------------------

// -1 модем отсутствует, 0 ошибка ДБ, 1 постановка в очередь
signed char TSqModule::WriteTableWaitRing( char *Number ) // Запись в таблицу ожиданий
 {
 char str[50];

 if( (ProcesFlag & 3) != 3 ) { return(-1); } // ошибка настроек FTDI
 // иденцификация устройства
 LiteQuery->SQL->Clear();
 LiteQuery->SQL->Add( "select * from `TableDevise`" );
 sprintf( str, " where `Code` = '%s';", Number );
 LiteQuery->SQL->Add( str );
 try { LiteQuery->Open( ); }
 catch (...) { return 0; } // ошибка БД
 if( LiteQuery->Eof == true ) { return 0; } // прибор не найден
 // поиск в таблице TableWaitRing
 LiteQuery->SQL->Clear( );
 LiteQuery->SQL->Add( "select * from `TableWaitRing`" );
 sprintf( str, " where `Code` = '%s';", Number );
 LiteQuery->SQL->Add( str );
 try { LiteQuery->Open( ); }
 catch (...) { return 0; } // ошибка БД
 if( LiteQuery->Eof == false ) return 1; // прибор есть
 // запись в таблицу TableWaitRing
 LiteQuery->Edit();
 LiteQuery->Fields->DataSet->FieldByName("Code")->AsString = Number;
 LiteQuery->Fields->DataSet->FieldByName("Count")->AsInteger = 1;
 try { LiteQuery->Post( ); }
 catch (...) { return 0; } // ошибка БД
 return 1;
 }
//----------------------------------------------------

// -1 модем отсутствует, 0 ошибка ДБ, 1 постановка в очередь
// 3 вызов не прошёл
void TSqModule::MessRing( char *Number, char Flag ) // ответка
 {
 AnsiString AS;
 char str[50];
 struct
	{
	struct COM Comand; // общая для всех команд
	struct DEV DevID; // информация о приборе
	} OutData;

 strcpy( OutData.Comand.Password, "ORION" );
 strcpy( OutData.Comand.Command, "Ring" );
 OutData.Comand.Name[0] = 0; OutData.Comand.Fun = Flag;
 strcpy( OutData.DevID.Code, Number );
 OutData.Comand.PackedLong = sizeof(struct DEV);
 // разослать клиентам
 // иденцификация устройства
 LiteQuery->SQL->Clear();
 LiteQuery->SQL->Add( "select * from `TableDevise`" );
 sprintf( str, " where `Code` = '%s';", Number );
 LiteQuery->SQL->Add( str );
 try { LiteQuery->Open( ); }
 catch (...) // ошибка БД
	{
	OutData.Comand.Fun = 0;
	ModuleTCP->SetAllMess( (void *)&OutData, sizeof(DEV) );
	return;
	}
 if( LiteQuery->Eof == true ) // прибор не найден
	{
	OutData.Comand.Fun = 0;
	ModuleTCP->SetAllMess( (void *)&OutData, sizeof(DEV) );
	return;
	}
 // разослать клиентам
 AS = LiteQuery->Fields->DataSet->FieldByName("Identifier")->AsString;
 strcpy( OutData.DevID.ID, AS.c_str() );
 ModuleTCP->SetAllMess( (void *)&OutData, sizeof(DEV) );
 return;
 }

// продолжение вызовов
void TSqModule::ContinueRing( void )
 {
 AnsiString AS;

 if( (ProcesFlag & (0x4 |0x8 |0x10 )) ) return; // не закончен предыдущий
 // поиск ожидающих вызова
 LiteQuery->SQL->Clear( );
 LiteQuery->SQL->Add( "select * from `TableWaitRing`" );
 try { LiteQuery->Open( ); }
 catch (...) { return; }// ошибка БД
 if( LiteQuery->Eof == true ) return; // нет обьектов
 LiteQuery->First( ); // первая запись
 AS = LiteQuery->Fields->DataSet->FieldByName("Code")->AsString;
// ModemDataModule->SetConnect( AS.c_str( ) ); // начать делать вызов
 }

// 3 вызов не прошёл
void TSqModule::ErrorRing( char *Number ) // ошибка вызова
 {
 char str[50];
 int Count;
 char Dev[20];

 // поиск в таблице TableWaitRing
 LiteQuery->SQL->Clear( );
 LiteQuery->SQL->Add( "select * from `TableWaitRing`" );
 sprintf( str, " where `Code` = '%s';", Number );
 LiteQuery->SQL->Add( str );
 try { LiteQuery->Open( ); }
 catch (...) { ContinueRing( ); MessRing( Number, 0 ); return; } // ошибка БД
 if( LiteQuery->Eof == true ) { return; } // нет обьектов
 Count = LiteQuery->Fields->DataSet->FieldByName("Count")->AsInteger;
 if( Count >= 3 ) // удалить
	{
	LiteQuery->SQL->Clear( );
	LiteQuery->SQL->Add( "delete from `TableWaitRing`" );
	sprintf( str, " where `Code` = '%s';", Number );
	LiteQuery->SQL->Add( str );
	try { LiteQuery->Execute( ); }
	catch (...) { }
	MessRing( Number, 3 ); ContinueRing( ); return;
	}

 Count++; // увеличить счёт
 LiteQuery->Edit( );
 LiteQuery->Fields->DataSet->FieldByName("Count")->AsInteger = Count;
 try { LiteQuery->Post( ); }
 catch (...)  { MessRing( Number, 0 ); } // ошибка БД
 ContinueRing( );
 }

void TSqModule::OkRing( char *Number ) // вызов прошёл
 {
 char str[50];
 char Dev[20];

 strcpy( Dev, Number );
 // убрать запрос
 LiteQuery->SQL->Clear( );
 LiteQuery->SQL->Add( "delete from `TableWaitRing`" );
 sprintf( str, " where `Code` = '%s';", Number );
 LiteQuery->SQL->Add( str );
 try { LiteQuery->Execute( ); }
 catch (...) { MessRing( Number, 0 ); ContinueRing( ); return; }
 MessRing( Number, 4 ); ContinueRing( ); // общий вызов
 }
//-------------------------------------------------------------

void TSqModule::WriteAllDevise( void *buf )
 {
 struct INP_ARH *InpData; // Входящие данные
 TByteDynArray blob;
 TDateTime Date;
 AnsiString AS;
 char str[100];

 InpData = (struct INP_ARH *)buf;
 // прибор в работе -> поиск ведём по ID (уникальное)
 LiteQuery->SQL->Clear( );
 LiteQuery->SQL->Add( "select * from `TableAllDevise`" );
 sprintf( str, " where `Identifier` = '%s';", &InpData->Pbl.ID[0] );
 LiteQuery->SQL->Add( str );
 try { LiteQuery->Open( ); }
 catch (...) { return; } // ошибка БД

 LiteQuery->Edit( );
 // записать параметры прибора
 LiteQuery->Fields->DataSet->FieldByName("Identifier")->AsString =
													&InpData->Pbl.ID[0];
 LiteQuery->Fields->DataSet->FieldByName("Code")->AsString =
													&InpData->Pbl.Number[0];
 Date = Date.CurrentDateTime( );
 AS = Date.FormatString( "dd-mm-yyyy hh:mm:S" );
// Date = AS;
 LiteQuery->Fields->DataSet->FieldByName("TimerInsert")->AsDateTime = Date;
 LiteQuery->Fields->DataSet->FieldByName("TypeDevise")->AsInteger =
												InpData->Param.TypeDevise;
 LiteQuery->Fields->DataSet->FieldByName("VerDevise")->AsInteger =
												InpData->Param.VerDevise;
 LiteQuery->Fields->DataSet->FieldByName("HostServer")->AsString =
												&InpData->Param.Host[0];
 LiteQuery->Fields->DataSet->FieldByName("PortServer")->AsInteger =
												InpData->Param.PortServer;
 LiteQuery->Fields->DataSet->FieldByName("Long")->AsFloat =
									InpData->Param.KordinateDevise.Lon;
 LiteQuery->Fields->DataSet->FieldByName("Lat")->AsFloat =
									InpData->Param.KordinateDevise.Lat;
 LiteQuery->Fields->DataSet->FieldByName("SensorSetup")->AsInteger =
											InpData->Param.Flag.FlagDevise;
 LiteQuery->Fields->DataSet->FieldByName("BitSetup")->AsInteger =
											InpData->Param.Flag.FlagBit;
 LiteQuery->Fields->DataSet->FieldByName("TimerArh")->AsInteger =
											InpData->Param.Timers.TimerArh;
 LiteQuery->Fields->DataSet->FieldByName("TimerCon")->AsInteger =
											InpData->Param.Timers.TimerCon;

 LiteQuery->Fields->DataSet->FieldByName("lPowerTimer")->AsInteger =
												InpData->Param.lPowerTimer;
 LiteQuery->Fields->DataSet->FieldByName("hPowerTimer")->AsInteger =
												InpData->Param.hPowerTimer;
 blob.set_length(sizeof(struct SEN)*16);
 memcpy( &blob[0], &InpData->Param.DevPor, sizeof(struct POR)*16 );
 LiteQuery->Fields->DataSet->FieldByName("Sensor")->AsBytes = blob;
 LiteQuery->Fields->DataSet->FieldByName("DopPor")->AsInteger =
												InpData->Param.DopPor;
 try { LiteQuery->Post( ); }
 catch (...) { return; } // ошибка БД
 }

//============================================
//============ РАБОТА С ТАБЛИЦАМИ ============
//============================================

int TSqModule::ReadTableServis( void *buf ) // 0x80 - ошибка
 {
 struct SERVERINFO *Data; // настройка сервера
 TDateTime Date;
 AnsiString AS;

 Data = (struct SERVERINFO *)buf;
 LiteQuery->SQL->Clear( );
 LiteQuery->SQL->Add( "select * from `TableServis`" );
 try { LiteQuery->Open( ); }
 catch (...) { FormMain->WriteLog( "Ошибка чтения TableServis" ); return 0x80; }
 if( LiteQuery->Eof == true ) return( 0x80 );

 AS = LiteQuery->Fields->DataSet->FieldByName("HostServer")->AsString;
 strcpy( Data->HostServer, AS.c_str() );
 Data->ExtPortServer = LiteQuery->Fields->DataSet->
				FieldByName("ExtPortServer")->AsInteger;
 Data->IntPortServer = LiteQuery->Fields->DataSet->
				FieldByName("IntPortServer")->AsInteger;
 AS = LiteQuery->Fields->DataSet->FieldByName("AdminPassword")->AsString;
 strcpy( Data->AdminPassword, AS.c_str() );
 Data->TimerSaveDB = LiteQuery->Fields->DataSet->
				FieldByName("TimerSaveDB")->AsInteger;
 AS = LiteQuery->Fields->DataSet->FieldByName("HostInsert")->AsString;
 strcpy( Data->HostInsert, AS.c_str() );
 AS = LiteQuery->Fields->DataSet->FieldByName("NameInsert")->AsString;
 strcpy( Data->HostInsert, AS.c_str() );

 Date = LiteQuery->Fields->DataSet->
				FieldByName("TimerInsert")->AsDateTime;
 Data->TimerInsert = Date.Val;
 return 0;
 }

int TSqModule::DeleteDevise( void *buf )
 {
 struct TERMINALINFO *Data; // информация о приборе
 char str[100];

 Data = (struct TERMINALINFO *)buf;

 // прибор был в работе (ID=.....) или поиск ведём по ID (уникальное)
 if( Data->ID[0] )
	{
	LiteQuery->SQL->Clear( );
	LiteQuery->SQL->Add( "select * from `TableDevise`" );
	sprintf( str, " where `Identifier` = '%s';", Data->ID );
	LiteQuery->SQL->Add( str );
	try { LiteQuery->Open( ); }
	catch (...) { return -1; } // ошибка БД
	if( LiteQuery->Eof == false ) // прибор найден -> конец поиска
		{
		// удаление данных TableDevise
		LiteQuery->SQL->Clear( );
		LiteQuery->SQL->Add( "delete from `TableDevise`" );
		sprintf( str, " where `Identifier` = '%s';", Data->ID );
		LiteQuery->SQL->Add( str );
		try { LiteQuery->Execute( ); }
		catch (...) { return -1; } // ошибка БД
		// удаление данных TableDeviseWait
		LiteQuery->SQL->Clear( );
		LiteQuery->SQL->Add( "delete from `TableDeviseWait`" );
		sprintf( str, " where `Identifier` = '%s';", Data->ID );
		LiteQuery->SQL->Add( str );
		try { LiteQuery->Execute( ); }
		catch (...) { return -1; } // ошибка БД
		return 0;
		}
	// прибор небыл в работе (нет данных в базе) -> стоит поискать по номеру
	}
 // поиск по номеру ... прибор небыл в работе или
 // новое устройство -> смотрим на номер
 if( Data->Code[0] )
	{
	LiteQuery->SQL->Clear( );
	LiteQuery->SQL->Add( "select * from `TableDevise`" );
	sprintf( str, " where `Code` = '%s';", Data->Code );
	LiteQuery->SQL->Add( str );
	try { LiteQuery->Open( ); }
	catch (...) { return -1; } // ошибка БД
	if( LiteQuery->Eof == false ) // прибор найден
		{
		// удаление данных TableDevise
		LiteQuery->SQL->Clear( );
		LiteQuery->SQL->Add( "delete from `TableDevise`" );
		sprintf( str, " where `Code` = '%s';", Data->Code );
		LiteQuery->SQL->Add( str );
		try { LiteQuery->Execute( ); }
		catch (...) { return -1; } // ошибка БД
		// удаление данных TableDeviseWait
		LiteQuery->SQL->Clear( );
		LiteQuery->SQL->Add( "delete from `TableDeviseWait`" );
		sprintf( str, " where `Code` = '%s';", Data->Code );
		LiteQuery->SQL->Add( str );
		try { LiteQuery->Execute( ); }
		catch (...) { return -1; } // ошибка БД
		return 0;
		}
	}
 return 0;
 }

void *TSqModule::GetNewParam( void *dev )
 {
 struct DEV *terminal;
 TByteDynArray blob;
 static struct NEWPAR Out;
 char str[100];

 terminal = (struct DEV *)dev;
 memset( (void *)&Out, 0, sizeof(struct NEWPAR) );
 memcpy( &Out.ID, terminal, sizeof(struct DEV) );
 // поиск ведём по ID (уникальное)
 LiteQuery->SQL->Clear( );
 LiteQuery->SQL->Add( "select * from `TableDeviseWait`" );
 sprintf( str, " where `Identifier` = '%s';", terminal->ID );
 LiteQuery->SQL->Add( str );
 try { LiteQuery->Open( ); }
 catch (...) { return &Out; } // ошибка БД
 if( LiteQuery->Eof == true ) // прибор не найден -> поиск по номеру
	{
	LiteQuery->SQL->Clear( );
	LiteQuery->SQL->Add( "select * from `TableDeviseWait`" );
	sprintf( str, " where `Code` = '%s';", terminal->Code );
	LiteQuery->SQL->Add( str );
	try { LiteQuery->Open( ); }
	catch (...) { return &Out; } // ошибка БД
	}

 if ( LiteQuery->Eof == true ) return &Out; // прибор не найден
 Out.FlagLoad = 1; // есть данные
 Out.TypeDevise = // тип устройства
	LiteQuery->Fields->DataSet->FieldByName("TypeDevise")->AsInteger;
 blob = LiteQuery->Fields->DataSet->FieldByName("Parametrs")->AsBytes;
 memcpy( &Out.Parametrs, &blob[0], sizeof( union SETDEV ) );
 return( &Out );
 }

void *TSqModule::SetCustomizationPac( void *Pbl )
 {
 TByteDynArray blob;
 static struct OUT_PARAM_TERMINAL OutPac;
 struct PUBLIC *InpPbl; // входящая шапка любой команды
 char str[100];

 InpPbl = (struct PUBLIC *)Pbl;
 blob.set_length(sizeof(union SETDEV));
 // --- установка шапки ---
 strcpy( (char *)&OutPac.Pbl.Password, "ORION" );
 OutPac.Pbl.Command = 4; // пакет настройки,
 strcpy( (char *)&OutPac.Pbl.ID, InpPbl->ID );
 strcpy( (char *)&OutPac.Pbl.Number, InpPbl->Number );
 DataModuleInternet->SetTime( (char *)&OutPac.Pbl.Date );
 OutPac.Pbl.NamberPacked = 0; OutPac.Pbl.PackedLong = sizeof( union SETDEV );

 // поиск ведём по ID (уникальное)
 LiteQuery->SQL->Clear();
 LiteQuery->SQL->Add( "select * from `TableDeviseWait`" );
 sprintf( str, " where `Identifier` = '%s';", InpPbl->ID );
 LiteQuery->SQL->Add( str );
 try { LiteQuery->Open( ); }
 catch (...) { return 0; } // ошибка БД
 if( LiteQuery->Eof == true ) // прибор не найден -> поиск по номеру
	{
	LiteQuery->SQL->Clear( );
	LiteQuery->SQL->Add( "select * from `TableDeviseWait`" );
	sprintf( str, " where `Code` = '%s';", InpPbl->Number );
	LiteQuery->SQL->Add( str );
	try { LiteQuery->Open( ); }
	catch (...) { return 0; } // ошибка БД
	}

 if ( LiteQuery->Eof == true ) return 0; // прибор не найден
 OutPac.Pbl.TypeDevise = // тип устройства
	LiteQuery->Fields->DataSet->FieldByName("TypeDevise")->AsInteger;
 blob = LiteQuery->Fields->DataSet->FieldByName("Parametrs")->AsBytes;
 memcpy( &OutPac.Data, &blob[0], sizeof( union SETDEV ) );
 return(&OutPac);
 }
//==================================================================

// ------- потверждение настройки -------
// снять флаг настройки
// переписать новые данные устройства (записать новый номер устройства)
// раздать всем клиентам новые данные
// удалить временные данные
void TSqModule::OkCustomizationPac( void *Pbl )
 {
 TByteDynArray blob; // struct PARAMETRS_TERMINAL;
 TDateTime Date;
 struct PUBLIC *Public;
 char str[100];
 AnsiString AS;
 struct
	{
	struct COM Command; // общая для всех команд
	struct TERMINALINFO Devise; // информация о приборе
	} OutData;

 Public = (struct PUBLIC *)Pbl;
 blob.set_length(sizeof(struct PARAMETRS_TERMINAL));

 // --- получение новых данных устройства (TableDeviseWait) ---
 // поиск ведём по ID (уникальное)
 LiteQuery->SQL->Clear();
 LiteQuery->SQL->Add( "select * from `TableDeviseWait`" );
 sprintf( str, " where `Identifier` = '%s';", Public->ID );
 LiteQuery->SQL->Add( str );
 try { LiteQuery->Open( ); }
 catch (...) { return; } // ошибка БД
 if( LiteQuery->Eof == true ) // прибор не найден -> поиск по номеру
	{
	LiteQuery->SQL->Clear( );
	LiteQuery->SQL->Add( "select * from `TableDeviseWait`" );
	sprintf( str, " where `Code` = '%s';", Public->Number );
	LiteQuery->SQL->Add( str );
	try { LiteQuery->Open( ); }
	catch (...) { return; } // ошибка БД
	}
 if ( LiteQuery->Eof == true ) return; // прибор не найден

 blob = LiteQuery->Fields->DataSet->FieldByName("Parametrs")->AsBytes;
 // удаление временных данных
 LiteQuery->SQL->Clear( );
 LiteQuery->SQL->Add( "delete from `TableDeviseWait`" );
 LiteQuery->SQL->Add( str ); // строка поиска
 try { LiteQuery->Execute(); }
 catch (...) { return; } // ошибка БД

 // -------- работа с таблицей TableDevise --------
 // поиск ведём по ID (уникальное)
 LiteQuery->SQL->Clear();
 LiteQuery->SQL->Add( "select * from `TableDevise`" );
 sprintf( str, " where `Identifier` = '%s';", Public->ID );
 LiteQuery->SQL->Add( str );
 try { LiteQuery->Open( ); }
 catch (...) { return; } // ошибка БД
 if( LiteQuery->Eof == true ) // прибор не найден -> поиск по номеру
	{
	LiteQuery->SQL->Clear();
	LiteQuery->SQL->Add( "select * from `TableDevise`" );
	sprintf( str, " where `Code` = '%s';", Public->Number );
	LiteQuery->SQL->Add( str );
	try { LiteQuery->Open( ); }
	catch (...) { return; } // ошибка БД
	}
 if ( LiteQuery->Eof == true ) return; // прибор не найден

 // --- прибор найден -> изменение данных устройства ---
 LiteQuery->Edit();
 LiteQuery->Fields->DataSet->FieldByName("Identifier")->AsString = Public->ID;
 LiteQuery->Fields->DataSet->FieldByName("Code")->AsString = Public->Number;
 LiteQuery->Fields->DataSet->FieldByName("VerDevise")->AsString =
														Public->VerDevise;
 Date = Date.CurrentDateTime( );
 LiteQuery->Fields->DataSet->FieldByName("TimerInsert")->AsDateTime = Date.Val;
 LiteQuery->Fields->DataSet->FieldByName("SaveFlag")->AsInteger = 0;
 LiteQuery->Fields->DataSet->FieldByName("Parametrs")->AsBytes = blob;
 try { LiteQuery->Post(); }
 catch (...) { return; } // ошибка БД

 // подготовить пакет и раздать всем клиентам новые данные
 // --- установка шапки ---
 memset( &OutData, 0, sizeof( struct TERMINALINFO ) );
 strcpy( (char *)&OutData.Command.Password, "ORION" );
 strcpy( (char *)&OutData.Command.Command, "SetDev" );
 OutData.Command.Name[0] = 0; OutData.Command.Fun = 2;
 OutData.Command.PackedLong = sizeof( struct TERMINALINFO ); // размер без шапки
 // --- подготовка данных ---
 // найти табличный номер устройства
 OutData.Devise.TabNo = LiteQuery->Fields->DataSet->FieldByName("TabNo")->AsInteger;
 strcpy( (char *)&OutData.Devise.ID, Public->ID );
 strcpy( (char *)&OutData.Devise.Code, Public->Number );
 OutData.Devise.VerDevise = Public->VerDevise;
 OutData.Devise.SaveFlag = 0; OutData.Devise.TimerInsert = Date.Val;
 AS = SqModule->LiteQuery->Fields->FieldByName("HostInsert")->AsString;
 strcpy( OutData.Devise.HostInsert, AS.c_str() );
 AS = SqModule->LiteQuery->Fields->FieldByName("NameInsert")->AsString;
 strcpy( OutData.Devise.NameInsert, AS.c_str() );
 OutData.Devise.TypeDevise =
	SqModule->LiteQuery->Fields->FieldByName("TypeDevise")->AsInteger;
 AS = SqModule->LiteQuery->Fields->FieldByName("Name")->AsString;
 strcpy( OutData.Devise.Name, AS.c_str() );
 AS = SqModule->LiteQuery->Fields->FieldByName("Region")->AsString;
 strcpy( OutData.Devise.Region, AS.c_str() );
 AS = SqModule->LiteQuery->Fields->FieldByName("DistrictCity")->AsString;
 strcpy( OutData.Devise.DistrictCity, AS.c_str() );
 AS = SqModule->LiteQuery->Fields->FieldByName("Addres")->AsString;
 strcpy( OutData.Devise.Addres, AS.c_str() );
 OutData.Devise.KordinateDevise.Lat =
	SqModule->LiteQuery->Fields->FieldByName("Lat")->AsFloat;
 OutData.Devise.KordinateDevise.Lon =
	SqModule->LiteQuery->Fields->FieldByName("Long")->AsFloat;
 memcpy( (void *)&OutData.Devise.Parametrs, (void *)&blob[0], sizeof(union SETDEV) );

 ModuleTCP->SetAllMess( (void *)&OutData, sizeof(struct TERMINALINFO) );
 }
//==================================================================

// -1 - нет данных, 0 - ошибка БД
// 0x1 - есть обьект (по ID), 0x2 - есть обьект (по номеру)
// 0x4 - необходима запись параметров
signed char TSqModule::FindDevise( void *Pbl )
 {
 struct PUBLIC *Public;
 char str[100];
 signed char error;

 error = 0;
 Public = (PUBLIC *)Pbl; LiteQuery->SQL->Clear();
 LiteQuery->SQL->Add( "select * from `TableDevise`" );
 sprintf( str, " where `Identifier` = '%s';", Public->ID );
 LiteQuery->SQL->Add( str );
 try { LiteQuery->Open( ); }
 catch (...) { return 0; } // ошибка БД
 if( LiteQuery->Eof == true ) // прибор не найден -> поиск по номеру
	{
	error = 0x2;
	LiteQuery->SQL->Clear( );
	LiteQuery->SQL->Add( "select * from `TableDevise`" );
	sprintf( str, " where `Code` = '%s'", Public->Number );
	LiteQuery->SQL->Add( str );
	try { LiteQuery->Open( ); }
	catch (...) { return 0; } // ошибка БД
	if( LiteQuery->Eof == true ) return -1; // нет устройства
	}
 else error = 0x1;
 if( LiteQuery->Fields->DataSet->FieldByName("SaveFlag")->AsInteger ) error |= 0x4;
 return error;
 }
//=====================================================

// Пришло время очистки данных ДБ
void __fastcall TSqModule::Timer(TObject *Sender) // 3600000 -> 1 час
									{ ControlData( ); }

void TSqModule::ControlData( void ) // Пришло время очистки данных ДБ
 {
 TDateTime ControlTime;
 AnsiString AS;
 char str[50];

return;
 ControlTime = ControlTime.CurrentDateTime(); ControlTime -= Server.TimerSaveDB;
// AS = Date.FormatString( "dd.mm.yyyy hh:nn:S" );
 LiteQuery->SQL->Clear( );
 LiteQuery->SQL->Add( "DELETE FROM TableDataGRP WHERE TimerInp < " );
 sprintf( str, "'%s',", AS.c_str() ); LiteQuery->SQL->Add( str );
 try { LiteQuery->Execute( ); }
 catch (...) { return; }
 LiteConnection->Commit();
 }
//---------------------------------------------------------------------------

