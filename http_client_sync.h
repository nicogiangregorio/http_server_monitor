#ifndef HTTP_CLIENT_SYNC_H
#define HTTP_CLIENT_SYNC_H

#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class http_client
{
public:
	http_client(boost::asio::io_service& io_service, 
		const std::string& server, 
		const std::string& path,
		std::streambuf& buf) : resolver_(io_service), socket_(io_service), output_(&buf)
	{
		handle_resolve(server);
		handle_connect();
		handle_write_request(server, path);
		handle_read_statusline();
		handle_read_header();
		handle_read_content();
	}

private:
	void handle_resolve(const std::string& server)
	{
		tcp::resolver::query query(server, "http");
		endpoint_iterator_ = resolver_.resolve(query);
	}

	void handle_connect()
	{
		error_ = boost::asio::error::host_not_found;

		while(error_ && endpoint_iterator_ != end_)
		{
			socket_.close();
			socket_.connect(*endpoint_iterator_, error_);
		}

		if (error_)
			throw boost::system::system_error(error_);
	}

	void handle_write_request(const std::string& server, const std::string& path)
	{
		// Form the request. We specify the "Connection: close" header so that the
		// server will close the socket after transmitting the response. This will
		// allow us to treat all data up until the EOF as the content.
		std::ostream request_stream(&request_);
		request_stream << "GET " << path << " HTTP/1.0\r\n";
		request_stream << "Host: " << server << "\r\n";
		request_stream << "Accept: */*\r\n";
		request_stream << "Connection: close\r\n\r\n";

		// Send the request.
		boost::asio::write(socket_, request_);
	}

	void handle_read_statusline()
	{
		// Read the response status line. The response streambuf will automatically
		// grow to accommodate the entire line. The growth may be limited by passing
		// a maximum size to the streambuf constructor.
		boost::asio::read_until(socket_, response_, "\r\n");

		std::istream response_stream(&response_);

		std::string http_version;
		response_stream >> http_version;

		unsigned int status_code;
		response_stream >> status_code;

		std::string status_message;
		std::getline(response_stream, status_message);

		if(!response_stream || http_version.substr(0,5) != "HTTP/")
			std::cout << "Invalid response\n";

		if (status_code!= 200)
			std::cout << "Response returned with status code " << status_code << "\n";
	}

	void handle_read_header()
	{
		boost::asio::read_until(socket_, response_, "\r\n\r\n");
		std::string header;
		std::istream response_stream(&response_);

		/*while(std::getline(response_stream, header) && header != "\r")
		std::cout << header << "\n";
		std::cout << std::endl;*/

		if (response_.size() > 0)
			output_ << &response_;
	}

	void handle_read_content()
	{
		while(boost::asio::read(socket_, response_, boost::asio::transfer_at_least(1), error_))
			output_ << &response_;
		if (error_ != boost::asio::error::eof)
			throw boost::system::system_error(error_);
	}

	tcp::resolver resolver_;
	tcp::resolver::iterator endpoint_iterator_;
	tcp::resolver::iterator end_;
	tcp::socket socket_;
	boost::asio::streambuf request_;
	boost::asio::streambuf response_;
	boost::system::error_code error_;
	std::ostream output_;
};

#endif