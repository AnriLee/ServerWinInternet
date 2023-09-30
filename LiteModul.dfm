object SqModule: TSqModule
  OldCreateOrder = False
  Height = 65
  Width = 300
  object TimerDelete: TTimer
    Interval = 3600000
    OnTimer = Timer
    Left = 248
    Top = 8
  end
  object LiteConnection: TFDConnection
    Params.Strings = (
      'BusyTimeout=100'
      'CacheSize=100000'
      'SharedCache=False'
      'Database=Orion.db3'
      'LockingMode=Normal'
      'DriverID=SQLite')
    LoginPrompt = False
    Left = 24
    Top = 8
  end
  object SQLiteDriverLink: TFDPhysSQLiteDriverLink
    Left = 112
    Top = 8
  end
  object LiteQuery: TFDQuery
    Connection = LiteConnection
    SQL.Strings = (
      'select * from `TableDevise`')
    Left = 184
    Top = 8
  end
end
