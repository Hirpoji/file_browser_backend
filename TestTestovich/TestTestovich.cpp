#include "filesystem_operations.h"
#include "websocket_server.h"

int main()
{
    createStorageDirectory();
    startWebSocketServer();
    return 0;
}
