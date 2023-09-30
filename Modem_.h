//---------------------------------------------------------------------------

#ifndef ModemH
#define ModemH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.ExtCtrls.hpp>
#include "FTD2XX.h"

struct FTDI
	{
	FT_HANDLE ftHandle;
	signed char FlagMess;
		// 0x1 ожидание потверждения команды
		// 0x2 пауза
		// 0x4 получена команда
		// 0x8 ожидание CSQ
		// 0x80 ошибка ожидание потверждения
	unsigned char StartFlag; // счётчик команд инициализации модема
	unsigned char ErrorFTDI; // счётчик ошибок потверждения
	int Tiker; // ожидание потверждения

	char FlagInit; // 1 - идёт инициализация модема
	int TimerInit; // задержка инициализации

	int TimerConnect; // ожидание отбоя от терминала

	// контроль состояния связи
	unsigned int TimerCSQ;
	unsigned char ConstCSQ;

	unsigned short InpCount;
	char BlokInp[100];
//	char Buf[500];
	};
//---------------------------------------------------------------------------

class TModemDataModule : public TDataModule
 {
 __published:	// IDE-managed Components
	TTimer *Timer;
	void __fastcall TimerHandle(TObject *Sender);
 private:	// User declarations
	void SetPortFTDI( void );
	void Work( void );
	void InitModem( void );
	void GetMess( void );
	char *Search( char *sourse, char *strSearch );
	void ModemCommand( void );
	char OutMess( char *str );
 public:		// User declarations
	__fastcall TModemDataModule(TComponent* Owner);
	void SetConnect( char *Number );

	char OutNumber[20];
	struct FTDI Port;
 };
//---------------------------------------------------------------------------

extern PACKAGE TModemDataModule *ModemDataModule;
//---------------------------------------------------------------------------
#endif


