eosiocpp -o ./Ship/Ship.wast ./Ship/Ship.cpp
eosiocpp -g ./Ship/Ship.abi ./Ship/Ship.cpp

eosiocpp -o ./Resources/Resources.wast ./Resources/Resources.cpp
eosiocpp -g ./Resources/Resources.abi ./Resources/Resources.cpp

echo "Build complete"