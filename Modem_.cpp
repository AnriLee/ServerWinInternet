#pragma hdrstop

#include <string.h>
#include <stdio.h>

#include "Main.h"
#include "Modem.h"
#include "LiteModul.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma link "Ftdi.lib"
#pragma classgroup "Vcl.Controls.TControl"
#pragma resource "*.dfm"
//---------------------------------------------------------------------------

#define WaitCom( Sec ) \
	{ Port.FlagMess |= 1; Port.InpCount = 0; Port.Tiker = Sec * 20; }
#define Wait( Sec ) { Port.FlagMess |= 2; Port.Tiker = Sec * 20; }

TModemDataModule *ModemDataModule;

//---------------------------------------------------------------------------
__fastcall TModemDataModule::
	TModemDataModule(TComponent* Owner) : TDataModule(Owner)
					{ Port.TimerInit = 20; Timer->Enabled = true; }
//---------------------------------------------------------------------------
// Вызывается каждые 50 mc
void __fastcall TModemDataModule::TimerHandle(TObject *Sender)
 {
 FT_STATUS ftStatus;
 unsigned long a;

 if( !(ProcesFlag & 1) ) // инициализация модема
	{
	if( Port.TimerInit ) Port.TimerInit--; // задержка инициализации
	else SetPortFTDI( );
	return;
	}

 ftStatus = FT_GetModemStatus( Port.ftHandle, &a );
 if( ftStatus != FT_OK ) { SetPortFTDI( ); return; }

 if( Port.TimerCSQ ) Port.TimerCSQ--;
 if( Port.Tiker ) Port.Tiker--;
 GetMess( ); // получение команды  CREG
 Work( );
 }
//---------------------------------------------------------------------------

void TModemDataModule::SetPortFTDI( void )
 {
 FT_STATUS ftStatus;

 Timer->Enabled = false;
 ProcesFlag &= ~3; // 1 - норма настроек FTDI, 2 - норма настроек модема
 if( Port.ftHandle ) FT_Close( Port.ftHandle ); // USB
 memset( &Port, 0, sizeof( struct FTDI ) );
 Port.TimerInit = 20 * 30; // таймер перезапуска 30 сек.
 ftStatus = FT_OpenEx( (char *)"ORION",
				FT_OPEN_BY_DESCRIPTION,	&Port.ftHandle );
 if( ftStatus != FT_OK ) { Timer->Enabled = true; return; }

 FT_SetDtr( Port.ftHandle );

 FT_SetBaudRate( Port.ftHandle, 57600 );
// FT_SetBaudRate( Port.ftHandle, 115200 );
 FT_SetDataCharacteristics ( Port.ftHandle, 8, 2, 0 );
 FT_SetTimeouts( Port.ftHandle, 5, 5 ); // чтение 5 мСек., запись 5 мСек.
 FT_Purge ( Port.ftHandle, FT_PURGE_RX | FT_PURGE_TX ); // очистка буферов
 Port.TimerCSQ = 20 * 60;
 ProcesFlag |= 1; Port.FlagInit = 1; // норма FTDI, инициализация модема
 for( int c = 0; c < 50000; c++ ); // задержка
 FT_ClrDtr( Port.ftHandle ); Timer->Enabled = true;
 }
//---------------------------------------------------------------------------

