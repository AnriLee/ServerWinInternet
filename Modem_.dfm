object ModemDataModule: TModemDataModule
  OldCreateOrder = False
  Height = 65
  Width = 76
  object Timer: TTimer
    Enabled = False
    Interval = 50
    OnTimer = TimerHandle
    Left = 16
    Top = 8
  end
end
