#include <cjson/cJSON.h>
#include <curses.h>
#include <float.h>
#include <menu.h>
#include <ncurses.h>
#include <panel.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Task {
  char title[32];
  char desc[512];
  char group[32];
  struct Task *linkedTasks;
  struct Task *nextTask;
} Task;

typedef struct Group {
  char name[32];
  struct Group *nextGroup;
} Group;

void clearWindow(WINDOW *window) {
  wclear(window);
  box(window, 0, 0);
}

void printTitle(WINDOW *window, char *text) {
  int width = getmaxx(window);
  mvwprintw(window, 0, (width - strlen(text)) / 2 - 1, " %s ", text);
}

void printWrapped(char *text, WINDOW *window, int startx, int starty) {
  char *letter = text;
  int x = startx, y = starty, width = getmaxx(window);
  wmove(window, startx, starty);

  while (*letter) {
    if (x >= width - 1) {
      y++;
      x = startx;
    }

    mvwaddch(window, y, x++, *letter++);
    wrefresh(window);
  }
}

void printTasks(Task *head, WINDOW *window, int currentTaskNo) {
  Task *temp = head;
  int taskNo = 0;
  wclear(window);
  box(window, 0, 0);
  printTitle(window, "Tasks");
  wmove(window, 1, 1);
  while (temp != NULL) {
    if (currentTaskNo == taskNo) {
      wattron(window, A_STANDOUT);
      wprintw(window, "%s", temp->title);
      wattroff(window, A_STANDOUT);
    } else
      wprintw(window, "%s", temp->title);
    temp = temp->nextTask;
    wmove(window, ++taskNo + 1, 1);
  }
  wrefresh(window);
}

void printInsertMenu(WINDOW *window, int currentItemNo) {
  char *items[] = {"Title", "Description", "Group", "Add", "Cancel"};
  int noOfItems = 5;
  int itemNo = 0;
  wclear(window);
  box(window, 0, 0);
  printTitle(window, "Add Task");
  wmove(window, 1, 1);
  while (itemNo < noOfItems) {
    if (currentItemNo == itemNo) {
      wattron(window, A_STANDOUT);
      wprintw(window, "%s", items[itemNo]);
      wattroff(window, A_STANDOUT);
    } else
      wprintw(window, "%s", items[itemNo]);
    wmove(window, ++itemNo + 1, 1);
  }
  wrefresh(window);
}

void printDesc(Task *task, WINDOW *window) {
  wclear(window);
  box(window, 0, 0);
  printTitle(window, "Description");
  wmove(window, 1, 1);
  printWrapped(task->desc, window, 1, 1);
  wrefresh(window);
}

void printField(WINDOW *window, Task *task, int fieldNo) {
  wclear(window);
  box(window, 0, 0);
  wmove(window, 1, 1);
  switch (fieldNo) {
  case 0:
    wprintw(window, "%s", task->title);
    break;

  case 1:
    printWrapped(task->desc, window, 1, 1);
    break;

  case 2:
    wprintw(window, "%s", task->group);
    break;

  case 3:
    wprintw(window, "Add Task");
  }
  wrefresh(window);
}

void printGroups(WINDOW *window, Group *groups, int currentGroupNo) {
  Group *group = groups;
  char *items[] = {"Create", "Cancel"};
  int groupNo = 0;
  wclear(window);
  box(window, 0, 0);
  printTitle(window, "Groups");
  wmove(window, 1, 1);
  while (group != NULL || groupNo < 2) {
    if (groupNo < 2) {
      if (currentGroupNo == groupNo) {
        wattron(window, A_STANDOUT);
        wprintw(window, "%s", items[groupNo]);
        wattroff(window, A_STANDOUT);
      } else {
        wprintw(window, "%s", items[groupNo]);
      }
    } else {
      if (currentGroupNo == groupNo) {
        wattron(window, A_STANDOUT);
        wprintw(window, "%s", group->name);
        wattroff(window, A_STANDOUT);
      } else
        wprintw(window, "%s", group->name);
      group = group->nextGroup;
    }
    wmove(window, ++groupNo + 1, 1);
  }
  wrefresh(window);
}

void createGroup(Group **head, WINDOW *window) {
  clearWindow(window);
  printTitle(window, "Create Group");
  Group *newGroup = (Group *)malloc(sizeof(Group));
  mvwprintw(window, 1, 1, "Enter Name of Group: ");
  wgetnstr(window, newGroup->name, sizeof(newGroup->name));
  // Inserting
  newGroup->nextGroup = *head;
  *head = newGroup;
}

Group *getGroup(Group *groups, int groupNo) {
  int i = 0;
  Group *temp = groups;
  while (i < groupNo) {
    temp = temp->nextGroup;
    i++;
  }
  return temp;
}

Group *selectGroup(Group **groups, WINDOW *window, unsigned int *size) {
  bool selecting = true;
  int currentGroupNo = 0, keyPress;
  int selectedGroupNo = -1;
  while (selecting) {
    printGroups(window, *groups, currentGroupNo);
    keyPress = getch();
    switch (keyPress) {
    case KEY_UP:
      if (currentGroupNo != 0)
        currentGroupNo--;
      else
        currentGroupNo = (*size) + 1;
      break;

    case KEY_DOWN:
      if (currentGroupNo != (*size) + 1)
        currentGroupNo++;
      else
        currentGroupNo = 0;
      break;
    case 10:
      selectedGroupNo = currentGroupNo;
      break;
    }

    if (selectedGroupNo == 0) {
      createGroup(groups, window);
      (*size)++;
    } else if (selectedGroupNo == 1) {
      return NULL;
    } else if (selectedGroupNo > 1) {
      selecting = false;
    }
    selectedGroupNo = -1;
  }
  return getGroup(*groups, selectedGroupNo);
}

