#include "ProtocolServer.hpp"

#ifndef PLATFORM_MSVC
  #include <arpa/inet.h>
  #include <sys/types.h> 
  #include <sys/socket.h>
  #include <poll.h>
  #include <unistd.h>
  #include <errno.h>
  #include <fcntl.h>
#endif

#include <string.h>
#include <thread>
#include <chrono>

constexpr long POLL_TIMEOUT_MSEC = 100; // 100 ms
constexpr size_t SOCKET_RECV_SIZE = 1024;

ProtocolServer::ProtocolServer(uint16_t port) 
    : port(port), acceptingClients(false), receivingData(false)
{

}

bool ProtocolServer::initialize()
{
    acceptSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (Utility::isSocketInvalid(acceptSocket))
    {
        return false;
    }
    // set socket to non-blocking - apparently non-functional with current wut impl;
    // falling back to blocking setup
    // if (!Utility::setSocketBlockingFlag(acceptSocket, false))
    // {
    //   return false;
    // }
    memset(reinterpret_cast<char*>(&serverAddr), '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    if(bind(acceptSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        return false;
    }
    listen(acceptSocket, 5);
    return true;
}

bool ProtocolServer::start()
{
    // TODO: check we are initialized
    acceptingClients = true;
    receivingData = true;
    processingRequests = true;
    acceptThread = std::thread(&ProtocolServer::acceptCallback, this);
    receiveThread = std::thread(&ProtocolServer::receiveCallback, this);
    processingThread = std::thread(&ProtocolServer::processingCallback, this);
    return true;
}

bool ProtocolServer::stop()
{
    acceptingClients = false;
    receivingData = false;
    processingRequests = false;

    // TODO: possibly add a more robust termination signalling system with CVs
    // or future etc. so we can wait a bit for a clean exit, then force close
    // the sockets if needed

    // close sockets
    if (acceptSocket != -1)
    {
        SOCK_CLOSE(acceptSocket);
    }

    for (const auto& sock : newSockets)
    {
        SOCK_CLOSE(sock);
    }
    newSockets.clear();
    for (const auto& entry : socketDataMap)
    {
        SOCK_CLOSE(entry.first);
    }
    socketDataMap.clear();
    // push an invalid element and wake up processing thread
    {
        std::lock_guard<std::mutex> guard(serverRequestsMut);
        serverRequests.push({ INVALID_SOCKET, "" });
    }
    serverRequestsCV.notify_all();

    // wait for server threads to stop
    if (acceptThread.joinable())
    {
        acceptThread.join();
    }
    if (receiveThread.joinable())
    {
        receiveThread.join();
    }
    if (processingThread.joinable())
    {
        processingThread.join();
    }

    acceptSocket = -1;
    return true;
}

