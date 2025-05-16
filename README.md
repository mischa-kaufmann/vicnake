# ViCnake - A Terminal-Based Snake Game

This project is my submission for the CS50x final project.

**⚠️ CURRENT STATUS: VERY EARLY DEVELOPMENT - HIGHLY EXPERIMENTAL & BUGGY ⚠️**

Please be aware that this game is currently in a **very early and unstable development phase**. The core idea is implemented, but it is **quite buggy and not yet reliably playable.** This repository reflects ongoing work and learning.

## The Idea: ViCnake

ViCnake is a classic Snake game built from scratch in **C** using the **`ncurses` library** for all terminal graphics and input handling. The "Vi" in ViCnake hints at one of the core design choices: the snake is controlled using the Vim-style navigation keys (`h`, `j`, `k`, `l`).

**Core Concepts:**
* **Classic Gameplay:** The player controls a snake that grows by eating food items. The game ends if the snake collides with itself.
* **Terminal Interface:** The entire game runs within the text-based terminal environment, leveraging `ncurses` for screen manipulation.
* **Vim Controls:** Using `h` (left), `j` (down), `k` (up), and `l` (right) for movement offers a different interaction style, especially for those familiar with the Vim editor.
* **Learning C and `ncurses`:** A primary goal of this project is to deepen understanding and practical skills in C programming and terminal UI development with `ncurses`.

## The Plan & Roadmap

This project started as a "Minimal Minimal Viable Product" (MMVP) to get the absolute basics. The plan is to build upon this foundation.

**Current MMVP Features (Attempted):**
* Snake movement with `hjkl`.
* Food that appears on the screen.
* Snake grows when it eats food.
* Game ends on self-collision.
* Snake can pass through the walls of the game area.
* A fixed game speed.
* Simple "Game Over" message with the score.

**Short-term Goals (Addressing Current State):**
1.  **Stabilize Core Gameplay:**
    * Thoroughly debug and fix issues with snake movement logic.
    * Ensure reliable self-collision detection.
    * Improve input responsiveness and consistency.
    * Fix any visual glitches or drawing errors.
2.  **Code Cleanup & Refactoring:**
    * Improve code structure and readability as understanding grows.
    * Add more comprehensive comments.

**Medium-term Goals (Planned Features):**
* **Robust Startup Screen:** A welcoming screen with clear instructions and game options.
* **Persistent Leaderboard:** Save and display high scores (likely file-based).
* **Variable Game Speed / Difficulty Levels:** Allow the player to choose the challenge.
* **Improved UI/UX:**
    * Better visual feedback.
    * More polished game over screen.
    * (Possibly) Simple ASCII art enhancements.

**Long-term / Stretch Goals (Future Ideas):**
* Multiple types of food with different effects.
* Obstacles or different level layouts (beyond the current pass-through walls).
* (Highly Ambitious) Exploring ways to make it accessible via a web terminal on a VPS.

## Technologies Used

* **Programming Language:** C (targeting C11 standard)
* **UI Library:** `ncurses`
* **Build System:** `make` (using a `Makefile`)
* **Version Control:** `git` and GitHub

## How to Compile and Run

**Prerequisites:**
* A C compiler (like GCC)
* `ncurses` development libraries (e.g., `libncurses-dev` on Debian/Ubuntu)
* `make`

**Compilation:**
1.  Clone this repository: `git clone https://github.com/mischa-kaufmann/vicnake.git`
2.  Navigate to the project directory: `cd vicnake`
3.  Compile the game: `make`

**Running the Game:**
```bash
./vicnake