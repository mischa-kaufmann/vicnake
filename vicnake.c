#define _XOPEN_SOURCE 500 // For usleep()

#include <ncurses.h> // For terminal UI, input, and colors
#include <stdlib.h>  // For rand(), srand(), exit()
#include <unistd.h>  // For usleep()
#include <time.h>    // For srand(time(NULL))
#include <stdbool.h> // For bool type (true, false)
#include <string.h>  // For strlen(), sprintf()

// --- Game Constants ---
#define GAME_AREA_WIDTH 60  // Width of the playable game area in characters
#define GAME_AREA_HEIGHT 30 // Height of the playable game area in characters
#define MAX_SNAKE_LENGTH (GAME_AREA_WIDTH * GAME_AREA_HEIGHT)
#define INITIAL_SNAKE_LENGTH 3
#define GAME_SPEED_DELAY 150000 // Game speed in microseconds (e.g., 0.15 seconds per frame)

// Color Pair Definitions
#define PAIR_DEFAULT 1  // For general text, score
#define PAIR_SNAKE 2    // For the snake
#define PAIR_FOOD 3     // For the food item
#define PAIR_GAMEOVER 4 // For game over messages
#define PAIR_BORDER 5   // For the game border

// --- Data Structures ---

// Represents a 2D point (coordinate)
typedef struct
{
    int y; // Row
    int x; // Column
} Point;

// Represents the possible directions the snake can move
typedef enum
{
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_NONE
} Direction;

// Represents the snake
typedef struct
{
    Point body[MAX_SNAKE_LENGTH]; // Array of points for snake segments
    int length;                   // Current length of the snake
    Direction current_dir;        // Current movement direction
    Direction last_input_dir;     // Last direction to prevent 180-degree turns
} Snake;

// Represents the food item
typedef struct
{
    Point pos;   // Position of the food (relative to game window's 0,0)
    bool active; // Is food currently on the board?
    char symbol; // Character for food (e.g., '*')
} Food;

// Represents the overall state of the game
typedef struct
{
    Snake snake; // The snake
    Food food;   // The food
    int score;   // Current game score

    bool game_over; // True if game has ended
    bool quit_game; // True if user chose to quit (e.g., by pressing 'q')
    // bool paused; // Pause feature removed for MMVP for simplicity

    // Terminal dimensions and game window properties
    int term_rows, term_cols;               // Dimensions of the entire terminal
    int game_win_start_y, game_win_start_x; // Top-left (offset) of the game window on stdscr
    WINDOW *game_win;                       // ncurses window for the dedicated game area

    bool has_color_support; // True if the terminal supports colors
} GameState;

// --- Function Prototypes ---

// ncurses setup and cleanup
void init_ncurses_settings(GameState *state);
void cleanup_ncurses();

// Game state and game window setup
void init_game_state(GameState *state);
void calculate_game_area_start_coords(GameState *state);
void create_game_window(GameState *state);
void place_random_food(GameState *state);
bool check_terminal_size(const GameState *state); // To ensure terminal is big enough

// Input, Logic, Rendering
void process_input(GameState *state);
void update_game_logic(GameState *state);
void move_snake(GameState *state);
bool check_self_collision(const GameState *state);
void render_game(const GameState *state);
void display_game_over_message(const GameState *state);

// --- Main Function ---
int main()
{
    GameState game_state; // Holds all game information

    srand(time(NULL)); // Initialize random number generator

    init_ncurses_settings(&game_state); // Setup ncurses, colors

    getmaxyx(stdscr, game_state.term_rows, game_state.term_cols); // Get terminal dimensions

    // Check if terminal is large enough for the game area
    if (!check_terminal_size(&game_state))
    {
        cleanup_ncurses(); // Clean up ncurses before printing error to stderr
        fprintf(stderr, "Terminal is too small!\n");
        fprintf(stderr, "Required: %d columns x %d rows for game area.\n", GAME_AREA_WIDTH, GAME_AREA_HEIGHT);
        fprintf(stderr, "Current:  %d columns x %d rows.\n", game_state.term_cols, game_state.term_rows);
        return 1; // Exit with an error
    }

    // Calculate where the top-left of our game window should be to center it
    calculate_game_area_start_coords(&game_state);

    // Create the dedicated window for the game
    create_game_window(&game_state);

    // Initialize game variables (snake position, score, etc.)
    init_game_state(&game_state);
    place_random_food(&game_state); // Place initial food

    // Main game loop
    while (!game_state.game_over && !game_state.quit_game)
    {
        process_input(&game_state);     // Get user input
        update_game_logic(&game_state); // Update game state (move snake, check collisions)
        render_game(&game_state);       // Draw everything
        usleep(GAME_SPEED_DELAY);       // Pause for game speed
    }

    // Game loop finished (either game over or user quit)
    if (game_state.game_over)
    { // Only show "Game Over" if it wasn't a quit mid-game
        display_game_over_message(&game_state);
        wtimeout(game_state.game_win, -1); // Make wgetch blocking to wait for a key
        wgetch(game_state.game_win);
    }

    cleanup_ncurses(); // Restore terminal
    return 0;
}

