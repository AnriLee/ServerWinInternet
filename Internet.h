//---------------------------------------------------------------------------

#ifndef InternetH
#define InternetH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <System.Win.ScktComp.hpp>
//---------------------------------------------------------------------------
// 0 - AK, 1 - P1, 2 - P3, 3 - Gaz
// 4 - PSK1, 5 - T1, 6 - T2, 7 - PKZ
// 8 - P2, 9 - PSK2, 10 - вн. AK
// 11 - IKZ, 12 - UKZ, 13 - U ~220

class TDataModuleInternet : public TDataModule
 {
 __published:	// IDE-managed Components
	TServerSocket *ServerInternet;
	void __fastcall Connect(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall ReadData(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall ErrServer(TObject *Sender, TCustomWinSocket *Socket, TErrorEvent ErrorEvent,
		  int &ErrorCode);
	void __fastcall Close(TObject *Sender, TCustomWinSocket *Socket);
 private:	// User declarations
	unsigned char CounterTimer;
	TMemoryStream *InpData; // буфер приёма пакетов TCP
 public:		// User declarations
	__fastcall TDataModuleInternet(TComponent* Owner);
	void SetTime( char *time );
 };
//---------------------------------------------------------------------------

extern PACKAGE TDataModuleInternet *DataModuleInternet;
//---------------------------------------------------------------------------

#endif
