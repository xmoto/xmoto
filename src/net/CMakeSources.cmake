target_sources_relative(xmoto PRIVATE
  ./ActionReader.cpp ./ActionReader.h
  ./BasicStructures.h
  ./NetActions.cpp ./NetActions.h
  ./NetClient.cpp ./NetClient.h
  ./NetServer.cpp ./NetServer.h
  ./ServerRules.cpp ./ServerRules.h
  ./VirtualNetLevelsList.cpp ./VirtualNetLevelsList.h
  ./extSDL_net.cpp ./extSDL_net.h

  ./helpers/Net.cpp ./helpers/Net.h

  ./thread/ServerThread.cpp ./thread/ServerThread.h
)
