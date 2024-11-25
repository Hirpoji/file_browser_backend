#include <uwebsockets/App.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <fstream>
#include <mutex>

using namespace std;
using json = nlohmann::json;

struct ClientSession {
    std::string currentDirectory = "C:/storage";
};

std::unordered_map<void*, ClientSession> clientSessions;
std::string rootDirectory = "C:/storage";
std::mutex monitorMutex;

json listDirectory(const std::string& directory) {
    json response;
    response["type"] = "directory";
    response["path"] = directory;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            json item;
            item["name"] = entry.path().filename().string();
            item["type"] = entry.is_directory() ? "folder" : "file";
            item["size"] = entry.is_regular_file() ? std::filesystem::file_size(entry) : 0;
            item["last_modified"] = std::chrono::duration_cast<std::chrono::seconds>(
                std::filesystem::last_write_time(entry).time_since_epoch())
                .count();
            response["contents"].push_back(item);
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        response["error"] = e.what();
    }

    return response;
}

void handleClientMessage(auto* ws, json& received, json& response) {
    auto& session = clientSessions[ws];

    if (received["action"] == "list_directory") {
        std::string relativePath = received.value("path", ".");
        std::string fullPath = std::filesystem::absolute(session.currentDirectory + "/" + relativePath).string();

        if (std::filesystem::exists(fullPath) && std::filesystem::is_directory(fullPath)) {
            response = listDirectory(fullPath);
        }
        else {
            response["error"] = "Directory does not exist";
        }
    }
    else if (received["action"] == "change_directory") {
        std::string relativePath = received.value("path", ".");
        std::string newPath;

        if (relativePath == "..") {
            newPath = std::filesystem::path(session.currentDirectory).parent_path().string();
        }
        else {
            newPath = std::filesystem::absolute(session.currentDirectory + "/" + relativePath).string();
        }

        std::cout << "Request to change directory: " << newPath << std::endl;

        if (std::filesystem::exists(newPath) && std::filesystem::is_directory(newPath)) {
            session.currentDirectory = newPath;
            response["message"] = "Directory changed successfully";
            response["currentPath"] = session.currentDirectory;
        }
        else {
            response["error"] = "Directory does not exist";
        }
    }
    else if (received["action"] == "create_folder") {
        std::string folderName = received["name"];
        std::string fullPath = session.currentDirectory + "/" + folderName;

        if (!std::filesystem::exists(fullPath)) {
            std::filesystem::create_directory(fullPath);
            response["message"] = "Folder created successfully";
            response["path"] = fullPath;
        }
        else {
            response["error"] = "Folder already exists";
        }
    }
    else if (received["action"] == "delete") {
        std::string relativePath = received["path"];
        std::string fullPath = std::filesystem::absolute(session.currentDirectory + "/" + relativePath).string();

        if (std::filesystem::exists(fullPath)) {
            try {
                if (std::filesystem::is_directory(fullPath)) {
                    std::filesystem::remove_all(fullPath);
                }
                else {
                    std::filesystem::remove(fullPath);
                }
                response["message"] = "Deleted successfully";
                response["updated_directory"] = listDirectory(session.currentDirectory);
            }
            catch (const std::exception& e) {
                response["error"] = std::string("Error deleting: ") + e.what();
            }
        }
        else {
            response["error"] = "File or folder does not exist";
        }
    }
    else if (received["action"] == "rename") {
        std::string relativePath = received.value("path", ".");
        std::filesystem::path oldPath = std::filesystem::absolute(std::filesystem::path(session.currentDirectory) / relativePath);
        std::filesystem::path newPath = std::filesystem::absolute(std::filesystem::path(session.currentDirectory) / received["new_name"].get<std::string>());

        if (std::filesystem::exists(oldPath)) {
            std::filesystem::rename(oldPath, newPath);
            response["message"] = "Renamed successfully";
        }
        else {
            response["error"] = "File or folder does not exist";
        }
    }
    else if (received["action"] == "upload") {
        std::string fileName = received["name"];
        std::string fullPath = session.currentDirectory + "/" + fileName;
        std::string fileContent = received["content"];

        std::ofstream outFile(fullPath, std::ios::binary);
        if (outFile.is_open()) {
            outFile << fileContent;
            outFile.close();
            response["message"] = "File uploaded successfully";
            response["path"] = fullPath;
        }
        else {
            response["error"] = "Failed to upload file";
        }
    }
    else if (received["action"] == "file_info") {
        std::string relativePath = received["path"];
        std::string fullPath = std::filesystem::absolute(session.currentDirectory + "/" + relativePath).string();

        if (std::filesystem::exists(fullPath)) {
            json info;
            info["name"] = std::filesystem::path(fullPath).filename().string();
            info["type"] = std::filesystem::is_directory(fullPath) ? "folder" : "file";
            info["size"] = std::filesystem::is_regular_file(fullPath) ? std::filesystem::file_size(fullPath) : 0;
            info["last_modified"] = std::chrono::duration_cast<std::chrono::seconds>(
                std::filesystem::last_write_time(fullPath).time_since_epoch())
                .count();
            response = info;
        }
        else {
            response["error"] = "File does not exist";
        }
    }
    else {
        response["error"] = "Unknown action";
    }
}

void startWebSocketServer() {
    struct PerSocketData {};

    auto app = uWS::App();

    app.ws<PerSocketData>("/*", {
        .open = [](auto* ws) {
            clientSessions[ws].currentDirectory = rootDirectory;
            ws->subscribe("filesystem_events");
            cout << "Client connected" << endl;
        },
        .message = [](auto* ws, string_view message, uWS::OpCode opCode) {
            json received = json::parse(message);
            json response;
            std::lock_guard<std::mutex> lock(monitorMutex);
            handleClientMessage(ws, received, response);
            ws->send(response.dump(), uWS::OpCode::TEXT);
        },
        .close = [](auto* ws, int code, string_view message) {
            clientSessions.erase(ws);
            cout << "Client disconnected" << endl;
        }
        }).listen(9001, [](auto* listen_socket) {
            if (listen_socket) {
                cout << "Listening on port " << 9001 << endl;
            }
            else {
                cerr << "Failed to start server" << endl;
            }
            });

    app.run();
}