int ProtocolServer::acceptCallback()
{
    SocketType clientSocket = INVALID_SOCKET;
    int haveData;
    socklen_t clientLen;
    char clientIP[64];
    sockaddr_in clientAddr{};
    pollfd pfds[1]; // only need to hold one, but treat as an array for formality

    memset(clientIP, '\0', sizeof(clientIP));
    pfds[0].fd = acceptSocket;
    pfds[0].events = POLLIN;
    clientLen = sizeof(clientAddr);

    Utility::platformLog("starting accept loop\n");

    while(acceptingClients)
    {
        haveData = SOCK_POLL(pfds, 1, POLL_TIMEOUT_MSEC * 10);
        if (haveData == 0)
        {
            if(haveData < 0)
            {
                Utility::platformLog("exited poll with errno %d\n", errno);
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
           // Utility::platformLog("waiting to accept...\n");
            continue;
        }
        // TODO: add handling of error events?
        if(pfds[0].revents & POLLIN)
        {
            clientSocket = accept(acceptSocket, (struct sockaddr*)&clientAddr, &clientLen);
            Utility::platformLog("accepted socket fd %d\n", clientSocket);
            if (!Utility::isSocketInvalid(clientSocket))
            {
                inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
                Utility::platformLog("client FD %d connected from addr %s\n", clientSocket, clientIP);
                std::lock_guard<std::mutex> guard(newSocketMut);
                newSockets.push_back(clientSocket);
            }
        }
    }
    Utility::platformLog("Accept thread exiting.\n");
    return 0;
}

void ProtocolServer::handleSocketRecvError(SocketType sock, int err)
{
    Utility::platformLog("Got socket recv error: %d\n", err);
    switch (err)
    {
#ifdef PLATFORM_MSVC
    case EAGAIN:
#endif
    case EWOULDBLOCK:
        break;
    default:
        break;
    }
}

void ProtocolServer::handleSocketDisconnect(SocketType sock)
{
    removedSockets.push_back(sock);
}

void ProtocolServer::processSocketData()
{
    size_t newline = 0;
    for (auto& entry : socketDataMap)
    {
        const auto& sock = entry.first;
        auto& data = entry.second;
        {
            while (data.size() > 0 && (newline = data.find_first_of('\n')) != std::string::npos)
            {
                std::lock_guard<std::mutex> guard(serverRequestsMut);
                serverRequests.push({ sock, data.substr(0, newline) });
                data.erase(0, newline + 1);
            }
        }
    }
    serverRequestsCV.notify_all();
}

int ProtocolServer::receiveCallback()
{
    pollfd* pfds{nullptr};
    size_t prevSocketCount = 0, newSocketCount = 0, pfdsIndex = 0, bytesAvailable = 0;
    int haveData = 0, bytesReceived = 0;
    char receivedData[SOCKET_RECV_SIZE];
    while (receivingData)
    {
        {
            std::lock_guard<std::mutex> guard(newSocketMut);
            for (const auto& sockToAdd : newSockets)
            {
                // NOTE: may be unecessary if inherited from accept socket; disabled for now since
                // it doesn't seem to work on DKP
                //if (!Utility::setSocketBlockingFlag(sockToAdd, false))
                //{
                //    Utility::platformLog("Unable to set socket %d to non-blocking\n", sockToAdd);
                //    continue;
                //}
                socketDataMap.emplace(sockToAdd, "");
            }
            newSockets.clear();
        }
        newSocketCount = socketDataMap.size();
        if (newSocketCount != prevSocketCount)
        {
            if (pfds)
            {
                delete[] pfds;
                pfds = nullptr;
            }
            if (newSocketCount > 0) {
                pfds = new pollfd[newSocketCount];
            }
            prevSocketCount = newSocketCount;
        }
        if (!pfds) // no clients case
        {
            //Utility::platformLog("waiting for clients...\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(POLL_TIMEOUT_MSEC)); // POLL_TIMEOUT_MSEC
            continue;
        }
        // assemble poll file descriptor set
        pfdsIndex = 0;
        for (const auto& entry : socketDataMap)
        {
            const auto sock = entry.first;
            pfds[pfdsIndex].fd = sock;
            pfds[pfdsIndex].events = POLLIN;
            pfdsIndex++;
            // extra bounds check just in case and to suppress warning
            if (pfdsIndex >= newSocketCount) break; 
        }
        // call poll on descriptor set
        haveData = SOCK_POLL(pfds, static_cast<unsigned int>(newSocketCount), POLL_TIMEOUT_MSEC);
        if (haveData == 0)
        {
            Utility::platformLog("poll no data\n");
            continue; // timeout case
        }
        else if (haveData < 0)
        {
            // handle error case by just logging for now
            Utility::platformLog("poll encountered an error %d\n", errno);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        Utility::platformLog("socks with revents: %d\n", haveData);
        pfdsIndex = 0;
        for (auto& entry : socketDataMap)
        {
            const auto sock = pfds[pfdsIndex].fd;
            auto& data = entry.second;
            auto& revents = pfds[pfdsIndex].revents;
            if (sock != entry.first)
            {
                Utility::platformLog("Encountered pfds/socketDataMap mismatch!\n");
                continue;
            }
            if (revents & POLLIN)
            {
                if (!Utility::getSocketBytesAvailable(sock, bytesAvailable) || bytesAvailable == 0)
                {
                    // such a situation after getting a POLLIN event indicates disconnect
                    Utility::platformLog("Unable to retreive number of bytes avaialble for sock %d\n", sock);
                    handleSocketDisconnect(sock);
                    continue;
                }
                if (bytesAvailable > sizeof(receivedData))
                {
                    bytesAvailable = sizeof(receivedData);
                }
                Utility::platformLog("attempting to recv %d bytes from sockfd %d\n", bytesAvailable, sock);
                bytesReceived = recv(sock, receivedData, static_cast<int>(bytesAvailable), 0);
                if (bytesReceived == 0) // TODO: orderly disconnect case
                {
                    handleSocketDisconnect(sock);
                }
                else if (bytesReceived < 0) // TODO: err or timeout case
                {
                    handleSocketRecvError(sock, errno);
                }
                else // got data case
                {
                    data.append(receivedData, bytesReceived);
                }
            }
            if (revents & (POLLHUP | POLLNVAL | POLLERR))
            {
                handleSocketDisconnect(sock); // sudden disconnect/abort case
            }
            pfdsIndex++;
            if (pfdsIndex >= newSocketCount) break;
        }

        processSocketData();

        for (auto sock : removedSockets)
        {
            Utility::platformLog("removing client at sock %d\n", sock);
            socketDataMap.erase(sock);
            SOCK_CLOSE(sock);
        }
        removedSockets.clear();
    }
    if (pfds)
    {
        delete[] pfds;
    }
    Utility::platformLog("receive thread exiting.\n");
    return 0;
}

int ProtocolServer::processingCallback()
{
    int sentBytes = 0;
    while (processingRequests)
    {
        std::unique_lock<std::mutex> lock(serverRequestsMut);
        // can possibly replace with a non-tiumeout wait and push in some kind of 
        // signal entry (e.g. sock = -1) value to stop
        serverRequestsCV.wait(lock, [&] {return serverRequests.size() > 0; });
        ServerRequest request = serverRequests.front();
        serverRequests.pop();
        lock.unlock();
        //process request
        if (Utility::isSocketInvalid(request.sock))
        {
            continue;
        }
        Utility::platformLog("handling: %s\n", request.data.c_str());
        std::string response;
        // need to do anything different for error or not?
        commandHandler.handleCommand(request.data, response);

        if ((sentBytes = send(request.sock, response.c_str(), static_cast<int>(response.size()), 0)) < 0)
        {
            Utility::platformLog("got error trying to send on sock %d: %d\n", request.sock, errno);
            continue;
        }
        if (static_cast<size_t>(sentBytes) != response.size())
        {
            // if this is happening, will need to introduce send inside while loop 
            Utility::platformLog(
                "only able to send %d / %d bytes for socket %d\n", 
                sentBytes, 
                response.size(), 
                request.sock
            );
            continue;
        }
    }
    Utility::platformLog("processing thread exiting.\n");
    return 0;
}
