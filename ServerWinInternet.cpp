//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <tchar.h>
//---------------------------------------------------------------------------
USEFORM("ModulTCP.cpp", ModuleTCP); /* TDataModule: File Type */
USEFORM("ModuleTimer.cpp", DataModuleTimer); /* TDataModule: File Type */
USEFORM("LiteModul.cpp", SqModule); /* TDataModule: File Type */
USEFORM("Internet.cpp", DataModuleInternet); /* TDataModule: File Type */
USEFORM("Main.cpp", FormMain);
//---------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
	try
	{
		Application->Initialize();
		Application->MainFormOnTaskBar = true;
		Application->CreateForm(__classid(TFormMain), &FormMain);
		Application->CreateForm(__classid(TModuleTCP), &ModuleTCP);
		Application->CreateForm(__classid(TSqModule), &SqModule);
		Application->CreateForm(__classid(TDataModuleInternet), &DataModuleInternet);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	catch (...)
	{
		try
		{
			throw Exception("");
		}
		catch (Exception &exception)
		{
			Application->ShowException(&exception);
		}
	}
	return 0;
}
//---------------------------------------------------------------------------
