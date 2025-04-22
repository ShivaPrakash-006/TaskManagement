# TASK MANAGEMENT

<!--toc:start-->

- [TASK MANAGEMENT](#task-management)
  - [FEATURES](#features)
  - [IMPLEMENTATION/ALGORITHM/GENERAL IDEA](#implementationalgorithmgeneral-idea)
  - [WORKING](#working)
  - [Bugs](#bugs)
  <!--toc:end-->

Create A Task Manager for Your Daily needs.

## FEATURES

- [ ] Beautiful TUI
- [x] Add any task to the list.
- [-] Group tasks (Linked List?).
  - [x] Create and Assign Groups
  - [-] Filter by Group
- [ ] Link tasks (Graphs).
- [ ] Set deadlines for tasks.
- [ ] Set priority for tasks.
- [ ] Different Views:
  - [ ] All
  - [ ] Group
  - [ ] Filter
- [ ] Sort By Deadlines/Priority.
- [ ] Sub Tasks(?).
- [ ] Set Tasks Active/Inactive.
- [ ] Persistent Storage.

## IMPLEMENTATION/ALGORITHM/GENERAL IDEA

The Popular `ncurses` library is used for the TUI.
Use Linked Lists to Store Tasks. External Storage of Tasks will use (JSON/SQLite).

## WORKING

There are two windows on startup: The Task Window which shows all the Tasks and the Description Window which shows the all the details of the window. Pressing _**F1**_ adds a new Task to the list and _**F2**_ deletes the currently selected task from the list.
Press F10 to exit

## Bugs

- Pressing Arrow Keys while task list empty crashes
- Does not automatically clear description screen
-
