#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <curl/easy.h>

#include <string>

#include "conffile.h"
#include "net.h"

const char* HSS_SERVER = "http://mbays.freeshell.org/kuklomenos/highScore.cgi";

// Following adapted from getinmemory.c from the libcurl distribution.

struct MemoryStruct {
  char *memory;
  size_t size;
};

static void *myrealloc(void *ptr, size_t size);

static void *myrealloc(void *ptr, size_t size)
{
  /* There might be a realloc() out there that doesn't like reallocing
     NULL pointers, so we take care of it here */
  if(ptr)
    return realloc(ptr, size);
  else
    return malloc(size);
}

static size_t
WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)data;

  mem->memory = (char*)myrealloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory) {
    memcpy(&(mem->memory[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
  }
  return realsize;
}

hssResponseCode doHSSCommand(std::string command, std::string& response)
{
    char url[128];
    snprintf(url, 128, "%s?%s", HSS_SERVER, command.c_str());

    struct MemoryStruct chunk;
    chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
    chunk.size = 0;    /* no data at this point */

    CURL *curl_handle;

    curl_global_init(CURL_GLOBAL_ALL);

    /* init the curl session */
    curl_handle = curl_easy_init();

    /* specify URL to get */
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);

    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

    /* some servers don't like requests that are made without a user-agent
       field, so we provide one */
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    /* get it! */
    curl_easy_perform(curl_handle);

    response = chunk.memory;

    hssResponseCode ret;

    if (response.substr(0, 7) == "Error :" )
	ret = HSS_ERROR;
    else if (response.substr(0, 7) == "Result:" )
	ret = HSS_SUCCESS;
    else
	ret = HSS_FAIL;

    response = response.substr(8);
    response.erase(response.find_first_of('\n'));

    if (chunk.memory)
	free(chunk.memory);

    /* cleanup curl stuff */
    curl_easy_cleanup(curl_handle);

    curl_global_cleanup();

    return ret;
}
