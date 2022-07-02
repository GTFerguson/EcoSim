# g++ testObjects.cpp ../objects/gameObject.cpp ../objects/food.cpp \
# ../objects/spawner.cpp ../objects/creature/creature.cpp \
# ../objects/creature/genome.cpp ../world/tile.cpp \
# ../objects/creature/navigator.cpp -o TestObjects 
# 
g++ creatureTest.cpp ../objects/gameObject.cpp ../objects/food.cpp \
../objects/spawner.cpp ../objects/creature/creature.cpp \
../objects/creature/genome.cpp ../world/tile.cpp \
../objects/creature/navigator.cpp -o creatureTest 

#g++ fileTest.cpp ../objects/gameObject.cpp ../objects/food.cpp \
#../objects/spawner.cpp ../objects/creature/creature.cpp \
#../objects/creature/genome.cpp ../world/tile.cpp \
#../world/world.cpp ../world/SimplexNoise.cpp ../fileHandling.cpp \
#../objects/creature/navigator.cpp -o FileTest

g++ testCalendar.cpp ../calendar.cpp -o CalendarTest
