#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum Priority { Urgent = 0, Normal = 1, Casual = 2 } Priority;
typedef enum Sort { Prior = 0, Name = 1, Deadline = 2 } Sort;

char *priorityToStr(Priority priority) {
  if (priority == Urgent)
    return "Urgent";
  else if (priority == Normal)
    return "Normal";
  else if (priority == Casual)
    return "Casual";
  else
    return "";
}

char *sortToStr(Sort sortingMethod) {
  if (sortingMethod == Prior)
    return "Priority";
  else if (sortingMethod == Name)
    return "Name";
  else if (sortingMethod == Deadline)
    return "Deadline";
  return "";
}

typedef struct Task {
  char title[32];
  char desc[256];
  char group[32];
  struct tm deadline;
  Priority priority;
  struct Task *nextTask;
} Task;

Task *getTask(Task *head, int taskNo) {
  int i = 0;
  Task *temp = head;
  while (i < taskNo) {
    temp = temp->nextTask;
    i++;
  }
  return temp;
}

bool taskInGroup(Task *head, char *group, uint taskNo) {
  Task *task = getTask(head, taskNo);
  return (strcmp(group, "") == 0 || strcmp(task->group, group) == 0);
}

bool filterTask(Task *head, char *filterStr, uint taskNo) {
  Task *task = getTask(head, taskNo);
  return strcasestr(task->title, filterStr) != NULL;
}

bool taskPriority(Task *head, int priority, uint taskNo) {
  if (priority == -1)
    return true;
  else if (priority == getTask(head, taskNo)->priority)
    return true;
  return false;
}

typedef int (*TaskComparator)(Task *, Task *);

int compareName(Task *A, Task *B) { return strcmp(A->title, B->title); }

int compareDeadline(Task *A, Task *B) {
  return difftime(mktime(&A->deadline), mktime(&B->deadline));
}

