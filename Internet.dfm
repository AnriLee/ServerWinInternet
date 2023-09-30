object DataModuleInternet: TDataModuleInternet
  OldCreateOrder = False
  Height = 107
  Width = 104
  object ServerInternet: TServerSocket
    Active = False
    Port = 49050
    ServerType = stNonBlocking
    ThreadCacheSize = 100
    OnClientConnect = Connect
    OnClientDisconnect = Close
    OnClientRead = ReadData
    OnClientError = ErrServer
    Left = 32
    Top = 24
  end
end