Task *createTask(WINDOW *menuWindow, WINDOW *detailWindow, Group **groups,
                 unsigned int *groupListSize) {
  Task *newTask = (Task *)malloc(sizeof(Task));
  wclear(menuWindow);
  wclear(detailWindow);
  box(menuWindow, 0, 0);
  box(detailWindow, 0, 0);
  printTitle(menuWindow, "Add Task");

  bool creating = true;
  int currentItemNo = 0, keyPress;
  int selectedItemNo = -1;
  while (creating) {
    printInsertMenu(menuWindow, currentItemNo);
    keyPress = getch();
    switch (keyPress) {
    case KEY_UP:
      if (currentItemNo != 0)
        currentItemNo--;
      printField(detailWindow, newTask, currentItemNo);
      break;
    case KEY_DOWN:
      if (currentItemNo != 4)
        currentItemNo++;
      printField(detailWindow, newTask, currentItemNo);
      break;
    case 10:
      selectedItemNo = currentItemNo;
      break;
    }
    if (selectedItemNo == 3)
      creating = false;
    else if (selectedItemNo == 0) {
      curs_set(1);
      wmove(detailWindow, 1, 1);
      wgetnstr(detailWindow, newTask->title, sizeof(newTask->title));
      curs_set(0);
    } else if (selectedItemNo == 1) {
      curs_set(1);
      wmove(detailWindow, 1, 1);
      wgetnstr(detailWindow, newTask->desc, sizeof(newTask->desc));
      curs_set(0);
    } else if (selectedItemNo == 2) {
      strcpy(newTask->group,
             selectGroup(groups, detailWindow, groupListSize)->name);
    } else if (selectedItemNo == 4) {
      wclear(detailWindow);
      box(detailWindow, 0, 0);
      printTitle(detailWindow, "Delete");
      mvwprintw(detailWindow, 1, 1, "Are you sure? (y/n)");
      char choice = wgetch(detailWindow);

      if (choice == 'y')
        return NULL;
    }
    selectedItemNo = -1;
  }
  newTask->nextTask = NULL;
  return newTask;
}

void insert(Task **head, Task *newTask) {
  if (*head == NULL)
    *head = newTask;
  else {
    Task *temp = *head;
    while (temp->nextTask != NULL)
      temp = temp->nextTask;
    temp->nextTask = newTask;
  }
}

void delete(Task **head, int pos, WINDOW *window) {
  wclear(window);
  box(window, 0, 0);
  printTitle(window, "Delete");
  mvwprintw(window, 1, 1, "Are you sure? (y/n)");
  char choice = wgetch(window);

  if (*head == NULL || choice != 'y') // Empty
    return;

  else if ((*head)->nextTask == NULL) {
    free(*head);
    *head = NULL;
  }

  else if (pos == 0) { // Delete Beginning
    Task *toFree = *head;
    *head = (*head)->nextTask;
    free(toFree);
  }

  else if (pos == -1) { // Delete End
    Task *temp = *head;
    while (temp->nextTask->nextTask != NULL)
      temp = temp->nextTask;
    free(temp->nextTask);
    temp->nextTask = NULL;
  }

  else if (pos > 0) { // Delete Middle
    Task *temp = *head;
    int i = 1;
    while (i < pos && temp->nextTask->nextTask != NULL) {
      temp = temp->nextTask;
      i++;
    }
    Task *toFree = temp->nextTask;
    temp->nextTask = temp->nextTask->nextTask;
    free(toFree);
  }
}

Task *getTask(Task *head, int taskNo) {
  int i = 0;
  Task *temp = head;
  while (i != taskNo) {
    temp = temp->nextTask;
    i++;
  }
  return temp;
}

int main() {
  Task *taskHead = NULL, *newTask;
  Group *groupHead = NULL, *newGroup;

  int choice;
  bool deleteMode = false, run = true;
  char title[32], desc[128];
  unsigned int size = 0, currentTaskNo = 0, task = 0, groupListSize = 0;

  initscr();
  keypad(stdscr, true);
  cbreak();
  curs_set(0);
  WINDOW *taskWindow = newwin(LINES, COLS / 2, 0, 0);
  WINDOW *descWindow = newwin(LINES, COLS / 2, 0, COLS / 2);
  wclear(taskWindow);
  wclear(descWindow);
  box(taskWindow, 0, 0);
  box(descWindow, 0, 0);
  refresh();
  wrefresh(taskWindow);
  wrefresh(descWindow);

  while (run) {
    printTasks(taskHead, taskWindow, currentTaskNo);
    choice = getch();
    switch (choice) {
    case KEY_UP:
      if (currentTaskNo == 0)
        currentTaskNo = size - 1;
      else
        currentTaskNo--;
      printDesc(getTask(taskHead, currentTaskNo), descWindow);
      break;
    case KEY_DOWN:
      if (currentTaskNo == size - 1)
        currentTaskNo = 0;
      else
        currentTaskNo++;
      printDesc(getTask(taskHead, currentTaskNo), descWindow);
      break;
    case KEY_F(1):
      newTask = createTask(taskWindow, descWindow, &groupHead, &groupListSize);
      if (newTask)
        insert(&taskHead, newTask);
      size++;
      break;
    case KEY_F(2):
      if (size != 0)
        deleteMode = true;
      break;
    case 10: // ENTER
      task = currentTaskNo;
      break;
    case KEY_F(10):
      run = false;
      break;
    }

    if (deleteMode) {
      delete (&taskHead, currentTaskNo, taskWindow);
      if (currentTaskNo == size - 1)
        currentTaskNo--;
      size--;
      deleteMode = false;
    }
  }
  endwin();

  return 0;
}
