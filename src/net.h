#ifndef INC_NET_H
#define INC_NET_H

#include <string>

enum hssResponseCode 
{
    HSS_FAIL, // couldn't talk to server
    HSS_ERROR, // server replied with an error
    HSS_SUCCESS // server replied with success
};

hssResponseCode doHSSCommand(std::string command, std::string& response);

#endif /* INC_NET_H */