// --- Function Implementations ---

/**
 * @brief Initializes ncurses settings: terminal mode, colors, cursor.
 */
void init_ncurses_settings(GameState *state)
{
    initscr();            // Start ncurses mode
    cbreak();             // Disable line buffering
    noecho();             // Don't echo typed characters
    curs_set(0);          // Make cursor invisible
    keypad(stdscr, TRUE); // Enable F-keys, arrow keys for stdscr (mainly for initial checks if any)

    // Initialize colors if supported
    if (has_colors())
    {
        state->has_color_support = true;
        if (start_color() == OK)
        {
            init_pair(PAIR_DEFAULT, COLOR_WHITE, COLOR_BLACK);
            init_pair(PAIR_SNAKE, COLOR_GREEN, COLOR_BLACK);
            init_pair(PAIR_FOOD, COLOR_RED, COLOR_BLACK);
            init_pair(PAIR_GAMEOVER, COLOR_YELLOW, COLOR_BLACK);
            init_pair(PAIR_BORDER, COLOR_BLUE, COLOR_BLACK);
        }
        else
        {
            state->has_color_support = false; // start_color() failed
        }
    }
    else
    {
        state->has_color_support = false;
    }
}

/**
 * @brief Restores terminal to its normal state.
 */
void cleanup_ncurses()
{
    if (stdscr)
    { // Ensure ncurses was initialized
        endwin();
    }
}

/**
 * @brief Calculates the top-left (y,x) screen coordinates for the game window.
 */
void calculate_game_area_start_coords(GameState *state)
{
    state->game_win_start_y = (state->term_rows - GAME_AREA_HEIGHT) / 2;
    state->game_win_start_x = (state->term_cols - GAME_AREA_WIDTH) / 2;
}

/**
 * @brief Creates the ncurses window for the game area.
 */
void create_game_window(GameState *state)
{
    state->game_win = newwin(GAME_AREA_HEIGHT, GAME_AREA_WIDTH,
                             state->game_win_start_y, state->game_win_start_x);
    if (state->game_win == NULL)
    {
        cleanup_ncurses();
        fprintf(stderr, "Error: Could not create game window.\n");
        exit(EXIT_FAILURE);
    }
    keypad(state->game_win, TRUE);  // Enable F-keys, arrow keys for this window
    wtimeout(state->game_win, 100); // Set non-blocking input for wgetch (100ms timeout)
}

/**
 * @brief Initializes game variables: snake, food, score, game flags.
 * Snake coordinates are relative to the game area's top-left (0,0).
 */
void init_game_state(GameState *state)
{
    state->snake.length = INITIAL_SNAKE_LENGTH;
    state->snake.current_dir = DIR_RIGHT; // Initial direction
    state->snake.last_input_dir = DIR_RIGHT;

    int start_y_rel = GAME_AREA_HEIGHT / 2; // Relative Y for snake head
    int start_x_rel = GAME_AREA_WIDTH / 4;  // Relative X for snake head

    for (int i = 0; i < state->snake.length; ++i)
    {
        state->snake.body[i].y = start_y_rel;
        state->snake.body[i].x = start_x_rel - i; // Snake extends leftwards
    }

    state->food.active = false; // Will be placed by place_random_food()
    state->food.symbol = '*';
    state->score = 0;
    state->game_over = false;
    state->quit_game = false;
    // state->paused removed for MMVP
}

/**
 * @brief Places food randomly within the game area, not on the snake.
 * Coordinates are relative to the game window.
 */
