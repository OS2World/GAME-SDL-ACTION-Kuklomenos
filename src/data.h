#ifdef __APPLE__
# include "Mac_data.h"
#else

#ifndef INC_DATA_H
#define INC_DATA_H

#include <string>
using namespace std;

extern char fontSmall[3328];
extern char fontBig[10240];

bool initFont();

const string findDataPath(string relPath);

#endif /* INC_DATA_H */
#endif /* __APPLE__ */
