## Features to implement
- Add Levels -> PARTIALLY IMPLEMENTED
- Add Lives
- Food types
- Add sound effects
- Add music

### Add Levels
Every level should have have this properties:
- Initial speed -> IMPLEMENTED
- Initial segments -> IMPLEMENTED
- Food count -> IMPLEMENTED
- Obstacles

The level starts with the initial speed and initial segments.
The goal of each level is to eat all the Normal food. When all the food is eaten, the level is completed and the next level starts.
Obstacles are static and if the snake collides with one, one life is lost and the level restarts.

### Food types
- Normal food: +1 segment, +1 speed. The sprite resembles a regular apple.
- Poison food: +0 segment, +2 speed, appears randomly for a short time. The sprite resembles a purple apple.
- Super food: +2 segments, +0 speed, appears randomly for a short time. The sprite resembles a golden apple.