void place_random_food(GameState *state)
{
    if (!state->game_win)
        return; // Should have game_win by now

    bool placed_on_snake;
    do
    {
        placed_on_snake = false;
        // Food position is relative to the game window (0 to GAME_AREA_HEIGHT-1)
        state->food.pos.y = rand() % GAME_AREA_HEIGHT;
        state->food.pos.x = rand() % GAME_AREA_WIDTH;

        for (int i = 0; i < state->snake.length; ++i)
        {
            if (state->food.pos.y == state->snake.body[i].y &&
                state->food.pos.x == state->snake.body[i].x)
            {
                placed_on_snake = true;
                break;
            }
        }
    } while (placed_on_snake);
    state->food.active = true;
}

/**
 * @brief Checks if the terminal is large enough for the game area.
 */
bool check_terminal_size(const GameState *state)
{
    // This check is for the whole terminal. The game area itself is fixed.
    // We need enough space to *place* our fixed game area.
    return (state->term_rows >= GAME_AREA_HEIGHT && state->term_cols >= GAME_AREA_WIDTH);
}

/**
 * @brief Processes keyboard input from the user.
 */
void process_input(GameState *state)
{
    if (!state->game_win)
        return;
    int ch = wgetch(state->game_win); // Read from game window (non-blocking)

    switch (ch)
    {
    case 'h':
    case KEY_LEFT:
        if (state->snake.last_input_dir != DIR_RIGHT)
            state->snake.current_dir = DIR_LEFT;
        break;
    case 'j':
    case KEY_DOWN:
        if (state->snake.last_input_dir != DIR_UP)
            state->snake.current_dir = DIR_DOWN;
        break;
    case 'k':
    case KEY_UP:
        if (state->snake.last_input_dir != DIR_DOWN)
            state->snake.current_dir = DIR_UP;
        break;
    case 'l':
    case KEY_RIGHT:
        if (state->snake.last_input_dir != DIR_LEFT)
            state->snake.current_dir = DIR_RIGHT;
        break;
    // Pause ('p') removed for MMVP simplicity
    case 'q':
    case 'Q':
        state->quit_game = true;
        break;
    case ERR: // No input (wgetch timeout)
        break;
    }
}

/**
 * @brief Moves the snake, handles eating food, and wall pass-through.
 */
void move_snake(GameState *state)
{
    if (state->snake.current_dir == DIR_NONE && state->snake.length > 0)
        return;
    if (state->snake.length <= 0)
        return;

    Point new_head_pos = state->snake.body[0]; // Get current head
    // Calculate new head position based on direction
    switch (state->snake.current_dir)
    {
    case DIR_UP:
        new_head_pos.y--;
        break;
    case DIR_DOWN:
        new_head_pos.y++;
        break;
    case DIR_LEFT:
        new_head_pos.x--;
        break;
    case DIR_RIGHT:
        new_head_pos.x++;
        break;
    case DIR_NONE:
        return;
    }

    // Wall pass-through logic (within game area 0 to SIZE-1)
    if (new_head_pos.x < 0)
        new_head_pos.x = GAME_AREA_WIDTH - 1;
    else if (new_head_pos.x >= GAME_AREA_WIDTH)
        new_head_pos.x = 0;
    if (new_head_pos.y < 0)
        new_head_pos.y = GAME_AREA_HEIGHT - 1;
    else if (new_head_pos.y >= GAME_AREA_HEIGHT)
        new_head_pos.y = 0;

    // Check for eating food
    if (state->food.active && new_head_pos.x == state->food.pos.x && new_head_pos.y == state->food.pos.y)
    {
        state->score++;
        if (state->snake.length < MAX_SNAKE_LENGTH)
        {
            state->snake.length++; // Snake grows
        }
        place_random_food(state); // New food
    }

    // Move snake body: shift segments forward
    for (int i = state->snake.length - 1; i > 0; --i)
    {
        state->snake.body[i] = state->snake.body[i - 1];
    }
    state->snake.body[0] = new_head_pos; // Place new head

    state->snake.last_input_dir = state->snake.current_dir; // Update last moved direction
}

/**
 * @brief Checks if the snake has collided with itself.
 */
