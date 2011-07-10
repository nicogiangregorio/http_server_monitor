//
// async_client.cpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include "http_client_sync.h"
#include "html_parser.h"
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/threadpool.hpp>

using namespace boost::threadpool;
boost::asio::io_service io_service;
typedef boost::mutex::scoped_lock scoped_lock;

boost::mutex mutex;

void execute_init_task(server_status *srv_stat, const string &server_name, const string &path)
{
    scoped_lock(mutex);
    srv_stat->server_name = server_name;
    srv_stat->path = path;
}

void execute_http_task(server_status *srv_stat)
{
    std::stringbuf buf;

    try
    {
        scoped_lock(mutex);
        http_client c(io_service, srv_stat->server_name, srv_stat->path, buf);
        html_parser hparser(buf);
        hparser.get_server_infos(srv_stat);
        srv_stat->status = 0;
    }
    catch (std::exception& e)
    {
        srv_stat->status = -1;
        std::cout << "Exception: " << e.what() << "\n";
        //return 1;
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: async_client <server1> <server2> ...\n";
        std::cout << "Example:\n";
        std::cout << "  async_client www.boost.org/LICENSE_1_0.txt\n";
        return 1;
    }



    int num_server = argc - 1;
    server_status status[num_server];
    pool tp(num_server);

    for (int ii = 0; ii < num_server; ii++)
    {
        std::string str(argv[ii + 1]);
        size_t pos = str.find("/");

        if (pos > 0)
        {
            tp.schedule(boost::bind(&execute_init_task,&status[ii],str.substr(0, pos), str.substr(pos, str.size())));
            tp.schedule(boost::bind(&execute_http_task,&status[ii]));
            tp.wait();
        }
    }

    for (int ii = 0; ii < num_server; ii++)
    {
        std::cout << "~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
        std::cout << "Server name: " << status[ii].server_name << endl;
        std::cout << "Status: " << status[ii].status << endl;
        std::cout << "Last restart: " << status[ii].last_start << endl;
        std::cout << "Last cpu usage: " << status[ii].cpu_usage << "%" << endl;
        std::cout << "Uptime: " << status[ii].uptime << endl;
        std::cout << "Number of idle threads: " << status[ii].idle_threads << endl;
        std::cout << "~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
    }
    return 0;
}
