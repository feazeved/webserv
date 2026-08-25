// 
void tokenizer_dump(std::vector<Token> &tokens) {
	tokIter it =  tokens.begin();
	std::cout << "---Print tokens---\n\n";
	for (;it != tokens.end(); it++)
	{
		std::string tp;
		switch (it->type) {
			case Token::OPEN_BRACKET: tp = "open bracket"; break;
			case Token::CLOSE_BRACKET: tp = "close bracket"; break;
			case Token::SEMICOLON: tp = "semicolon"; break;
			case Token::WORD: tp = "word"; break;
			// TODO: default?
		}
		std::cout << tp ;
		if (tp == "word")
			std::cout << "(" << it->value << ")";
		std::cout << "\n";
	}
}

void config_dump(std::vector<ServerConfig> &config) {
	std::vector<ServerConfig>::iterator it =  config.begin();
	std::cout << "---Print Config---\n\n";
	for (; it != config.end(); it++)
	{
		std::cout << "SERVER\n";
		std::cout << "\tlisten: " << (*it).port << "\n";
		std::cout << "\thost: " << (*it).host << "\n";
		std::cout << "\tmax_body_size: " << (*it).maxBodySize << "\n";
		std::cout << "\n";

		std::vector<Location>::iterator itl =  (*it).locations.begin();
		for (; itl != (*it).locations.end(); itl++)
		{
			std::cout << "\tLOCATION " << (*itl).path << "\n";
			std::cout << "\t\troot: " << (*itl).root << "\n";
			std::cout << "\t\tindex: " << (*itl).index << "\n";
			std::cout << "\t\tupload_store: " << (*itl).upload_store << "\n";
			std::cout << "\t\tautoindex: " << ((*itl).autoindex ? "on " : "off ") << "\n";
			std::cout << "\n";
		}
	}
}
