g++ -g -std=c++17 src/main.cpp src/window.cpp src/fileHandling.cpp \
src/calendar.cpp \
src/world/tile.cpp src/world/world.cpp src/world/SimplexNoise.cpp \
src/objects/food.cpp src/objects/gameObject.cpp src/objects/spawner.cpp \
src/statistics/statistics.cpp \
src/objects/creature/creature.cpp src/objects/creature/genome.cpp \
src/objects/creature/navigator.cpp \
include/colorpairs.hpp \
-lncurses -o EcoSim &> err

if [ -s err ]
then
  vim err
else
  rm err
  echo "Compile Successful!"
fi
