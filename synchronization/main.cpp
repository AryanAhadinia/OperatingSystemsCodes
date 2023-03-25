// Operating Systems :: HW6
// Aryan Ahadinia, 98103878
// Computer Engineering Department
// Sharif University of Technology

#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

using namespace std;

int graph[256][256] = {0};
pthread_mutex_t mutexes[256][256];

long total_em = 0;
pthread_mutex_t total_em_mutex;

typedef struct
{
    int p;
    int path_id;
    int car_id;
    vector<char> path;
} Car;

time_t now()
{
    struct timeval time_now;
    gettimeofday(&time_now, nullptr);
    return (time_now.tv_sec * 1000) + (time_now.tv_usec / 1000);
}

int emission(int p, int h)
{
    int sum = 0;
    for (int i = 1; i <= 10000000; i++)
    {
        sum += i / (1000000 * p * h);
    }
    return sum;
}

void *car_worker(void *args)
{
    Car *car = (Car *)args;
    int p = car->p;
    int path_id = car->path_id;
    int car_id = car->car_id;
    vector<char> path = car->path;

    ofstream file(to_string(path_id) + "-" + to_string(car_id) + ".txt");

    for (int i = 0; i < path.size() - 1; i++)
    {
        char org = path[i];
        char dst = path[i + 1];

        pthread_mutex_lock(&mutexes[org][dst]);
        time_t start = now();
        int em = emission(p, graph[org][dst]);
        pthread_mutex_lock(&total_em_mutex);
        total_em += em;
        int em_now = total_em;
        pthread_mutex_unlock(&total_em_mutex);
        time_t exit = now();
        pthread_mutex_unlock(&mutexes[org][dst]);

        file << org << ", ";
        file << start << ", ";
        file << dst << ", ";
        file << exit << ", ";
        file << em << ", ";
        file << em_now << endl;
    }

    file.close();
    return NULL;
}

int main(int argc, char *argv[])
{
    ifstream in_file(argv[1]);

    char org, dst, tmp;
    int h;

    while (true)
    {
        in_file >> org;
        if (org == '#')
            break;
        in_file >> tmp >> dst >> tmp >> h;
        graph[org][dst] = h;
        if (pthread_mutex_init(&mutexes[org][dst], NULL) != 0)
            cout << "Failed to create mutex for " << org << "-" << dst << "." << endl;
    }

    char point;
    string buffer;

    vector<vector<char> > paths = vector<vector<char> >();
    vector<int> cars_counts = vector<int>();

    while (true)
    {
        in_file >> point;
        if (point == EOF)
            break;
        vector<char> path;
        int cars_count;
        path.push_back(point);
        while (true)
        {
            in_file >> buffer;
            if (buffer[0] != '-')
            {
                cars_count = stoi(buffer);
                if (in_file.eof())
                    goto reading_end;
                break;
            }
            in_file >> point;
            path.push_back(point);
        }
        paths.push_back(path);
        cars_counts.push_back(cars_count);
    }
reading_end:

    in_file.close();

    cout << "DS Created!" << endl;
    for (int i = 0; i < 256; i++)
        for (int j = 0; j < 256; j++)
            if (graph[i][j] != 0)
                cout << (char)i << " - " << (char)j << " - " << graph[i][j] << endl;
    cout << "#" << endl;
    for (int i = 0; i < paths.size(); i++)
    {
        for (int j = 0; j < paths[i].size(); j++)
        {
            cout << paths[i][j];
            if (j != paths[i].size() - 1)
                cout << " - ";
        }
        cout << endl;
        cout << cars_counts[i] << endl;
    }

    vector<Car> cars = vector<Car>();
    srand (time(NULL));
    for (int i = 0; i < paths.size(); i++)
    {
        for (int j = 0; j < cars_counts[i]; j++)
        {
            Car car;
            car.p = rand() % 10 + 1;
            car.path_id = i;
            car.car_id = j;
            car.path = paths[i];
            cars.push_back(car);
        }
    }

    pthread_t threads[cars.size()];
    for (int i = 0; i < cars.size(); i++)
        pthread_create(&threads[i], NULL, car_worker, &cars[i]);

    for (int i = 0; i < cars.size(); i++)
        pthread_join(threads[i], NULL);

    return 0;
}