# Burnout Paradise Road Rule Limit Remover
Removes minimum time limit of 2 seconds from Time Road Rules and maximum score limit of $1000000000 from Showtime Road Rules by modifying the game's executable.

This would normally be done via dll, but since some cross-platform capability is nice and I don't feel like making sprx mods, this is how it's gonna be.

## Building
Requires CMake and Qt6. Qt should be in PATH.

```
mkdir build
cd build
cmake ..
cmake --build .
```