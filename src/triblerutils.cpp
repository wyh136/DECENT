// Copyright (c) 2015-2015 The Decent developers

#include "triblerutils.h"

using namespace std;

TriblerData triblerData;

size_t write_to_string(void *ptr, size_t size, size_t count, void *stream)
{
  ((string*)stream)->append((char*)ptr, 0, size*count);
  return size*count;
}

string TriblerUtils::Request(string subPath)
{
    string requestPath = basePath + subPath;

    string response;
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, requestPath.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return response;
}

string TriblerUtils::Request(string controller, map<string, string> params)
{
    string urlParams = "";
    for (map<string, string>::iterator it = params.begin(); it != params.end(); ++it)
    {
        urlParams += it == params.begin() ? "?" : "&";
        urlParams += it->first + "=" + it->second;
    }
    return Request(controller + urlParams);
}


bool TriblerUtils::GetToken(string &token)
{
    string response = Request("token-json");
    if (response.length() == 0)
        return false;

    json_spirit::Value value;
    bool success = this->GetJsonField(response, "token", value);
    if (success)
        token = value.get_str();

    return success;
}


bool TriblerUtils::GetJsonField(string json, string fieldName, json_spirit::Value & value)
{
    json_spirit::Value val;
    bool success = json_spirit::read_string(json, val);
    if (success)
    {
        json_spirit::Object jsonObject = val.get_obj();
        for(vector<json_spirit::Pair>::iterator it = jsonObject.begin(); it != jsonObject.end(); ++it)
        {
            json_spirit::Pair p = *it;
            if (p.name_ == fieldName)
            {
                value = p.value_;
                return true;
            }
        }
    }
    return false;
}

bool TriblerUtils::IsConnected()
{
    string token;
    bool success = this->GetToken(token);
    if (!success)
        return false;

    map<string, string> params;
    params["token"] = token;
    params["action"] = "isconnected";
    string resultJson = this->Request("index", params);
    json_spirit::Value isConnected;
    return this->GetJsonField(resultJson, "isConnected", isConnected) && isConnected.get_bool();
}

string TriblerUtils::Escape(string original)
{
    string value = curl_easy_escape(curl, original.c_str(), original.length());
    return value;
}

string TriblerUtils::Unescape(string escaped)
{
    int length;
    string value = curl_easy_unescape(curl, escaped.c_str(), escaped.length(), &length);
    return value;
}

bool TriblerUtils::StartDownloadContent(string magnetHash)
{
    string token;
    bool fSuccess = GetToken(token);
    if (!fSuccess)
        return error("StartDownloadContent() : tribler not available");

    map<string, string> params;
    params["token"] = token;
    params["action"] = "add-url";
    params["s"] = magnetHash;

    string jsonResult = Request("index", params);
    json_spirit::Value value;
    fSuccess = GetJsonField(jsonResult, "success", value) && value.get_bool();
    return fSuccess;
}

bool TriblerUtils::UploadNewContent(string filePath, string &magnetHash)
{
    string token;
    bool fSuccess = GetToken(token);
    if (!fSuccess)
        return error("UploadNewContent() : tribler not available");

    map<string, string> params;
    params["token"] = token;
    params["action"] = "upload-file";
    params["path"] = Escape(filePath);
    params["key"] = "";
    string resultJson = Request("index", params);

    json_spirit::Value value;
    fSuccess = GetJsonField(resultJson, "success", value) && value.get_bool();
    if (!fSuccess)
        return error("UploadNewContent() : tribler can't add file");

    fSuccess = GetJsonField(resultJson, "magnet", value);
    magnetHash = value.get_str();
    return fSuccess;
}

bool TriblerUtils::GetDownloadedContent(string magnetHash, string &filePath)
{
    string token;
    bool fSuccess = GetToken(token);
    if (!fSuccess)
        return error("GetDownloadedContent() : tribler not available");

    map<string, string> params;
    params["action"] = "retrieve-file";
    params["token"] = token;
    params["key"] = "";
    params["magnet"] = magnetHash;

    string jsonResult = Request("index", params);
    json_spirit::Value value;
    fSuccess = GetJsonField(jsonResult, "success", value) && value.get_bool();
    if (!fSuccess)
        return error("GetDownloadedContent() : can't get filePath from tribler");

    fSuccess = GetJsonField(jsonResult, "filePath", value);
    filePath = Unescape(value.get_str());
    return fSuccess;
}

int TriblerUtils::GetDownloadingPercent(string magnetHash)
{
    string token;
    bool success = GetToken(token);
    if (!success)
    {
        error("GetDownloadingPercent() : tribler not available");
        return 0;
    }

    map<string, string> params;
    params["action"] = "getprogress";
    params["magnet"] = magnetHash;
    params["token"] = token;
    string jsonResult = Request("index", params);

    json_spirit::Value value;
    success = GetJsonField(jsonResult, "success", value) && value.get_bool();
    if (!success)
    {
        error("GetDownloadingPercent() : tribler connection failed");
        return 0;
    }

    success = GetJsonField(jsonResult, "progress", value);
    return value.get_real();
}

int TriblerData::DownloadingPercent(uint160 magnetHash)
{
    int percent = 0;
    {
        LOCK(cs);
        if (actualData.count(magnetHash))
            percent = actualData[magnetHash];
        else
            actualData[magnetHash] = 0;
    }
    return percent;
}

void TriblerData::UpdateAllData()
{
    map<uint160, int> oldData;
    {
        LOCK(triblerData.cs);
        oldData = actualData;
    }

    TriblerUtils tribler;
    BOOST_FOREACH(const PAIRTYPE(uint160, int) &item, oldData)
    {
        if (fShutdown)
            return;

        int newPercent = tribler.GetDownloadingPercent(item.first.GetHex());
        triblerData.UpdateOne(item.first, newPercent);

        Sleep(500);
    }

    bool fConnected = tribler.IsConnected();

    {
        LOCK(triblerData.cs);
        triblerConnectionStatus = fConnected;
    }
}

void TriblerData::UpdateOne(const uint160 &magnetHash, int percent)
{
    {
        LOCK(cs);
        actualData[magnetHash] = percent;
    }
}

void TriblerUpdater()
{
    while (!fShutdown)
    {
        triblerData.UpdateAllData();

        if (fShutdown)
            return;

        Sleep(3000);
    }

}
bool TriblerData::IsConnected()
{
    return triblerConnectionStatus;
}
