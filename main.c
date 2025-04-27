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
#include <time.h>

typedef enum Priority { Urgent = 1, Normal = 2, Casual = 3 } Priority;

typedef struct Task {
  char title[32];
  char desc[512];
  char group[32];
  bool completed;
  struct tm *deadline;
  Priority priority;
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
  wmove(window, starty, startx);

  while (*letter) {
    if (x >= width - 1) {
      y++;
      x = startx;
    }

    mvwaddch(window, y, x++, *letter++);
    wrefresh(window);
  }
  y += 2;
  wmove(window, y, startx);
}

void printTasks(Task *head, WINDOW *window, int currentTaskNo, char *group,
                char *filterStr) {
  Task *temp = head;
  int taskNo = 0;
  int lineNo = 3;
  wclear(window);
  box(window, 0, 0);
  printTitle(window, "Tasks");
  mvwprintw(window, 1, 1, "Search (/): ");
  wmove(window, 3, 1);
  while (temp != NULL) {
    if ((strcmp(temp->group, "") != 0 && strcmp(temp->group, group) == 0) ||
        strcmp(group, "") == 0 && strcasestr(temp->title, filterStr) != NULL) {
      if (currentTaskNo == taskNo) {
        wattron(window, A_STANDOUT);
        wprintw(window, "%s", temp->title);
        wattroff(window, A_STANDOUT);
      } else
        wprintw(window, "%s", temp->title);
      wmove(window, ++lineNo, 1);
    }
    taskNo++;
    temp = temp->nextTask;
  }
  wrefresh(window);
}

void printInsertMenu(WINDOW *window, int currentItemNo) {
  char *items[] = {"Title",    "Description", "Group", "Deadline",
                   "Priority", "Add",         "Cancel"};
  int noOfItems = 7;
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
  printWrapped(task->desc, window, 1, 1);
  int y = sizeof(task->desc) / getmaxx(window);
  mvwprintw(window, ++y, 1, "Group: %s", task->group);
  mvwprintw(window, ++y, 1, "Deadline: %s", asctime(task->deadline));

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
    wprintw(window, "%s", asctime(task->deadline));
    break;

  case 4:
    wprintw(window, "%i", task->priority);
    break;

  case 5:
    wprintw(window, "Add Task");
  }
  wrefresh(window);
}

void printGroups(WINDOW *window, Group *groups, int currentGroupNo,
                 char *filterStr) {
  Group *group = groups;
  char *items[] = {"Create", "Cancel"};
  int lineNo = 3, groupNo = 0;
  wclear(window);
  box(window, 0, 0);
  printTitle(window, "Groups");
  mvwprintw(window, 1, 1, "Search (/): ");
  wmove(window, 3, 1);
  while (group != NULL || groupNo < 2) {
    if (groupNo < 2) {
      if (currentGroupNo == groupNo) {
        wattron(window, A_STANDOUT);
        wprintw(window, "%s", items[groupNo]);
        wattroff(window, A_STANDOUT);
      } else {
        wprintw(window, "%s", items[groupNo]);
      }
      wmove(window, ++lineNo, 1);
    } else {
      if (strcasestr(group->name, filterStr) != NULL) {
        if (currentGroupNo == groupNo) {
          wattron(window, A_STANDOUT);
          wprintw(window, "%s", group->name);
          wattroff(window, A_STANDOUT);
        } else
          wprintw(window, "%s", group->name);
        wmove(window, ++lineNo, 1);
      }
      group = group->nextGroup;
    }
    groupNo++;
  }
  wrefresh(window);
}