int comparePriority(Task *A, Task *B) { return A->priority - B->priority; }

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
                char *filterStr, int sortedPriority, int active) {

  Task *temp = head;
  int taskNo = 0;
  int lineNo = 3;
  wattron(window, COLOR_PAIR(4));
  wclear(window);
  box(window, 0, 0);
  if (active)
    printTitle(window, "Completed Tasks");
  else
    printTitle(window, "Pending Tasks");
  wattroff(window, COLOR_PAIR(4));
  wattron(window, COLOR_PAIR(5));
  mvwprintw(window, 1, 1, "Search (/): ");
  wmove(window, 3, 1);
  while (temp != NULL) {
    if ((strcmp(temp->group, "") != 0 && strcmp(temp->group, group) == 0) ||
        strcmp(group, "") == 0 && strcasestr(temp->title, filterStr) != NULL &&
            (sortedPriority == -1 || temp->priority == sortedPriority)) {
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
  wattroff(window, COLOR_PAIR(5));
  wrefresh(window);
}

void printInsertMenu(WINDOW *window, int currentItemNo) {
  char *items[] = {"Title",    "Description", "Group", "Deadline",
                   "Priority", "Add",         "Cancel"};
  int noOfItems = 7;
  int itemNo = 0;
  wattron(window, COLOR_PAIR(4));
  wclear(window);
  box(window, 0, 0);
  printTitle(window, "Add Task");
  wattroff(window, COLOR_PAIR(4));
  wattron(window, COLOR_PAIR(5));
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
  wattroff(window, COLOR_PAIR(5));
  wrefresh(window);
}

void printDetails(Task *task, WINDOW *window, uint detailNo, int active) {
  wattron(window, COLOR_PAIR(4));
  wclear(window);
  box(window, 0, 0);
  printTitle(window, "Description");
  wattroff(window, COLOR_PAIR(4));
  wattron(window, COLOR_PAIR(3));
  if (detailNo == 0)
    wattron(window, A_STANDOUT);
  printWrapped(task->desc, window, 1, 1);
  wattroff(window, A_STANDOUT);

  int y = sizeof(task->desc) / getmaxx(window);

  if (detailNo == 1)
    wattron(window, A_STANDOUT);
  mvwprintw(window, ++y, 1, "Group: %s", task->group);
  wattroff(window, A_STANDOUT);
  if (detailNo == 2)
    wattron(window, A_STANDOUT);
  mvwprintw(window, ++y, 1, "Deadline: %s", asctime(&task->deadline));
  wattroff(window, A_STANDOUT);
  if (detailNo == 3)
    wattron(window, A_STANDOUT);
  mvwprintw(window, ++y, 1, "Priority: %s", priorityToStr(task->priority));
  wattroff(window, A_STANDOUT);
  if (detailNo == 4)
    wattron(window, A_STANDOUT);
  if (!active)
    mvwprintw(window, ++y + 1, 1, "Mark as Completed");
  else
    mvwprintw(window, ++y + 1, 1, "Mark as Pending");
  wattroff(window, A_STANDOUT);
  wattroff(window, COLOR_PAIR(3));

  wrefresh(window);
}

void printField(WINDOW *window, Task *task, int fieldNo) {
  wattron(window, COLOR_PAIR(4));
  wclear(window);
  box(window, 0, 0);
  wattroff(window, COLOR_PAIR(4));
  wattron(window, COLOR_PAIR(3));
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
    wprintw(window, "%s", asctime(&task->deadline));
    break;

  case 4:
    wprintw(window, "%s", priorityToStr(task->priority));
    break;

  case 5:
    wprintw(window, "Add Task");
  }
  wrefresh(window);
  wattroff(window, COLOR_PAIR(3));
}

void printGroups(WINDOW *window, Group *groups, int currentGroupNo,
                 char *filterStr) {
  Group *group = groups;
  char *items[] = {"Create", "Cancel"};
  int lineNo = 3, groupNo = 0;
  wattron(window, COLOR_PAIR(4));
  wclear(window);
  box(window, 0, 0);
  printTitle(window, "Groups");
  wattroff(window, COLOR_PAIR(4));
  wattron(window, COLOR_PAIR(3));
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
  wattroff(window, COLOR_PAIR(3));
  wrefresh(window);
}

int deleteGroup(Group **head, int pos, WINDOW *window) {
  if (window != NULL) {
    wattron(window, COLOR_PAIR(1));
    wclear(window);
    box(window, 0, 0);
    printTitle(window, "Delete");
    mvwprintw(window, 1, 1, "Are you sure? (y/n)");
    wattroff(window, COLOR_PAIR(1));
    char choice = wgetch(window);
    if (choice != 'y')
      return 0;
  }
  if (*head == NULL) // Empty
    return 0;

  else if ((*head)->nextGroup == NULL) {
    free(*head);
    *head = NULL;
  }

  else if (pos == 0) { // Delete Beginning
    Group *toFree = *head;
    *head = (*head)->nextGroup;
    free(toFree);
  }

  else if (pos == -1) { // Delete End
    Group *temp = *head;
    while (temp->nextGroup->nextGroup != NULL)
      temp = temp->nextGroup;
    free(temp->nextGroup);
    temp->nextGroup = NULL;
  }

  else if (pos > 0) { // Delete Middle
    Group *temp = *head;
    int i = 1;
    while (i < pos && temp->nextGroup->nextGroup != NULL) {
      temp = temp->nextGroup;
      i++;
    }
    Group *toFree = temp->nextGroup;
    temp->nextGroup = temp->nextGroup->nextGroup;
    free(toFree);
  }
  return 1;
}

void createGroup(Group **head, WINDOW *window) {
  echo();
  curs_set(1);
  wattron(window, COLOR_PAIR(4));
  clearWindow(window);
  printTitle(window, "Create Group");
  wattroff(window, COLOR_PAIR(4));
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

bool filterGroup(Group *groups, char *filterStr, uint groupNo) {
  Group *group = getGroup(groups, groupNo - 2);
  return strcasestr(group->name, filterStr) != NULL;
}

Group *selectGroup(Group **groups, WINDOW *window, uint *size) {
  bool selecting = true, searching = false, deleting = false;
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

    case 330:
      deleting = true;
      break;

    case '/':
      searching = true;
      break;

    case 10:
      selectedGroupNo = currentGroupNo;
      break;
    }

    if (deleting) {
      if (deleteGroup(groups, currentGroupNo, window)) {
        currentGroupNo = 0;
        (*size)--;
      }
      deleting = false;
    }

    if (searching) {
      echo();
      curs_set(1);
      mvwprintw(window, 1, 1, "Search (/): ");
      wrefresh(window);
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

void printPriority(WINDOW *window, int currentItemNo) {
  wattron(window, COLOR_PAIR(4));
  wclear(window);
  box(window, 0, 0);
  printTitle(window, "Priority");
  wattroff(window, COLOR_PAIR(4));
  wmove(window, 1, 1);
  for (int i = 0; i < 3; i++) {
    if (i == currentItemNo) {
      wattron(window, A_STANDOUT);
      wprintw(window, "%s", priorityToStr(i));
      wattroff(window, A_STANDOUT);
    } else {
      wprintw(window, "%s", priorityToStr(i));
    }
    wmove(window, i + 2, 1);
  }
  wrefresh(window);
}

Priority selectPriority(WINDOW *window) {
  Priority priority = Normal;
  int currentItemNo = 1, key;
  bool selecting = true;
  while (selecting) {
    printPriority(window, currentItemNo);
    key = getch();
    switch (key) {
    case KEY_UP:
      if (currentItemNo != 0)
        currentItemNo--;
      break;
    case KEY_DOWN:
      if (currentItemNo != 2)
        currentItemNo++;
      break;
    case 10:
      selecting = false;
    }
  }
  return (Priority)currentItemNo;
}

struct tm createDeadline(WINDOW *window) {
  struct tm newTime;
  curs_set(1);
  wmove(window, 1, 1);
  wprintw(window, "(DD-MM-YY HH:MM): ");
  int day, month, year, hr, min;
  wscanw(window, "%d-%d-%d %d:%d", &day, &month, &year, &hr, &min);
  newTime.tm_mday = day;
  newTime.tm_mon = month - 1;
  newTime.tm_year = 100 + year;
  newTime.tm_hour = hr;
  newTime.tm_min = min;
  newTime.tm_sec = 0;
  mktime(&newTime);
  curs_set(0);
  return newTime;
}

Task *createTask(WINDOW *menuWindow, WINDOW *detailWindow, Group **groups,
                 uint *groupListSize) {
  echo();
  Task *newTask = (Task *)malloc(sizeof(Task));
  time_t currRawTime = time(NULL);
  memcpy(&newTask->deadline, localtime(&currRawTime), sizeof(struct tm));
  newTask->priority = Normal;
  Group *newGroup = NULL;
  wattron(menuWindow, COLOR_PAIR(4));
  wattron(detailWindow, COLOR_PAIR(4));
  wclear(menuWindow);
  wclear(detailWindow);
  box(menuWindow, 0, 0);
  box(detailWindow, 0, 0);
  printTitle(menuWindow, "Add Task");
  wattroff(menuWindow, COLOR_PAIR(4));
  wattroff(detailWindow, COLOR_PAIR(4));

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
      newTask->deadline = createDeadline(detailWindow);
    } else if (selectedItemNo == 4) {
      newTask->priority = selectPriority(detailWindow);
    } else if (selectedItemNo == 6) {
      wattron(detailWindow, COLOR_PAIR(1));
      wclear(detailWindow);
      box(detailWindow, 0, 0);
      printTitle(detailWindow, "Cancel?");
      mvwprintw(detailWindow, 1, 1, "Are you sure? (y/n)");
      wattroff(detailWindow, COLOR_PAIR(1));
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

void editTask(Task **head, uint taskNo, int detailNo, Group **groups,
              uint *groupSize, WINDOW *window) {
  wattron(window, COLOR_PAIR(4));
  clearWindow(window);
  printTitle(window, "Edit Task");
  wattroff(window, COLOR_PAIR(4));
  echo();
  Task *task = getTask(*head, taskNo);

  switch (detailNo) {
  case 0:
    curs_set(1);
    wmove(window, 1, 1);
    wgetnstr(window, task->desc, sizeof(task->desc));
    curs_set(0);
    break;

  case 1:
    strncpy(task->group, selectGroup(groups, window, groupSize)->name,
            sizeof(task->group));
    break;
  case 2:
    task->deadline = createDeadline(window);
    break;

  case 3:
    task->priority = selectPriority(window);
    break;
  }
  noecho();
}

void printSorting(WINDOW *window, int currentItemNo) {
  wclear(window);
  box(window, 0, 0);
  printTitle(window, "Sort By");
  wmove(window, 1, 1);
  for (int i = 0; i < 3; i++) {
    if (i == currentItemNo) {
      wattron(window, A_STANDOUT);
      wprintw(window, "%s", sortToStr(i));
      wattroff(window, A_STANDOUT);
    } else {
      wprintw(window, "%s", sortToStr(i));
    }
    wmove(window, i + 2, 1);
  }
  wrefresh(window);
}

TaskComparator selectSortingMethod(WINDOW *window) {
  Sort sortMethod = Prior;
  int currentItemNo = 0, key;
  bool selecting = true;
  while (selecting) {
    printSorting(window, currentItemNo);
    key = getch();
    switch (key) {
    case KEY_UP:
      if (currentItemNo != 0)
        currentItemNo--;
      break;
    case KEY_DOWN:
      if (currentItemNo != 2)
        currentItemNo++;
      break;
    case 10:
      selecting = false;
    }
  }

  switch (currentItemNo) {
  case 0:
    return comparePriority;

  case 1:
    return compareName;

  case 2:
    return compareDeadline;
  };

  return comparePriority;
}

Task *splitList(Task *head) {
  Task *slow = head, *fast = head->nextTask;

  while (fast != NULL && fast->nextTask != NULL) {
    fast = fast->nextTask->nextTask;
    slow = slow->nextTask;
  }

  Task *temp = slow->nextTask;
  slow->nextTask = NULL;
  return temp;
}

Task *merge(Task *first, Task *second, TaskComparator cmp) {
  if (!first)
    return second;
  if (!second)
    return first;

  if (cmp(first, second) <= 0) {
    first->nextTask = merge(first->nextTask, second, cmp);
    return first;
  } else {
    second->nextTask = merge(first, second->nextTask, cmp);
    return second;
  }
}

Task *mergeSort(Task *head, TaskComparator cmp) {
  if (head == NULL || head->nextTask == NULL)
    return head;

  Task *second = splitList(head);

  head = mergeSort(head, cmp);
  second = mergeSort(second, cmp);
  return merge(head, second, cmp);
}

int selectDetail(WINDOW *window, Task *task, int active) {
  int currentDetailNo = 0;
  bool selecting = true;
  int keyPress;

  while (selecting) {
    printDetails(task, window, currentDetailNo, active);
    keyPress = getch();
    switch (keyPress) {
    case KEY_UP:
      if (currentDetailNo != 0)
        currentDetailNo--;
      break;
    case KEY_DOWN:
      if (currentDetailNo != 4)
        currentDetailNo++;
      break;
    case 10:
      selecting = false;
    }
  }

  return currentDetailNo;
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

int delete(Task **head, int pos, WINDOW *window) {
  if (window != NULL) {
    wattron(window, COLOR_PAIR(1));
    wclear(window);
    box(window, 0, 0);
    printTitle(window, "Delete");
    mvwprintw(window, 1, 1, "Are you sure? (y/n)");
    wattroff(window, COLOR_PAIR(1));
    char choice = wgetch(window);
    if (choice != 'y')
      return 0;
  }
  if (*head == NULL) // Empty
    return 0;

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
  return 1;
}

void moveTask(Task **destHead, Task **sourceHead, uint taskNo) {
  Task *task;
  if (*sourceHead == NULL) // Empty
    return;

  else if ((*sourceHead)->nextTask == NULL) {
    task = *sourceHead;
    *sourceHead = NULL;
  }

  else if (taskNo == 0) { // Delete Beginning
    task = *sourceHead;
    *sourceHead = (*sourceHead)->nextTask;
  }

  else if (taskNo == -1) { // Delete End
    Task *temp = *sourceHead;
    while (temp->nextTask->nextTask != NULL)
      temp = temp->nextTask;
    task = temp->nextTask;
    temp->nextTask = NULL;
  }

  else if (taskNo > 0) { // Delete Middle
    Task *temp = *sourceHead;
    int i = 1;
    while (i < taskNo && temp->nextTask->nextTask != NULL) {
      temp = temp->nextTask;
      i++;
    }
    task = temp->nextTask;
    temp->nextTask = temp->nextTask->nextTask;
  }
  task->nextTask = NULL;
  insert(destHead, task);
}

struct tm *strptime(char *str, struct tm *time) {
  int day, month, year, hr, min;
  sscanf(str, "%d-%d-%d %d:%d", &day, &month, &year, &hr, &min);
  time->tm_mday = day;
  time->tm_mon = month - 1;
  time->tm_year = 100 + year;
  time->tm_hour = hr;
  time->tm_min = min;
  time->tm_sec = 0;
  mktime(time);
  return time;
}

void load(Task **pendingHead, Task **completeHead, Group **groupHead,
          uint *pSize, uint *cSize, uint *gSize) {
  FILE *pendingFile = fopen("pending.txt", "r");
  char buffer[sizeof(Task)];
  while (fgets(buffer, sizeof(buffer), pendingFile)) {
    Task *newTask = (Task *)malloc(sizeof(Task));
    char deadline[32];
    sscanf(buffer, "%[^;];%[^;];%[^;];%[^;];%i\n", newTask->title,
           newTask->desc, newTask->group, deadline, &newTask->priority);
    strptime(deadline, &newTask->deadline);
    newTask->nextTask = *pendingHead;
    *pendingHead = newTask;
    (*pSize)++;
  }
  fclose(pendingFile);
  FILE *completedFile = fopen("completed.txt", "r");
  while (fgets(buffer, sizeof(buffer), completedFile)) {
    Task *newTask = (Task *)malloc(sizeof(Task));
    char deadline[32];
    sscanf(buffer, "%[^;];%[^;];%[^;];%[^;];%i\n", newTask->title,
           newTask->desc, newTask->group, deadline, &newTask->priority);
    strptime(deadline, &newTask->deadline);
    newTask->nextTask = *completeHead;
    *completeHead = newTask;
    (*cSize)++;
  }
  fclose(completedFile);
  FILE *groupFile = fopen("groups.txt", "r");
  while (fgets(buffer, sizeof(buffer), groupFile)) {
    Group *newGroup = (Group *)malloc(sizeof(Group));
    sscanf(buffer, "%[^\n]\n", newGroup->name);
    newGroup->nextGroup = *groupHead;
    *groupHead = newGroup;
    (*gSize)++;
  }
  fclose(groupFile);
}

void quit(Task **pendingHead, Task **completeHead, Group **groupHead) {
  FILE *pendingFile = fopen("pending.txt", "w");
  Task *temp = *pendingHead;
  while (temp != NULL) {
    char deadline[32];
    strftime(deadline, sizeof(deadline), "%d-%m-%y %H:%M", &temp->deadline);
    fprintf(pendingFile, "%s;%s;%s;%s;%i\n", temp->title, temp->desc,
            temp->group, deadline, temp->priority);
    Task *toFree = temp;
    temp = temp->nextTask;
    free(toFree);
  }
  fclose(pendingFile);
  FILE *completedFile = fopen("completed.txt", "w");
  temp = *completeHead;
  while (temp != NULL) {
    char deadline[32];
    strftime(deadline, sizeof(deadline), "%d-%m-%y %H:%M", &temp->deadline);
    fprintf(completedFile, "%s;%s;%s;%s;%i\n", temp->title, temp->desc,
            temp->group, deadline, temp->priority);
    Task *toFree = temp;
    temp = temp->nextTask;
    free(toFree);
  }
  fclose(completedFile);
  FILE *groupFile = fopen("groups.txt", "w");
  Group *gTemp = *groupHead;
  while (gTemp != NULL) {
    fprintf(groupFile, "%s\n", gTemp->name);
    Group *toFree = gTemp;
    gTemp = gTemp->nextGroup;
    free(toFree);
  }
  fclose(groupFile);
}

void displayHelp(WINDOW *window) {
  wattron(window, COLOR_PAIR(6));
  clearWindow(window);
  printTitle(window, "Help");
  int y = 0;
  mvwprintw(window, ++y, 1, "Task Organiser & Manager");
  y++;
  mvwprintw(window, ++y, 1, "n -> Create New Task");
  mvwprintw(window, ++y, 1, "delete -> Delete");
  mvwprintw(window, ++y, 1, "g -> Select Group");
  mvwprintw(window, ++y, 1, "F2 -> Rename Task");
  mvwprintw(window, ++y, 1, "p -> Select Priority");
  mvwprintw(window, ++y, 1, "/ -> Search");
  mvwprintw(window, ++y, 1, "F5 -> Sort Tasks");
  mvwprintw(window, ++y, 1, "? -> Display This");
  mvwprintw(window, ++y, 1, "ENTER -> Select");
  mvwprintw(window, ++y, 1, "q -> Save and Quit");
  y++;
  mvwprintw(window, ++y, 1, "Made By Shiva Prakash.");
  mvwprintw(window, ++y, 1,
            "More info at github.com/ShivaPrakash-006/TaskManagement");
  wrefresh(window);
  getch();
  wattroff(window, COLOR_PAIR(6));
}

void setupColors() {
  init_pair(1, COLOR_RED, 0);
  init_pair(2, COLOR_BLUE, 0);
  init_pair(3, COLOR_CYAN, 0);
  init_pair(4, COLOR_GREEN, 0);
  init_pair(5, COLOR_YELLOW, 0);
  init_pair(6, COLOR_MAGENTA, 0);
  init_pair(7, COLOR_WHITE, 0);
  init_pair(8, COLOR_BLACK, 0);
}

int main() {
  Task *pendingTaskHead = NULL, *completedTaskHead = NULL;
  Group *groupHead = NULL, *newGroup;
  Task **currentTaskHead = &pendingTaskHead,
       **inactiveTaskHead = &completedTaskHead;
  Task *currentTask, *newTask;
  int active = 0; // 0 for pending, 1 for completed

  int choice;
  bool deleteMode = false, run = true, searching = false;
  char title[32], desc[128], group[32] = "", filterStr[32] = "";
  int sortedPriority = -1, detailNo = -1;
  uint pSize = 0, cSize = 0, *aSize = &pSize, *iSize = &cSize,
       currentTaskNo = 0, groupListSize = 0;
  load(&pendingTaskHead, &completedTaskHead, &groupHead, &pSize, &cSize,
       &groupListSize);
  TaskComparator cmpFunction = comparePriority;
  pendingTaskHead = mergeSort(pendingTaskHead, cmpFunction);
  completedTaskHead = mergeSort(completedTaskHead, cmpFunction);
  currentTask = getTask(*currentTaskHead, currentTaskNo);

  initscr();
  keypad(stdscr, true);
  cbreak();
  noecho();
  start_color();
  curs_set(0);
  WINDOW *taskWindow = newwin(LINES, COLS / 2, 0, 0);
  WINDOW *detailWindow = newwin(LINES, COLS / 2, 0, COLS / 2);
  wclear(taskWindow);
  wclear(detailWindow);
  box(taskWindow, 0, 0);
  box(detailWindow, 0, 0);
  refresh();
  wrefresh(taskWindow);
  wrefresh(detailWindow);
  start_color();
  setupColors();
  while (run) {
    if (*aSize != 0)
      printDetails(currentTask, detailWindow, -1, active);
    printTasks(*currentTaskHead, taskWindow, currentTaskNo, group, filterStr,
               sortedPriority, active);
    choice = getch();
    switch (choice) {
    case KEY_UP:
      for (int i = 0; i < *aSize; i++) {
        if (currentTaskNo == 0)
          currentTaskNo = (*aSize) - 1;
        else
          currentTaskNo--;
        if (taskInGroup(*currentTaskHead, group, currentTaskNo) &&
            filterTask(*currentTaskHead, filterStr, currentTaskNo) &&
            taskPriority(*currentTaskHead, sortedPriority, currentTaskNo))
          break;
      }
      if (*aSize != 0) {
        currentTask = getTask(*currentTaskHead, currentTaskNo);
        printDetails(currentTask, detailWindow, -1, active);
      }
      break;
    case KEY_DOWN:
      for (int i = 0; i < *aSize; i++) {
        if (currentTaskNo == (*aSize) - 1)
          currentTaskNo = 0;
        else
          currentTaskNo++;
        if (taskInGroup(*currentTaskHead, group, currentTaskNo) &&
            filterTask(*currentTaskHead, filterStr, currentTaskNo) &&
            taskPriority(*currentTaskHead, sortedPriority, currentTaskNo))
          break;
      }
      if (*aSize != 0) {
        currentTask = getTask(*currentTaskHead, currentTaskNo);
        printDetails(currentTask, detailWindow, -1, active);
      }
      break;

    case KEY_RIGHT:
    case KEY_LEFT:
      if (active == 0) {
        currentTaskHead = &completedTaskHead;
        inactiveTaskHead = &pendingTaskHead;
        active = 1;
        aSize = &cSize;
        iSize = &pSize;
      } else {
        currentTaskHead = &pendingTaskHead;
        inactiveTaskHead = &completedTaskHead;
        active = 0;
        aSize = &pSize;
        iSize = &cSize;
      }
      currentTaskNo = 0;
      break;

    case 'n':
      newTask =
          createTask(taskWindow, detailWindow, &groupHead, &groupListSize);
      if (newTask) {
        insert(currentTaskHead, newTask);
        *currentTaskHead = mergeSort(*currentTaskHead, cmpFunction);
      }
      (*aSize)++;
      break;
    case KEY_F(2):
      wattron(detailWindow, COLOR_PAIR(4));
      clearWindow(detailWindow);
      printTitle(detailWindow, "Rename");
      wattroff(detailWindow, COLOR_PAIR(4));
      echo();
      curs_set(1);
      wmove(detailWindow, 1, 1);
      wgetnstr(detailWindow, currentTask->title, sizeof(currentTask->title));
      curs_set(0);
      noecho();
      break;
    case 'g':
      newGroup = selectGroup(&groupHead, detailWindow, &groupListSize);
      if (aSize != 0 && newGroup != NULL) {
        strcpy(group, newGroup->name);
      } else {
        strcpy(group, "");
      }
      break;
    case 'p':
      sortedPriority = selectPriority(detailWindow);
      break;

    case KEY_F(5):
      cmpFunction = selectSortingMethod(detailWindow);
      *currentTaskHead = mergeSort(*currentTaskHead, cmpFunction);
      break;

    case 10: // ENTER
      detailNo = selectDetail(detailWindow,
                              getTask(*currentTaskHead, currentTaskNo), active);
      break;
    case 330: // DELETE
      if (aSize != 0)
        deleteMode = true;
      break;

    case '/':
      searching = true;
      break;

    case '?':
      displayHelp(detailWindow);
      break;

    case 'q':
      quit(&pendingTaskHead, &completedTaskHead, &groupHead);
      run = false;
      break;
    }

    if (detailNo != -1 && detailNo != 4) {
      editTask(currentTaskHead, currentTaskNo, detailNo, &groupHead,
               &groupListSize, detailWindow);
      detailNo = -1;
    } else if (detailNo == 4) {
      moveTask(inactiveTaskHead, currentTaskHead, currentTaskNo);
      if (currentTaskNo == (*aSize) - 1)
        currentTaskNo--;
      (*aSize)--;
      (*iSize)++;
      *currentTaskHead = mergeSort(*currentTaskHead, cmpFunction);
      *inactiveTaskHead = mergeSort(*inactiveTaskHead, cmpFunction);
      detailNo = -1;
    }

    if (deleteMode) {
      if (delete (currentTaskHead, currentTaskNo, taskWindow)) {
        if (currentTaskNo == (*aSize) - 1)
          currentTaskNo--;
        (*aSize)--;
      }
      *currentTaskHead = mergeSort(*currentTaskHead, cmpFunction);
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
    if (*aSize != 0)
      currentTask = getTask(*currentTaskHead, currentTaskNo);
  }
  endwin();

  return 0;
}
