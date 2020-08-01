#include <iostream>
#include <map>
#include <unordered_map>

using namespace std;

// Перечислимый тип для статуса задачи
enum class TaskStatus {
    NEW,          // новая
    IN_PROGRESS,  // в разработке
    TESTING,      // на тестировании
    DONE          // завершена
};

// Объявляем тип-синоним для map<TaskStatus, int>,
// позволяющего хранить количество задач каждого статуса
using TasksInfo = map<TaskStatus, int>;

class TeamTasks {
public:
    // Получить статистику по статусам задач конкретного разработчика
    const TasksInfo &GetPersonTasksInfo(const string &person) const {
        return tasks.at(person);
    }

    // Добавить новую задачу (в статусе NEW) для конкретного разработчитка
    void AddNewTask(const string &person) {
        tasks[person][TaskStatus::NEW]++;
    }

    // Обновить статусы по данному количеству задач конкретного разработчика,
    // подробности см. ниже
    tuple<TasksInfo, TasksInfo> PerformPersonTasks(const string &person, int task_count) {
        TasksInfo updated;
        for (TaskStatus status : {TaskStatus::NEW, TaskStatus::IN_PROGRESS, TaskStatus::TESTING}) {
            for (int i = 0; i < tasks[person][status]; i++) {
                updated[status]++;
                if (--task_count == 0) {
                    goto EndOfBoth;
                }
            }
        }
        EndOfBoth:

        TasksInfo result_updated, result_not_updated;

        for (TaskStatus status : {TaskStatus::NEW, TaskStatus::IN_PROGRESS, TaskStatus::TESTING}) {
            int amount_updated = updated[status];
            result_updated[NextStatus(status)] = amount_updated;
            result_not_updated[status] = (tasks[person][status] - result_updated[status]) - amount_updated;
            tasks[person][status] -= amount_updated;
            tasks[person][NextStatus(status)] += amount_updated;
        }

        // clear from 0
        ClearFrom0(result_updated);
        ClearFrom0(result_updated);

        return make_tuple(result_updated, result_not_updated);
    }

private:
    void ClearFrom0(TasksInfo& tasks) {
        for (auto it = tasks.begin(); it != tasks.end();) {
            if (it->second == 0) {
                it = tasks.erase(it);
            } else {
                ++it;
            }
        }
    }

    TaskStatus NextStatus(TaskStatus status) {
        return TaskStatus(static_cast<int>(status) + 1);
    }

    unordered_map<string, TasksInfo> tasks;
    TasksInfo EMPTY;
};

// Принимаем словарь по значению, чтобы иметь возможность
// обращаться к отсутствующим ключам с помощью [] и получать 0,
// не меняя при этом исходный словарь
void PrintTasksInfo(TasksInfo tasks_info) {
    cout << tasks_info[TaskStatus::NEW] << " new tasks" <<
         ", " << tasks_info[TaskStatus::IN_PROGRESS] << " tasks in progress" <<
         ", " << tasks_info[TaskStatus::TESTING] << " tasks are being tested" <<
         ", " << tasks_info[TaskStatus::DONE] << " tasks are done" << endl;
}

int main() {
    TeamTasks tasks;
    tasks.AddNewTask("Ilia");
    for (int i = 0; i < 3; ++i) {
        tasks.AddNewTask("Ivan");
    }
    cout << "Ilia's tasks: ";
    PrintTasksInfo(tasks.GetPersonTasksInfo("Ilia"));
    cout << "Ivan's tasks: ";
    PrintTasksInfo(tasks.GetPersonTasksInfo("Ivan"));

    TasksInfo updated_tasks, untouched_tasks;

    tie(updated_tasks, untouched_tasks) =
            tasks.PerformPersonTasks("Ivan", 2);
    cout << "Updated Ivan's tasks: ";
    PrintTasksInfo(updated_tasks);
    cout << "Untouched Ivan's tasks: ";
    PrintTasksInfo(untouched_tasks);

    tie(updated_tasks, untouched_tasks) =
            tasks.PerformPersonTasks("Ivan", 2);
    cout << "Updated Ivan's tasks: ";
    PrintTasksInfo(updated_tasks);
    cout << "Untouched Ivan's tasks: ";
    PrintTasksInfo(untouched_tasks);

    return 0;
}
