# Levels Approach

This note is intentionally lightweight. The goal is to help you decide **where** level logic should live in your current architecture and what SGDK-specific constraints are worth keeping in mind while you implement it yourself.

## What your current architecture already does well

- `in_game.c` is the gameplay coordinator. It owns the runtime state, per-frame update loop, status transitions, score, and level progression.
- `level.c` is currently a data generator. It fills a `Level` struct with starting parameters.
- `snake.c` and `food.c` are focused gameplay entities with their own init/update/render/cleanup responsibilities.

That split is a good base. I would keep it.

## Suggested rule of thumb

Use these module responsibilities:

- `level.c`: level data, obstacle layouts, level-related helpers.
- `in_game.c`: state machine, transition timing, restart/next-level decisions, collision consequences.
- `snake.c`: movement, growth, snake-internal rules.
- `food.c`: food spawn/render logic.

The important idea is that a level should describe the playfield, not own the whole gameplay loop.

## A practical way to model levels

Right now a level is procedural metadata:

- initial speed
- initial segments
- food count
- food step
- bonus

For obstacles, I would extend that model in a data-oriented way.

Possible direction:

- store an obstacle count in `Level`
- store either a pointer to a static obstacle array or copy a small fixed-size array into `Level`
- keep obstacle positions aligned to your 8x8 gameplay grid

For a small Mega Drive project, static data is usually the simplest option. For example, each level can have a predefined list of obstacle cells rather than generating them dynamically.

That keeps the implementation easy to reason about and avoids unnecessary memory management.

## SGDK-friendly representation

Your game already behaves on an 8x8 grid:

- snake segments are 8x8
- food is 8x8
- movement and collision are tile-like even though sprites move in pixels

Lean into that.

Instead of thinking of obstacles as arbitrary pixel rectangles, think of them as grid cells.

Two good options:

1. Store obstacles as `Block { x, y }` in pixel coordinates, but always as multiples of 8.
2. Store obstacles as tile coordinates such as `(tx, ty)` and convert to pixels only for rendering.

For learning purposes, tile coordinates are often easier because:

- level layout becomes easier to read
- collision checks are simpler conceptually
- it maps well to SGDK background tile maps later if you want to draw walls on `BG_A` or `BG_B`

## Recommended implementation order

Keep the first version narrow.

### Step 1: treat obstacles as pure collision data

Before worrying about nice visuals, make levels change gameplay.

Implement:

- obstacle data per level
- collision check between snake head and obstacles
- level restart when obstacle collision happens

At this stage, rendering obstacles can be very simple.

### Step 2: add level rendering

Once collision works, decide how you want to display obstacles:

- as sprites
- as background tiles
- as a mix of both

For static walls, background tiles are usually a better fit than sprites on Mega Drive hardware.

Why:

- obstacles do not move
- you avoid spending sprite budget on walls
- rendering cost is easier to manage

If you use BG tiles, the level module can expose layout data and `in_game.c` can call a small render helper when a level starts.

### Step 3: make food spawning level-aware

After obstacles exist, food spawning should avoid:

- obstacle cells
- snake cells
- possibly HUD area if you want to reserve part of the screen

This is where a small helper such as `level_isBlockedCell()` becomes useful.

### Step 4: only then add special cases

After the normal path works, add:

- poison food
- super food
- lives
- more complex obstacle patterns

Do not mix those into the first obstacle implementation.

## Where the new code probably belongs

### In `level.h` / `level.c`

Good candidates:

- obstacle layout definitions
- helper that initializes a level from level number
- helper that checks whether a cell is blocked by the level
- helper that renders or clears level visuals

### In `in_game.c`

Good candidates:

- calling level init on level start
- calling level render on level transition
- checking snake-head vs obstacle collision each frame
- deciding the consequence: lose a life, restart level, or game over

### In `food.c`

Possible later improvement:

- retry spawn until the chosen cell is valid

If that starts needing knowledge about snake + obstacles together, it may be cleaner to keep spawn selection in `in_game.c` instead of making `food.c` know too much.

## A clean mental model for level restart

Your current `nextLevel()` already resets snake and food and reinitializes state. That is a strong hint for the architecture.

I would mirror that pattern for restart.

Think in terms of two operations:

- `start level N`
- `restart current level`

That avoids duplicated setup logic and makes lives easier to add later.

## One design choice worth making early

Decide whether obstacles are:

- only gameplay data, with visuals derived from that data
- or primarily a rendered tilemap, with collision derived from the tilemap

For your current project, I would start with the first option:

- keep one authoritative obstacle list in code/data
- render from that list
- collide against that same list

It is simpler for learning and debugging.

## Pitfalls to watch for in your current code

### Grid alignment vs speed

Your snake speed can be `1`, `2`, or `4`, while obstacles and food live on an 8-pixel grid.

That is workable, but it means collision should be thought through carefully. A head moving at speed `4` can enter an obstacle cell across multiple frames while not always landing exactly on a new tile each frame.

A simple approach is:

- keep obstacle positions on the 8x8 grid
- continue using overlap checks in pixels for collision
- only allow direction changes on grid alignment, as you already do

### Random spawning

Right now food spawns anywhere inside screen bounds. Once obstacles exist, random spawn without validation will eventually place food inside a wall.

Plan for that early.

### HUD overlap

You draw text in the upper rows. Decide whether gameplay should still use those rows.

If not, define a playable area and keep level layouts and food spawn inside it.

## A minimal milestone plan

If you want to keep this project educational, this is a good progression:

1. Add fixed obstacle arrays for 2-3 levels.
2. Render them in the simplest possible way.
3. Detect snake collision against them.
4. Restart the current level on collision.
5. Prevent food from spawning on blocked cells.
6. Only after that, add lives and richer level visuals.

## My main recommendation

Do not turn `level.c` into a second gameplay state machine.

Let it remain the place that answers questions like:

- what does level 3 contain?
- where are its obstacles?
- how many food items are required?
- what bonus rules apply?

And let `in_game.c` keep answering:

- what happens this frame?
- did the player hit a wall?
- should the level restart or advance?
- should the game return to menu?

That boundary matches the code you already have, so implementing levels will feel like an extension of the current design instead of a rewrite.