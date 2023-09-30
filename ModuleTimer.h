//---------------------------------------------------------------------------

#ifndef ModuleTimerH
#define ModuleTimerH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <System.Win.ScktComp.hpp>

//---------------------------------------------------------------------------
class TDataModuleTimer : public TDataModule
 {
 __published:	// IDE-managed Components
	TTimer *Timer;
	void __fastcall Handle(TObject *Sender);
 private:	// User declarations
 public:		// User declarations
	__fastcall TDataModuleTimer(TComponent* Owner);
	TCustomWinSocket *Socket;
	int Count;
 };
//---------------------------------------------------------------------------

//extern PACKAGE TDataModuleTimer *DataModuleTimer;
//---------------------------------------------------------------------------
#endif
