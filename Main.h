//---------------------------------------------------------------------------

#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Data.DB.hpp>
#include <FireDAC.Comp.Client.hpp>
#include <FireDAC.Comp.DataSet.hpp>
#include <FireDAC.DApt.hpp>
#include <FireDAC.DApt.Intf.hpp>
#include <FireDAC.DatS.hpp>
#include <FireDAC.Phys.hpp>
#include <FireDAC.Phys.Intf.hpp>
#include <FireDAC.Phys.SQLite.hpp>
#include <FireDAC.Phys.SQLiteDef.hpp>
#include <FireDAC.Stan.Async.hpp>
#include <FireDAC.Stan.Def.hpp>
#include <FireDAC.Stan.Error.hpp>
#include <FireDAC.Stan.ExprFuncs.hpp>
#include <FireDAC.Stan.Intf.hpp>
#include <FireDAC.Stan.Option.hpp>
#include <FireDAC.Stan.Param.hpp>
#include <FireDAC.Stan.Pool.hpp>
#include <FireDAC.UI.Intf.hpp>
#include <FireDAC.VCLUI.Wait.hpp>
#include <Vcl.DBGrids.hpp>
#include <Vcl.Grids.hpp>
#include <Vcl.Menus.hpp>
#include <Vcl.ExtCtrls.hpp>
//---------------------------------------------------------------------------

class TFormMain : public TForm
 {
__published:	// IDE-managed Components
	TLabel *Label1;
	TFDTable *FDTable1;
	TPopupMenu *PopupMenu1;
	TMenuItem *hgfmmm1;
	TMenuItem *kjkjhkhk1;
	TTimer *Timer;
	void __fastcall Start(TObject *Sender);
	void __fastcall Stop(TObject *Sender, TCloseAction &Action);
	void __fastcall HandleTimer(TObject *Sender);
private:	// User declarations
	FILE *OutMess;
	void OpenLog( void );
	void CloseLog( void );
public:		// User declarations
	__fastcall TFormMain(TComponent* Owner);
	void WriteLog( char *str );

	char Patch[500];
 };
//---------------------------------------------------------------------------

extern char *UtfAsci( char *str, wchar_t *sourse );
extern unsigned int StartControlTime( void );
extern unsigned int GetControlTime( unsigned int OldTime );
extern struct SERVERINFO Server; // настройка сервера
extern short ProcesFlag;
	// 0x1 - норма настроек FTDI
	// 0x2 - норма настроек модема
	// 0x4 - команда "соединить"
	// 0x8 - соединение
	// 0x10 - ожидание "отбой"
	// 0x20 - получен NO CARRIER | NO DIALTONE | BUSY | ANSWER
	// 0x80 - ожидание больших данных (для TCP)
	//------- внешняя БД -------
	// 0x100 - база подключена
	// 0x800 - необходима синхроницация данных
//---------------------------------------------------------------------------

extern PACKAGE TFormMain *FormMain;
//---------------------------------------------------------------------------
#endif
