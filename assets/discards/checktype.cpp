static inline
bool s_checkType(const std::string &method, std::vector<std::string>::iterator &mit, std::vector<std::string>::iterator &end){
	for(; mit != end; mit++)
		if(method == *mit)
			return true;
	return false;
}