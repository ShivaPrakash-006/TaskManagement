# TASK MANAGEMENT

<!--toc:start-->

- [TASK MANAGEMENT](#task-management)
  - [FEATURES](#features)
  - [IMPLEMENTATION/ALGORITHM/GENERAL IDEA](#implementationalgorithmgeneral-idea)
  - [WORKING](#working)
  - [Bugs](#bugs)
  - [NOTES](#notes)
  <!--toc:end-->

Create A Task Manager for Your Daily needs.

## FEATURES

- [x] Moderate TUI
- [ ] Beautiful TUI
- [x] Add any task to the list.
- [x] Group tasks (Linked List?).
  - [x] Create and Assign Groups
  - [x] Filter by Group
- [x] Set deadlines for tasks.
- [x] Set priority for tasks.
- [x] Different Views:
  - [x] All
  - [x] Group
  - [x] Filter
  - [x] Priority
- [x] Edit Tasks.
- [x] Sort By Deadlines/Priority.
- [x] Sorting Modes
  - [x] Alphabetic
  - [x] Deadlines
  - [x] Priority
- [x] Set Tasks Pending/Completed.
- [x] Persistent Storage.

## IMPLEMENTATION/ALGORITHM/GENERAL IDEA

The Popular `ncurses` library is used for the TUI.
Use Linked Lists to Store Tasks. External Storage of Tasks will use JSON.
Merge Sort is used to sort by order.

## WORKING

There are two windows on startup: The Task Window which shows all the Tasks and the Description Window which shows the all the details of the window. Pressing _**F1**_ adds a new Task to the list and _**F2**_ deletes the currently selected task from the list.
Press F10 to exit

## Bugs

- Pressing Arrow Keys while task list empty crashes
- Does not automatically clear description screen
- `printWrapped` segmentation error randomly → I think I fixed it, but be aware.

## NOTES

None for now
