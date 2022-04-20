:: C++ compiler settings
g++ -o PGE_graph_visualiser.exe PGE_graph_visualiser.cpp -static-libstdc++ -lpthread -lsetupapi -lwinmm -luser32 -lgdi32 -lgdiplus -static -lopengl32 -lShlwapi -ldwmapi -lstdc++fs -std=c++20

:: this runs the program !! make sure you're in the right directory !!
start PGE_graph_visualiser
