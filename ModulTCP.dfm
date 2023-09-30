object ModuleTCP: TModuleTCP
  OldCreateOrder = False
  Height = 64
  Width = 74
  object ServerTCP: TServerSocket
    Active = False
    Port = 49060
    ServerType = stNonBlocking
    ThreadCacheSize = 100
    OnClientConnect = NewConnect
    OnClientDisconnect = CloseConnect
    OnClientRead = SocketRead
    OnClientError = ErrServer
    Left = 16
    Top = 9
  end
end
