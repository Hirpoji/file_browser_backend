#pragma once
#ifndef FILESYSTEM_OPERATIONS_H
#define FILESYSTEM_OPERATIONS_H

#include <string>
#include <nlohmann/json.hpp>

void createStorageDirectory();
std::string listDirectory(const std::string& relativePath);
std::string createFolder(const std::string& folderName);
std::string changeDirectory(const std::string& folderName);
std::string renameFolder(const std::string& oldName, const std::string& newName);
std::string saveFile(const std::string& fileName, const std::string& fileContent);
std::string removeFolder(const std::string& folderName);
std::string removeFile(const std::string& fileName);
std::string renameFile(const std::string& oldName, const std::string& newName);

#endif 