bool check_self_collision(const GameState *state)
{
    // Can't collide if very short
    if (state->snake.length < 4)
        return false; // Needs at least 4 segments to make a loop to hit itself

    Point head = state->snake.body[0];
    // Check head against rest of the body (from segment 1 onwards)
    for (int i = 1; i < state->snake.length; ++i)
    {
        if (head.x == state->snake.body[i].x && head.y == state->snake.body[i].y)
        {
            return true; // Collision
        }
    }
    return false; // No collision
}

/**
 * @brief Updates the main game logic: moves snake, checks for collisions.
 */
void update_game_logic(GameState *state)
{
    if (state->game_over || state->quit_game)
        return; // No updates if game ended

    move_snake(state); // Move snake and handle food

    if (check_self_collision(state))
    { // Check for collision after moving
        state->game_over = true;
    }
}

/**
 * @brief Renders all game elements (border, snake, food, score) in the game window.
 */
void render_game(const GameState *state)
{
    if (!state->game_win)
        return;

    werase(state->game_win); // Clear the game window

    // Draw border
    if (state->has_color_support)
        wattron(state->game_win, COLOR_PAIR(PAIR_BORDER));
    else
        wattron(state->game_win, A_BOLD);
    box(state->game_win, 0, 0); // Simple box around the window
    if (state->has_color_support)
        wattroff(state->game_win, COLOR_PAIR(PAIR_BORDER));
    else
        wattroff(state->game_win, A_BOLD);

    // Draw snake
    if (state->has_color_support)
        wattron(state->game_win, COLOR_PAIR(PAIR_SNAKE));
    else
        wattron(state->game_win, A_REVERSE);
    for (int i = 0; i < state->snake.length; ++i)
    {
        char display_char = (i == 0) ? '@' : 'o'; // Head '@', body 'o'
        mvwaddch(state->game_win, state->snake.body[i].y, state->snake.body[i].x, display_char);
    }
    if (state->has_color_support)
        wattroff(state->game_win, COLOR_PAIR(PAIR_SNAKE));
    else
        wattroff(state->game_win, A_REVERSE);

    // Draw food
    if (state->food.active)
    {
        if (state->has_color_support)
            wattron(state->game_win, COLOR_PAIR(PAIR_FOOD));
        else
            wattron(state->game_win, A_BOLD);
        mvwaddch(state->game_win, state->food.pos.y, state->food.pos.x, state->food.symbol);
        if (state->has_color_support)
            wattroff(state->game_win, COLOR_PAIR(PAIR_FOOD));
        else
            wattroff(state->game_win, A_BOLD);
    }

    // Draw score
    if (state->has_color_support)
        wattron(state->game_win, COLOR_PAIR(PAIR_DEFAULT));
    mvwprintw(state->game_win, 0, 2, "Score: %d", state->score); // Score at top-left inside border
    if (state->has_color_support)
        wattroff(state->game_win, COLOR_PAIR(PAIR_DEFAULT));

    // Pause message removed for MMVP

    wrefresh(state->game_win); // Refresh the game window to show changes
}

/**
 * @brief Displays the "GAME OVER" message and final score in the game window.
 */
void display_game_over_message(const GameState *state)
{
    if (!state->game_win)
        return;

    const char *msg_go = "GAME OVER";
    char msg_sc[30];
    sprintf(msg_sc, "Final Score: %d", state->score);

    // Center messages in the game window
    int y_go = GAME_AREA_HEIGHT / 2 - 1;
    int x_go = (GAME_AREA_WIDTH - strlen(msg_go)) / 2;
    int y_sc = GAME_AREA_HEIGHT / 2 + 1;
    int x_sc = (GAME_AREA_WIDTH - strlen(msg_sc)) / 2;

    if (state->has_color_support)
        wattron(state->game_win, COLOR_PAIR(PAIR_GAMEOVER) | A_BOLD);
    else
        wattron(state->game_win, A_REVERSE | A_BOLD);

    mvwprintw(state->game_win, y_go, x_go, "%s", msg_go);
    mvwprintw(state->game_win, y_sc, x_sc, "%s", msg_sc);

    if (state->has_color_support)
        wattroff(state->game_win, COLOR_PAIR(PAIR_GAMEOVER) | A_BOLD);
    else
        wattroff(state->game_win, A_REVERSE | A_BOLD);

    wrefresh(state->game_win); // Show game over messages
}