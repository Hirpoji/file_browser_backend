#include "filesystem_operations.h"
#include <filesystem>
#include <iostream>
#include <fstream>

using namespace std;
using json = nlohmann::json;

string currentDirectory = "C:/storage";

void createStorageDirectory()
{
    if (!filesystem::exists(currentDirectory))
    {
        filesystem::create_directory(currentDirectory);
        cout << "The storage folder has been created at C:/" << endl;
    }
    else
    {
        cout << "The storage folder already exists at C:/" << endl;
    }
}

string removeFolder(const string& folderName)
{
    string fullPath = currentDirectory + "/" + folderName;
    json response;

    try
    {
        if (filesystem::exists(fullPath) && filesystem::is_directory(fullPath))
        {
            filesystem::remove_all(fullPath);
            response["message"] = "Folder removed successfully";
            response["path"] = fullPath;
        }
        else
        {
            response["error"] = "Folder does not exist or is not a directory";
        }
    }
    catch (const filesystem::filesystem_error& e)
    {
        response["error"] = e.what();
    }

    return response.dump();
}

string removeFile(const string& fileName)
{
    string fullPath = currentDirectory + "/" + fileName;
    json response;

    try
    {
        if (filesystem::exists(fullPath) && filesystem::is_regular_file(fullPath))
        {
            filesystem::remove(fullPath);
            response["message"] = "File removed successfully";
            response["path"] = fullPath;
        }
        else
        {
            response["error"] = "File does not exist";
        }
    }
    catch (const filesystem::filesystem_error& e)
    {
        response["error"] = e.what();
    }

    return response.dump();
}

string renameFile(const string& oldName, const string& newName)
{
    string oldPath = currentDirectory + "/" + oldName;
    string newPath = currentDirectory + "/" + newName;
    json response;

    try
    {
        if (filesystem::exists(oldPath) && filesystem::is_regular_file(oldPath))
        {
            if (!filesystem::exists(newPath))
            {
                filesystem::rename(oldPath, newPath);
                response["message"] = "File renamed successfully";
                response["oldPath"] = oldPath;
                response["newPath"] = newPath;
            }
            else
            {
                response["error"] = "New file name already exists";
            }
        }
        else
        {
            response["error"] = "Old file does not exist or is not a file";
        }
    }
    catch (const filesystem::filesystem_error& e)
    {
        response["error"] = e.what();
    }

    return response.dump();
}

string listDirectory(const string& relativePath)
{
    string fullPath = currentDirectory + "/" + relativePath;

    json response;
    response["type"] = "directory";
    response["path"] = fullPath;

    try
    {
        for (const auto& entry : filesystem::directory_iterator(fullPath))
        {
            json item;
            item["name"] = entry.path().filename().string();
            item["type"] = entry.is_directory() ? "folder" : "file";
            response["contents"].push_back(item);
        }
    }
    catch (const filesystem::filesystem_error& e)
    {
        response["error"] = e.what();
    }

    return response.dump();
}

string createFolder(const string& folderName)
{
    string fullPath = currentDirectory + "/" + folderName;
    json response;

    try
    {
        if (!filesystem::exists(fullPath))
        {
            filesystem::create_directory(fullPath);
            response["message"] = "Folder created successfully";
            response["path"] = fullPath;
        }
        else
        {
            response["error"] = "Folder already exists";
        }
    }
    catch (const filesystem::filesystem_error& e)
    {
        response["error"] = e.what();
    }

    return response.dump();
}

string changeDirectory(const string& folderName)
{

    string newPath = currentDirectory + "/" + folderName;

    try
    {
        newPath = std::filesystem::canonical(newPath).string();
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        json response;
        response["error"] = "Invalid path: " + string(e.what());
        return response.dump();
    }

    json response;

    try
    {
        if (filesystem::exists(newPath) && filesystem::is_directory(newPath))
        {
            currentDirectory = newPath;
            response["message"] = "Directory changed successfully";
            response["currentPath"] = currentDirectory;
        }
        else
        {
            response["error"] = "Folder does not exist or is not a directory";
        }
    }
    catch (const filesystem::filesystem_error& e)
    {
        response["error"] = e.what();
    }

    return response.dump();
}

string renameFolder(const string& oldName, const string& newName)
{
    string oldPath = currentDirectory + "/" + oldName;
    string newPath = currentDirectory + "/" + newName;
    json response;

    try
    {
        if (filesystem::exists(oldPath) && filesystem::is_directory(oldPath))
        {
            if (!filesystem::exists(newPath))
            {
                filesystem::rename(oldPath, newPath);
                response["message"] = "Folder renamed successfully";
                response["oldPath"] = oldPath;
                response["newPath"] = newPath;
            }
            else
            {
                response["error"] = "New folder name already exists";
            }
        }
        else
        {
            response["error"] = "Old folder does not exist or is not a directory";
        }
    }
    catch (const filesystem::filesystem_error& e)
    {
        response["error"] = e.what();
    }

    return response.dump();
}

string saveFile(const string& fileName, const string& fileContent) {
    string filePath = currentDirectory + "/" + fileName;
    json response;

    try {
        ofstream file(filePath, ios::binary);
        if (file.is_open()) {
            file << fileContent;
            file.close();
            response["message"] = "File uploaded successfully";
            response["path"] = filePath;
        }
        else {
            response["error"] = "Failed to open file for writing";
        }
    }
    catch (const std::exception& e) {
        response["error"] = e.what();
    }

    return response.dump();
}