void createGroup(Group **head, WINDOW *window) {
  echo();
  curs_set(1);
  clearWindow(window);
  printTitle(window, "Create Group");
  Group *newGroup = (Group *)malloc(sizeof(Group));
  mvwprintw(window, 1, 1, "Enter Name of Group: ");
  wgetnstr(window, newGroup->name, sizeof(newGroup->name));
  // Inserting
  newGroup->nextGroup = *head;
  *head = newGroup;
  noecho();
  curs_set(0);
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

bool filterGroup(Group *groups, char *filterStr, unsigned int groupNo) {
  Group *group = getGroup(groups, groupNo - 2);
  return strcasestr(group->name, filterStr) != NULL;
}

Group *selectGroup(Group **groups, WINDOW *window, unsigned int *size) {
  bool selecting = true, searching = false;
  int currentGroupNo = 0, keyPress;
  int selectedGroupNo = -1;
  char filterStr[32] = "";
  while (selecting) {
    printGroups(window, *groups, currentGroupNo, filterStr);
    selectedGroupNo = -1;
    keyPress = getch();
    switch (keyPress) {
    case KEY_UP:
      for (int i = 0; i < (*size) + 1; i++) {
        if (currentGroupNo != 0)
          currentGroupNo--;
        else
          currentGroupNo = (*size) + 1;
        if (currentGroupNo == 0 || currentGroupNo == 1 ||
            filterGroup(*groups, filterStr, currentGroupNo))
          break;
      }
      break;
    case KEY_DOWN:
      for (int i = 0; i < (*size) + 1; i++) {
        if (currentGroupNo != (*size) + 1)
          currentGroupNo++;
        else
          currentGroupNo = 0;
        if (currentGroupNo == 0 || currentGroupNo == 1 ||
            filterGroup(*groups, filterStr, currentGroupNo))
          break;
      }
      break;

    case '/':
      searching = true;
      break;

    case 10:
      selectedGroupNo = currentGroupNo;
      break;
    }

    if (searching) {
      echo();
      curs_set(1);
      mvwprintw(window, 1, 1, "Search (/): ");
      wgetnstr(window, filterStr, 32);
      wrefresh(window);
      searching = false;
      curs_set(0);
      noecho();
    }

    if (selectedGroupNo == 0) {
      createGroup(groups, window);
      (*size)++;
    } else if (selectedGroupNo == 1) {
      return NULL;
    } else if (selectedGroupNo > 1) {
      selecting = false;
    }
  }
  return getGroup(*groups, selectedGroupNo - 2);
}

Task *createTask(WINDOW *menuWindow, WINDOW *detailWindow, Group **groups,
                 unsigned int *groupListSize) {
  echo();
  Task *newTask = (Task *)malloc(sizeof(Task));
  time_t currRawTime = time(NULL);
  newTask->deadline = localtime(&currRawTime);
  Group *newGroup = NULL;
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
      if (currentItemNo != 6)
        currentItemNo++;
      printField(detailWindow, newTask, currentItemNo);
      break;
    case 10:
      selectedItemNo = currentItemNo;
      break;
    }
    if (selectedItemNo == 5)
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
      newGroup = selectGroup(groups, detailWindow, groupListSize);
      if (newGroup != NULL)
        strcpy(newTask->group, newGroup->name);
      echo();
    } else if (selectedItemNo == 3) {
      curs_set(1);
      wmove(detailWindow, 1, 1);
      wprintw(detailWindow, "(DD-MM-YY HH:MM): ");
      int day, month, year, hr, min;
      wscanw(detailWindow, "%d-%d-%d %d:%d", &day, &month, &year, &hr, &min);
      newTask->deadline->tm_mday = day;
      newTask->deadline->tm_mon = month - 1;
      newTask->deadline->tm_year = 100 + year;
      newTask->deadline->tm_hour = hr;
      newTask->deadline->tm_min = min;
      newTask->deadline->tm_sec = 0;
      mktime(newTask->deadline);
      curs_set(0);
    } else if (selectedItemNo == 6) {
      wclear(detailWindow);
      box(detailWindow, 0, 0);
      printTitle(detailWindow, "Cancel?");
      mvwprintw(detailWindow, 1, 1, "Are you sure? (y/n)");
      char choice = wgetch(detailWindow);

      if (choice == 'y') {
        noecho();
        return NULL;
      }
    }
    selectedItemNo = -1;
  }
  newTask->nextTask = NULL;
  noecho();
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

bool taskInGroup(Task *head, char *group, unsigned int taskNo) {
  Task *task = getTask(head, taskNo);
  return (strcmp(group, "") == 0 || strcmp(task->group, group) == 0);
}

bool filterTask(Task *head, char *filterStr, unsigned int taskNo) {
  Task *task = getTask(head, taskNo);
  return strcasestr(task->title, filterStr) != NULL;
}

int main() {
  Task *taskHead = NULL, *newTask;
  Group *groupHead = NULL, *newGroup;

  int choice;
  bool deleteMode = false, run = true, searching = false;
  char title[32], desc[128], group[32] = "", filterStr[32] = "";
  unsigned int size = 0, currentTaskNo = 0, task = 0, groupListSize = 0;

  initscr();
  keypad(stdscr, true);
  cbreak();
  noecho();
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
    printTasks(taskHead, taskWindow, currentTaskNo, group, filterStr);
    choice = getch();
    switch (choice) {
    case KEY_UP:
      for (int i = 0; i < size; i++) {
        if (currentTaskNo == 0)
          currentTaskNo = size - 1;
        else
          currentTaskNo--;
        if (taskInGroup(taskHead, group, currentTaskNo) &&
            filterTask(taskHead, filterStr, currentTaskNo))
          break;
      }
      printDesc(getTask(taskHead, currentTaskNo), descWindow);
      break;
    case KEY_DOWN:
      for (int i = 0; i < size; i++) {
        if (currentTaskNo == size - 1)
          currentTaskNo = 0;
        else
          currentTaskNo++;
        if (taskInGroup(taskHead, group, currentTaskNo) &&
            filterTask(taskHead, filterStr, currentTaskNo))
          break;
      }
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
    case KEY_F(3):
      newGroup = selectGroup(&groupHead, descWindow, &groupListSize);
      if (size != 0 && newGroup != NULL) {
        strcpy(group, newGroup->name);
      } else {
        strcpy(group, "");
      }
    case 10: // ENTER
      task = currentTaskNo;
      break;

    case '/':
      searching = true;
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

    if (searching) {
      echo();
      curs_set(1);
      mvwprintw(taskWindow, 1, 1, "Search (/): ");
      wgetnstr(taskWindow, filterStr, 32);
      wrefresh(taskWindow);
      searching = false;
      curs_set(0);
      noecho();
    }
  }
  endwin();

  return 0;
}