void TModemDataModule::Work( void )
 {
 // норма настроек FTDI, норма настроек модема
 if( !(ProcesFlag & 1) ) return;// норма настроек FTDI

 if( (Port.FlagMess & 1) ) // ожидание потверждения
	{
	if( Port.Tiker ) return;
	Port.FlagMess &= ~1; Port.FlagMess |= 0x80; Port.ErrorFTDI++;
	}
 else if( (Port.FlagMess & 2) ) // пауза
	{
	if( Port.Tiker ) return;
	Port.FlagMess &= ~2;
	}
 else if( (Port.FlagMess & 8) ) // ожидание CSQ
	{
	if( Port.Tiker ) return;
	Port.FlagMess &= ~(1 | 8); Port.ErrorFTDI++;
	}

 if( Port.ErrorFTDI >= 3 ) SetPortFTDI( );
 if( !(ProcesFlag & 2) ) { InitModem( ); return; } // норма настроек модема

 // ============ обработка инициализации связи ============
 if( !Port.TimerCSQ )
	{
	// "соединить", соединение, ожидание "отбой"
	if( !(ProcesFlag & (0x4 | 0x8 | 0x10)) )
		{
		Port.TimerCSQ = 20 * 60; Port.FlagMess |= 0x8;
		OutMess( "AT+CSQ\r\n" ); return;
		}
	}
 // ============ основная работа модема ============
 if( (ProcesFlag & 0x4) ) // 0x4 - команда "соединить" ожидание "OK"
	{
	// 0x20 - получен NO CARRIER | NO DIALTONE | BUSY | ANSWER
	ProcesFlag &= ~(0x4 |0x8 |0x10 |0x20);
	if( (Port.FlagMess & 0x80) ) SqModule->ErrorRing( OutNumber );
	else { WaitCom( 20 ); Port.FlagMess |= 1; ProcesFlag |= 0x8; }
	}
 else if( (ProcesFlag & 0x8) ) // 0x8 - ожидание "отбой"
	{
	if( (Port.FlagMess & 0x80) ) // если вышло время ожидания
		{
		ProcesFlag &= ~(0x4 |0x8 |0x10 |0x20); ProcesFlag |= 0x10;
		OutMess( (char *)"ATH\r\n" ); // ожидание отбоя
		}
	else // вызов прошёл
		{ ProcesFlag &= ~(0x4 |0x8 |0x10 |0x20); SqModule->OkRing( OutNumber ); }
	}
 // 0x20 - получен NO CARRIER | NO DIALTONE | ANSWER
 else if( (ProcesFlag & (0x10 |0x20)) ) // 0x10 - ожидание "отбой"
	{ ProcesFlag &= ~(0x4 |0x8 |0x10 |0x20 ); SqModule->ErrorRing( OutNumber ); }
 }
//---------------------------------------------------------------------------

void TModemDataModule::InitModem( void )
 {
 if( !(ProcesFlag & 0x1) ) return; // норма настроек FTDI
 if( !Port.FlagInit ) return; // инициализация модема

 if( !Port.StartFlag ) // начало
	{ Port.StartFlag++; Wait( 40 ); return;	}

 if( Port.FlagMess == 1 ) return; // ожидание потверждения
 else if( Port.FlagMess == 2 ) return; // пауза
 else if( Port.FlagMess == 4 ) // потверждение, получена команда
   { Port.ErrorFTDI = 0; Port.StartFlag++; }

 if( Port.StartFlag == 1 ) OutMess( "ATE0\r\n" ); // старт настроек
 else if( Port.StartFlag == 2 ) OutMess( "AT+IPR=57600\r\n" ); // СКОРОСТЬ
 else if( Port.StartFlag == 3 ) OutMess( "AT&W\r\n" ); // Сохранить информацию
 // модем в исходное
 else if( Port.StartFlag == 4 ) OutMess( "ATS7=50\r\n" ); // ВРЕМЯ ДОЗВОНА
 else if( Port.StartFlag == 5 ) OutMess( "AT+GSMBUSY=1\r\n" ); // запретить входящии звонки
 else if( Port.StartFlag == 6 ) OutMess( "AT+CREG=1\r\n" ); //  Разрешить индикацию сети
// ------------------
// else if( ComCount == 9 ) // ожидание "SMS Ready"
//	{ if( (ModemStatus.ModemFlag & BitReadySMS) ) Wait( 20 ); }
// else if( ComCount == 10 ) { ClrBitSMS; SetWait( 10 ); } // задержка после "SMS Ready"
// else if( ModemStatus.StartFlag == 10 ) ModemCommand( "AT+GMR\r\n" ); // версия
// ------------------
 else if( Port.StartFlag == 7 ) // ОКОНЧАНИЕ НАСТРОЕК
	{
	ProcesFlag |= 3; // норма настроек модема
	Port.ErrorFTDI = 0; Port.StartFlag = 0; Port.FlagInit =	0; // инициализация модема окончена
	}
 }
//-----------------------------------------------------------------------

void TModemDataModule::GetMess( void )
 {
 DWORD BytesReceived;
 FT_STATUS ftStatus;
 unsigned char c, Pauza;

 if( !(ProcesFlag & 0x1) ) return; // норма настроек FTDI

 for( Pauza = 200, Port.InpCount = 0; ; )
	{
	ftStatus = FT_Read( Port.ftHandle,
		&Port.BlokInp[Port.InpCount], 1, &BytesReceived );
	if( ftStatus != FT_OK || !BytesReceived )
		{
		if( !Port.InpCount ) return;
		Pauza--; if( Pauza ) continue; // timout
//		strcat( Port.Buf, Port.BlokInp );
		Port.BlokInp[++Port.InpCount] = 0; ModemCommand( );
		return;
		}
	if( Port.BlokInp[Port.InpCount] == '\n' ) // конец команды
		{
//		strcat( Port.Buf, Port.BlokInp );
		if( Port.InpCount < 3 ) { Port.InpCount = 0; continue; }
		// обработка команды -> убрать лишнее
		for( c = 0; c < Port.InpCount; c++ )
			{ if( Port.BlokInp[c] == '\r' || Port.BlokInp[c] == '\n' ) break; }
		Port.BlokInp[c] = 0; Port.InpCount = c; ModemCommand( );
		return;
		}
	Port.InpCount++; Pauza = 200;
	}
 }
