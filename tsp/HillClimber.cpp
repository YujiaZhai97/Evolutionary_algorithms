

#include <iostream>

#include<fstream>
#include<sstream>
#include<deque>
#include<string>
#include <cstdlib>
#include <ctime>
#include<set>
#include<time.h>
#include<list>
#include<vector>
#include<random>
#include<iterator>
#define CLOCK_PER_SECOND ((clock_t)1000)

using namespace std;

struct Coordinate {
    float x = 0;
    float y = 0;
};

struct Distance {
    float sum = 0;
    deque<Coordinate> route;
};

template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}
vector<int> randomGenerator() {
    vector<int> foo;
    for (int i = 0; i < 1000; i++) {
        foo.push_back(i);
    }
    vector<int> num;
    set<int> used;
    while (num.size() < 1000) {
        int r = *select_randomly(foo.begin(), foo.end());
        if (used.count(r) == 0) {
            num.push_back(r);
            used.insert(r);
        }
    }
    return num;
};

vector<int> randomGenerator2points() {
    vector<int> foo;
    for (int i = 0; i < 1000; i++) {
        foo.push_back(i);
    }
    vector<int> num;
    set<int> used;
    while (num.size() < 2) {
        int r = *select_randomly(foo.begin(), foo.end());
        if (used.count(r) == 0) {
            num.push_back(r);
            used.insert(r);
        }
    }
    return num;
};


    Distance randomSort(deque<Coordinate> allPoints) {
        Distance result;
        unsigned seed = time(0);
        srand(seed);
        vector<int> nums(1000);
        nums = randomGenerator();
        for (int i : nums) {
            result.route.push_back(allPoints[i]);
        }
        Coordinate first = result.route[0];
        Coordinate pre;
        Coordinate now;
        pre = first;
        int count = 1;
        while (count < 1000) {
            now = result.route[count];
            result.sum += sqrt(pow((pre.x - now.x), 2) + pow((pre.y - now.y), 2));
            pre = now;
            count++;
        }
        result.sum += sqrt(pow((first.x - now.x), 2) + pow((first.y - now.y), 2));
        return result;
    };

    Distance calculateResult(Distance nowresult) {
        nowresult.sum = 0;
        deque<Coordinate> route;
        route = nowresult.route;
        
        Coordinate first = route[0];
        Coordinate pre;
        Coordinate now;
        pre = first;
        int count = 1;
        while (count < 1000) {
            now = nowresult.route[count];
            nowresult.sum += sqrt(pow((pre.x - now.x), 2) + pow((pre.y - now.y), 2));
            pre = now;
            count++;
        }
        nowresult.sum += sqrt(pow((first.x - now.x), 2) + pow((first.y - now.y), 2));
        return nowresult;
    }

    void Process(int round, deque<Coordinate> allPoints) {
        ofstream output;//daochu text
        int count = 0;
        clock_t start;
        clock_t now;
        float period;
        Distance initialresult;
        Distance nowresult;
        Distance sresult;
        Distance lresult;
        int r1;
        int r2;
        Coordinate temp;
        sresult.sum = numeric_limits<float>::max();
        lresult.sum = numeric_limits<float>::min();

        cout << "start" << round << "round" << endl;
        list<Distance> allresultS;
        list<Distance> allresultL;
        nowresult = randomSort(allPoints);
        sresult.route = nowresult.route;
        lresult.route = nowresult.route;
        while (count < 20) {
            period = 0;
            //start = clock();
            //now = clock();
            cout << "count " << count << endl;
            // nowresult = randomSort(allPoints);
            while (period <= 5000) {
                unsigned seed = time(0);
                srand(seed);
                //r1 = rand() % 1000;
                //r2 = rand() % 1000;
                //if (r2 == r1) {
                 //   r2 = rand() % 1000;
                //}
                vector<int> swap(2);
                swap = randomGenerator2points();
                r1 = swap[0];
                r2 = swap[1];
                
                temp = sresult.route[r1];
                sresult.route[r1]= sresult.route[r2];
                sresult.route[r2] = temp;

                nowresult = calculateResult(sresult);
                allresultS.push_back(nowresult);

                if (nowresult.sum <sresult.sum) {
                   sresult = nowresult;
                }

                temp = lresult.route[r1];
                lresult.route[r1] = sresult.route[r2];
                lresult.route[r2] = temp;
                nowresult = calculateResult(lresult);
                allresultL.push_back(nowresult);
                if (nowresult.sum > lresult.sum) {
                    lresult = nowresult;
                }
                //now = clock();
                period++;

                
            }
            cout << lresult.sum << " " << sresult.sum << endl;
            output.open("shortest_output_" + to_string(round) + ".txt", fstream::app);//fangzhishanchuyuanyoude
            for (Distance i : allresultS) {
                output << i.sum << endl;
            }
            output.close();
            count++;
            allresultS.clear();

            output.open("longest_output_" + to_string(round) + ".txt", fstream::app);//fangzhishanchuyuanyoude
            for (Distance i : allresultL) {
                output << i.sum << endl;
            }
            output.close();
            allresultL.clear();
        }
        output.open("paths_" + to_string(round) + ".txt", fstream::app);
        for (Coordinate i : sresult.route) {
            output << i.x << "," << i.y << endl;
        }
        output.close();
        output.open("pathl_" + to_string(round) + ".txt", fstream::app);
        for (Coordinate i : lresult.route) {
            output << i.x << "," << i.y << endl;
        }
        output.close();
    }

    int main()
    {
        ifstream input("D:\\CppRepository\\HillClimber\\HillClimber\\tsp.txt");
        string str;
        deque<Coordinate> allPoints;
        int i = 0;
        while (getline(input, str)) {
            istringstream in(str);
            Coordinate temp;
            float x, y;
            in >> x >> y;
            temp.x = x;
            temp.y = y;
            allPoints.push_back(temp);
            i++;
        }

        cout << "file read" << endl;
        for (int i = 1; i < 5; i++) {
            Process(i, allPoints);
        }
        return 0;

    }










