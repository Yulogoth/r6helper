#pragma once
#include "libs/Json.h"
#include "Options.h"
#include <unordered_map>
#include <array>
#include <algorithm>
#include <fstream>

class pstring : public std::string
{
public:
    pstring() : std::string() {}

    template<typename T>
    pstring(const T v) : std::string(v) {}

    template<typename T>
    pstring& operator<<(const T s)
    {
        std::stringstream stream;
        stream << *this;
        stream << s;
        *this = stream.str();
        return *this;
    }

    pstring& operator+(const unsigned int i)
    {
        std::stringstream stream;
        stream << *this;
        stream << i;
        *this = stream.str();
        return *this;
    }
};


namespace Config
{
    void GetVal(Json::Value& config, int* setting)
    {
        if (config.isNull())
            return;

        *setting = config.asInt();
    }

    void GetVal(Json::Value& config, bool* setting)
    {
        if (config.isNull())
            return;

        *setting = config.asBool();
    }

    void GetVal(Json::Value& config, float* setting)
    {
        if (config.isNull())
            return;

        *setting = config.asFloat();
    }

    void GetVal(Json::Value& config, char** setting)
    {
        if (config.isNull())
            return;

        *setting = _strdup(config.asCString());
    }

    void GetVal(Json::Value& config, char* setting)
    {
        if (config.isNull())
            return;

        strcpy(setting, config.asCString());
    }

    void SaveCFG(std::string fileName) {
        Json::Value settings;

        settings["platform"] = *g_Options.platform;
        settings["timeout"] = *g_Options.timeout;
        settings["be"] = &g_Options.be;

        Json::StyledWriter styledWriter;
        std::string strJson = styledWriter.write(settings);
        FILE* file = fopen(fileName.c_str(), "w+");
        if (file)
        {
            fwrite(strJson.c_str(), 1, strJson.length(), file);
            fclose(file);
        }
    }

    void LoadCFG(std::string fileName) {
        FILE* infile = fopen(fileName.c_str(), "r");

        if (!infile)
        {
            SaveCFG(fileName);
            return;
        }

        fseek(infile, 0, SEEK_END);
        long filesize = ftell(infile);
        char* buf = new char[filesize + 1];
        fseek(infile, 0, SEEK_SET);
        fread(buf, 1, filesize, infile);
        fclose(infile);

        buf[filesize] = '\0';
        std::stringstream ss;
        ss.str(buf);
        delete[] buf;

        Json::Value settings;
        ss >> settings;

        GetVal(settings["platform"], g_Options.platform);
        GetVal(settings["timeout"], g_Options.timeout);
        GetVal(settings["be"], g_Options.be);
    }
}