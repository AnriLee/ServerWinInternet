#ifndef ModulTCPH
#define ModulTCPH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <System.Win.ScktComp.hpp>
#include <Vcl.ExtCtrls.hpp>
//---------------------------------------------------------------------------

struct SOSCEDSTREAM
	{
	unsigned char ProcesFlag;
	int CountPac; // счётчик приёма больших данных
	void *InStream; // Данные приёма пакетов TCP
	void *OutStream; // Данные для отправки
	};

struct INITARH
	{
	char ID[20]; // ID прибора
	char Code[20]; // (номер) прибора
	double StartTime;
	double EngTime;
//	unsigned short ErrorDev;
//	unsigned short ErrorBit;
	};

struct DATAARH // архивные данные от сервера
	{
	double TimerOut; // время измерения данных
	double TimerInp; // время получения данных
	unsigned char constCSQ; // уровень связи
	struct FLAG FlagSetup;
	struct FLAG FlagError;
	struct SEN Sen[16];
	};

struct OUTARH // принимаемые данные
	{
	struct COM Pbl; // общая для всех команд
	struct INITARH Param;
	// struct DATAARH ......
	};

class TModuleTCP : public TDataModule
 {
__published:	// IDE-managed Components
	TServerSocket *ServerTCP;
	void __fastcall SocketRead(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall ErrServer(TObject *Sender, TCustomWinSocket *Socket,
									TErrorEvent ErrorEvent, int &ErrorCode);
	void __fastcall NewConnect(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall CloseConnect(TObject *Sender, TCustomWinSocket *Socket);
private:	// User declarations
	void StartControlTime( unsigned int *ControlTime );
	unsigned int GetControlTime( unsigned int *ControlTime );
	// команды ответов на запросы
	void SetParamServer( char *User, TCustomWinSocket *Socket );
//	void WriteParamServer( char * IP, char * User );
	void WriteParamServer( char * IP, char * User, TCustomWinSocket *Socket );
//	void ReWriteParamServer( char flag );
	void GetNewParam( TCustomWinSocket *Socket );
	void SetAllDevise( TCustomWinSocket *Socket );
	void SetOldData( TCustomWinSocket *Socket );
	unsigned short TimerDB;
	int ProcesFlag; // 0x80 - ожидание больших даных
public:		// User declarations
	__fastcall TModuleTCP(TComponent* Owner);

	void SetArh( char *Name, TCustomWinSocket *Socket );
	void SetArhData( char *Name, TCustomWinSocket *Socket );

	void OutTableData( void * );
	void SetAllMess( void *data, int size );

	TMemoryStream *TCPStream; // буфер приёма пакетов TCP
 };
//---------------------------------------------------------------------------

extern PACKAGE TModuleTCP *ModuleTCP;
//---------------------------------------------------------------------------
#endif
