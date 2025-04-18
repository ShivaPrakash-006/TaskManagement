#include <cjson/cJSON.h>
#include <curses.h>
#include <float.h>
#include <menu.h>
#include <ncurses.h>
#include <panel.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct Task {
  char title[32];
  char desc[128];
  char group[32];
  struct Task *linkedTasks;
  struct Task *nextTask;
} Task;

Task *createTask(WINDOW *window) {
  Task *newTask = (Task *)malloc(sizeof(Task));
  wclear(window);
  box(window, 0, 0);
  wmove(window, 1, 1);
  wprintw(window, "Title: ");
  wgetnstr(window, newTask->title, 32);
  wmove(window, 2, 1);
  wprintw(window, "Description: ");
  wgetnstr(window, newTask->desc, 128);
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

void printMenu(Task *head, WINDOW *window, int currentTaskNo) {
  Task *temp = head;
  int taskNo = 0;
  wclear(window);
  box(window, 0, 0);
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

void printDesc(Task *task, WINDOW *window) {
  wclear(window);
  box(window, 0, 0);
  wmove(window, 1, 1);
  wprintw(window, "%s", task->desc);
  wrefresh(window);
}

int main() {
  Task *head = NULL, *newTask;

  int choice;
  bool deleteMode = false, run = true;
  char title[32], desc[128];
  unsigned int size = 0, currentTaskNo = 0, task = 0;

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
    printMenu(head, taskWindow, currentTaskNo);
    choice = getch();
    switch (choice) {
    case KEY_UP:
      if (currentTaskNo == 0)
        currentTaskNo = size - 1;
      else
        currentTaskNo--;
      printDesc(getTask(head, currentTaskNo), descWindow);
      break;
    case KEY_DOWN:
      if (currentTaskNo == size - 1)
        currentTaskNo = 0;
      else
        currentTaskNo++;
      printDesc(getTask(head, currentTaskNo), descWindow);
      break;
    case KEY_F(1):
      newTask = createTask(taskWindow);
      insert(&head, newTask);
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
      delete (&head, currentTaskNo, taskWindow);
      if (currentTaskNo == size - 1)
        currentTaskNo--;
      size--;
      deleteMode = false;
    }
  }
  endwin();

  return 0;
}
