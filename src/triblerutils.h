// Copyright (c) 2015-2015 The Decent developers

#ifndef TRIBLERUTILS_H
#define TRIBLERUTILS_H

#include <curl/curl.h>
#include "json/json_spirit.h"
#include "json/json_spirit_reader.h"
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_stream_reader.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_utils.h"
#include "main.h"

using namespace std;

class TriblerUtils
{
public:
    TriblerUtils() {basePath = "http://localhost:8888/gui/";}
    string Escape(string original);
    string Unescape(string escaped);
    string Request(string subpath);
    string Request(string controller, map<string, string>);
    bool GetToken(string &token);
    bool GetJsonField(string json, string fieldName, json_spirit::Value &value);
    bool IsConnected();
    bool UploadNewContent(string filePath, string &magnetHash);
    bool StartDownloadContent(string magnetHash);
    bool GetDownloadedContent(string magnetHash, string &filePath);
    int GetDownloadingPercent(string magnetHash);
private:
    string RequestTribler(string subPath);
    CURL* curl;
    string basePath;
    CURLcode res;
};

class TriblerData
{
public:
    mutable CCriticalSection cs;
    int DownloadingPercent(uint160 magnetHash);
    void UpdateAllData();
    bool IsConnected();
private:
    map<uint160, int> actualData;
    void UpdateOne(const uint160 &magnetHash, int percent);
    bool triblerConnectionStatus;
};

extern TriblerData triblerData;
void TriblerUpdater();

#endif // TRIBLERUTILS_H