//==============================================================================

char * TModemDataModule::Search( char *sourse, char *strSearch )
 {
 char *s, *d;

 s = sourse; d = strSearch;
 for( ; *s; s++ )
	{
	if( *s != *d ) d = strSearch;
	else { d++; if( !*d ) return( s ); }
	}
 return 0;
 }

void TModemDataModule::ModemCommand( void )
 {
 unsigned char command, c, data, *str;
 char Mess[10][15] =
	{
	{ "OK" }, // 0
	{ "ERROR" }, // 1
	{ "+CSQ" }, // 2
	{ "CREG" }, // 3 Проверка сети CSD

	{ "CONNECT" }, // 4 СВЯЗЬ
	{ "BUSY" }, // 5 - ЗАНЯТО
	{ "NO CARRIER" }, // 6 - КОНЕЦ СВЯЗИ
	{ "NO DIALTONE" }, // 7
	{ "ANSWER" }, // 8 - ошибка
	{ "***" } // 9
	};

 for( command = 0; command < 10; command++ )
	{ if( Search( Port.BlokInp, &Mess[command][0]) ) break; }

 switch( command )
	{
	case 0: // получена команда "OK"
		{ Port.FlagMess &= ~1; Port.FlagMess |= 0x4; break; }
	case 1: // "ERROR"
		{ Port.FlagMess &= ~1; Port.FlagMess |= 0x80; break; }
	case 2: // "+CSQ"
		{
		Port.FlagMess &= ~(1 | 8);
		str = strchr( (char * )&Port.BlokInp, ' ' ); str++;
		Port.ConstCSQ = atoi( str );
		if( Port.ConstCSQ < 5 ) SetPortFTDI( );
		break;
		}
	case 3: // "CREG" Проверка сети CSD -> автоматическая команда
		{
		// 1 - норма настроек FTDI и 2 - норма настроек модема
		if( (ProcesFlag & 3) != 3 ) return;
		data = atoi( strpbrk( &Port.BlokInp[0], "012345" ) );
		if( data != 1 ) ProcesFlag &= ~3; // модем не в сети
		break;
		}
	case 4: // "CONNECT"  СВЯЗЬ -> для цифрового вызова
		{ Port.FlagMess &= ~1; ProcesFlag &= ~0x4; ProcesFlag |= 0x8; break; }
	case 5: // "BUSY" ЗАНЯТО или отбой не поднимая трубки (прошёл вызов)
		{ Port.FlagMess = 0; break; }
	// ошибка соединения
	case 6: // "NO CARRIER" КОНЕЦ СВЯЗИ
	case 7: // "NO DIALTONE"
	case 8: // "ANSWER"
		{
		Port.FlagMess = 0;
		ProcesFlag &= ~(0x4 |0x8 |0x10 | 0x20 ); ProcesFlag |= 0x20;
		break;
		}
	default: { Port.FlagMess = 0x80; break; } // нет такой команды -> ошибка
	}

 }
//---------------------------------------------------------------------------

char TModemDataModule::OutMess( char *str )
 {
 DWORD BytesReceived;
 FT_STATUS ftStatus;

 Port.FlagMess &= ~(0x4 | 0x80 ); // потверждение, ошибка ожидания
 FT_Purge ( Port.ftHandle, FT_PURGE_RX | FT_PURGE_TX ); // очистка буферов
 ftStatus = FT_Write( Port.ftHandle, str, strlen(str), &BytesReceived );
 if( ftStatus != FT_OK ) return( -1 );
 WaitCom( 8 ); // ожидание потверждения команды
 return( 1 );
 }
//---------------------------------------------------------------------------

void TModemDataModule::SetConnect( char *Number )
 {
 char buf[100];

 if( (ProcesFlag & 3) != 3 ) return; // ошибка настроек FTDI
 if( (ProcesFlag & 4) ) return; // 0x4 - команда "соединить"
 strcpy( OutNumber, Number );
// sprintf( buf, "ATD%s\r\n", Number ); // цифровой звонок
 sprintf( buf, "ATD%s;\r\n", Number ); // аналоговый звонок
 if( OutMess(buf) < 0 ) { SqModule->ErrorRing( OutNumber ); return; }
 ProcesFlag &= ~(0x4 |0x8 |0x10 | 0x20 ); ProcesFlag |= 4; // "соединить"
 return;
 }




