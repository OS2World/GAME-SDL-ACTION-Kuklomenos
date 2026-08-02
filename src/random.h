#ifndef INC_RANDOM_H
#define INC_RANDOM_H

#include <vector>

float ranf();
float ranf(float m);
int rani(int m);
float gaussian();

template<class T>
std::vector<T> randomSort(std::vector<T> vec)
{
    std::vector<T> randomized;

    for (int i = vec.size(); i != 0; i--)
    {
	int j = 0;
	for ( typename std::vector<T>::iterator it = vec.begin();
		it != vec.end();
		j++, it++)
	    if (rani(i-j) == 0)
	    {
		randomized.push_back(*it);
		vec.erase(it);
		break;
	    }
    }

    return randomized;
}

#endif /* INC_RANDOM_H */
