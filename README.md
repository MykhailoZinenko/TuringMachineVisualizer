# Turing Machine Visualizer - Development Roadmap

This document outlines the planned tasks and improvements for the Turing Machine Visualizer application.

## Code Refactoring

- **[REFACTOR]** Cleanup AI-generated code and rearrange methods for better organization
- **[REFACTOR]** Improve include structure (consider using pragma once instead of include guards)
- **[REFACTOR]** Ensure proper separation of .h and .cpp files throughout the codebase
- **[REFACTOR]** Better separate concerns (UI/model/etc.) with clear architectural boundaries
- **[REFACTOR]** Extract functionality from large classes (e.g., commands from MainWindow) to improve modularity

## Core Functionality Improvements

- **[FEATURE]** Add global hotkey manager for consistent keyboard shortcuts throughout the application
- **[FEATURE]** Implement "stop" functionality distinct from run/pause
- **[FEATURE]** Support copy/paste operations for tape content
- **[FEATURE]** Add support for saving multiple tapes in tabs within one project
- **[FEATURE]** Disable tape editing during runtime (separate input tape from output)

## UI Enhancements

- **[FEATURE]** Make UI fully flexible with draggable, droppable, and resizable components
- **[FEATURE]** Add better visual feedback during machine execution
- **[FEATURE]** Support history viewing of previous runs to examine tape state during each transition
- **[FEATURE]** Add underline/overline support for characters in the tape
- **[FEATURE]** Support multiple characters in one tape cell
- **[FEATURE]** Generate visual graph representations of the state machine

## Major Enhancements

- **[MAJOR]** Design and implement a high-level "language" framework for programmatically generating states and transitions
  - Support operations like `lst = fill(from 'a' to 'z')` 
  - Add functions like `find_last()`, `find_first()`, etc.
- **[MAJOR]** Implement multi-tape Turing machine support
