//---------------------------------------------------------------------------


#pragma hdrstop

#include "ModuleTimer.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma classgroup "Vcl.Controls.TControl"
#pragma resource "*.dfm"
//---------------------------------------------------------------------------

__fastcall TDataModuleTimer::
	TDataModuleTimer(TComponent* Owner)	: TDataModule(Owner) { }
//---------------------------------------------------------------------------
void __fastcall TDataModuleTimer::Handle(TObject *Sender)
 {
 if( Count ) Count--;
 else Socket->Close();
 }
//---------------------------------------------------------------------------
