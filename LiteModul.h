//---------------------------------------------------------------------------

#ifndef LiteModulH
#define LiteModulH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Data.DB.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>
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
//---------------------------------------------------------------------------

struct RET_FIND // данные поиска
	{
	unsigned char Flag;
	unsigned char TypeDevise;
	};
//---------------------------------------------------------------------------

class TSqModule : public TDataModule
 {
 __published:	// IDE-managed Components
	TTimer *TimerDelete;
	TFDConnection *LiteConnection;
	TFDPhysSQLiteDriverLink *SQLiteDriverLink;
	TFDQuery *LiteQuery;
	void __fastcall Timer(TObject *Sender);
 private:	// User declarations
	void ControlData( void );

	void OpenTableServis( void );
	void OpenTableDevise( void );
	void OpenTableAllDevise( void ); // Создание таблицы AllDevise
	void OpenTableWaitRing( void ); // ожидание звонка
//	void OpenTableInteraction( void ); // короткая настройка терминала
	void OpenTableData( void );
	void MessRing( char *Number, char Flag ); // ответка
	void ContinueRing( void ); // продолжение вызовов
//	int TimerControlRing;
 public:		// User declarations
	__fastcall TSqModule(TComponent* Owner);

	int WriteTableServis( void *buf );
	int WriteTableDevise( void *buf );
	void WriteTableData( void *buf );
	int ReadTableServis( void *buf );
	int DeleteDevise( void *bufe );
	void *GetNewParam( void *dev );
	void *SetCustomizationPac( void *Pbl );
	void OkCustomizationPac( void *Pbl );
	signed char FindDevise( void *Pbl );
	void WriteAllDevise( void *buf );

	void CommandRing( char *Number );
	signed char WriteTableWaitRing( char *Number ); // Запись в таблицу ожиданий
	void ErrorRing( char *Number ); // ошибка вызова
	void OkRing( char *Number ); // вызов прошёл
 };
//---------------------------------------------------------------------------

extern PACKAGE TSqModule *SqModule;
//---------------------------------------------------------------------------
#endif
