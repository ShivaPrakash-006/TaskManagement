#include <cjson/cJSON.h>
#include <curses.h>
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

Task *createTask() {
  Task *newTask = (Task *)malloc(sizeof(Task));
  clear();
  printw("Title: ");
  getnstr(newTask->title, 32);
  printw("Description: ");
  getnstr(newTask->desc, 128);
  newTask->nextTask = NULL;
  return newTask;
}

void insert(Task **head) {
  Task *newTask = createTask();
  if (*head == NULL)
    *head = newTask;
  else {
    Task *temp = *head;
    while (temp->nextTask != NULL)
      temp = temp->nextTask;
    temp->nextTask = newTask;
  }
}

void delete(Task **head, int pos) {
  clear();
  printw("Are you sure? (y/n)");
  char choice = getch();

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

void printMenu(Task *head, int currentTaskNo) {
  Task *temp = head;
  int taskNo = 0;
  clear();
  move(0, 0);
  while (temp != NULL) {
    if (currentTaskNo == taskNo) {
      attron(A_STANDOUT);
      printw("%s", temp->title);
      attroff(A_STANDOUT);
    } else
      printw("%s", temp->title);
    temp = temp->nextTask;
    move(++taskNo, 0);
    refresh();
  }
}

WINDOW *createDescWindow(Task **head, int task) {}

int main() {
  Task *head = NULL;

  int choice;
  bool deleteMode = false, run = true;
  char title[32], desc[128];
  unsigned int size = 0, currentTaskNo = 0, task = 0;

  initscr();
  keypad(stdscr, true);
  cbreak();

  while (run) {
    printMenu(head, currentTaskNo);
    choice = getch();
    switch (choice) {
    case KEY_UP:
      if (currentTaskNo == 0)
        currentTaskNo = size - 1;
      else
        currentTaskNo--;
      break;
    case KEY_DOWN:
      if (currentTaskNo == size - 1)
        currentTaskNo = 0;
      else
        currentTaskNo++;
      break;
    case KEY_F(1):
      insert(&head);
      size++;
      break;
    case KEY_F(2):
      if (size != 0)
        deleteMode = true;
      break;
    case 10:
      task = currentTaskNo;
      break;
    case KEY_F(10):
      run = false;
      break;
    }

    if (deleteMode) {
      delete (&head, task);
      size--;
      deleteMode = false;
    }
  }
  endwin();

  return 0;
}
