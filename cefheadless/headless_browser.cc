// Copyright (c) 2023  François Leblanc <fleblanc50@gmail.com>

#include <iostream>
#include <fstream>
#include <filesystem>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "include/base/cef_logging.h"
#include "cefheadless/headless_browser.h"

HeadlessBrowser::HeadlessBrowser(const CefRefPtr<CefBrowser>& browser) :
	_browser(browser),
	output(-1) {

	static int index;

	index++;

	namefifo = "cefheadlessbrowser" + std::to_string(index) + ".w2f";

	//TODO: error
	mkfifo(namefifo.c_str(), 666);

	using std::filesystem::perms;
	std::filesystem::permissions(namefifo,
			perms::owner_all | perms::group_all,
			std::filesystem::perm_options::add);

	//output = open(namefifo.c_str(), O_WRONLY | O_NONBLOCK);
									
	//std::cerr << "output is: " << output << std::endl;


}

HeadlessBrowser::~HeadlessBrowser() {
	if(output>0)
	{
		close(output);
		output = -1;
	}
}

void HeadlessBrowser::write(const std::string& text) {
	/*
	std::ofstream output("ici.txt");//namefifo);
	output << text << std::endl;
	output.close();
	*/

	std::cout << text << std::endl;
/*
	if(output<0)
	{
		output = open(namefifo.c_str(), O_WRONLY);// | O_NONBLOCK);
	}

	//static const char* end  = "\n";

	::write(output, text.c_str(), text.length());
	//::write(output, end, sizeof(end));
	
	close(output);
*/
}



