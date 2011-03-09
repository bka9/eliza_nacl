

#include "eliza.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ppapi/c/pp_errors.h>
#include <ppapi/c/ppb_instance.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/var.h>

namespace {
    bool IsError(int32_t result) {
      return ((PP_OK != result) && (PP_ERROR_WOULDBLOCK != result));
    }
}  

Eliza::Eliza(pp::Instance* instance)
    : instance_id_(instance->pp_instance()),
      url_request_(instance),
      url_loader_(instance),
      cc_factory_(this),
      m_bMemoryRecall(0),
      m_nUserRepeatCount(0),
      m_sUserName("USER"){
  url_request_.SetURL("script.txt");
  url_request_.SetMethod("GET");
  load_data();
}

void Eliza::read_data(){
    vstring tokens;
    tok.setDelim("\n");
    tok.tokenize(url_response_body_,tokens);
    std::string temp = "";
    int counter = 0, counter2 = 0, line = 0;
	transpos current_transpos;
    char cSymbol = 0;
    char cPrevSymbol;
    int number_of_tokens = tokens.size();
    std:string buffer = "";
    for(int i = 0; i < number_of_tokens; i++  ){
        buffer = tokens[i];
        cPrevSymbol = cSymbol;
        cSymbol = buffer[0];
        if(temp.length() > 0 && cSymbol != ';' 
    		&& cSymbol != cPrevSymbol) {
			temp.erase(temp.length() - 1, 1);
			comments.push_back(temp);
			temp.erase();
		}
		buffer.erase(0, 1);
		switch(cSymbol) {
		case ';':
			temp += ';';
			temp += buffer;
			temp += '\n';
			dump_data();
			break;
		case 'S':
			signOn.push_back(buffer);
			break;
		case 'T':
			++counter;
			buffer.erase(buffer.length() - 1, 1);
			if(counter % 2 == 1) {
				current_transpos.verbP1 = buffer;
			} else {
				current_transpos.verbP2 = buffer;
				transpos_list.push_back(current_transpos);
			}
			break;
		case 'E':
			++counter2;
			buffer.erase(buffer.length() - 1, 1);
			if(counter2 % 2 == 1) {
				current_correction.verbP1 = buffer;
			} else {
				current_correction.verbP2 = buffer;
				correction_list.push_back(current_correction);
			}
			break;
		case 'K':
			buffer.erase(buffer.length() - 1, 1);
			current_data.keywords.push_back(buffer);
			break;
		case 'R':
			current_data.responses.push_back(buffer);
			break;
		case 'M':
			subjectRecall.push_back(buffer);
			break;
		case 'N':
			nullResponse.push_back(buffer);
			break;
		case 'X':
			noKeyWord.push_back(buffer);
			break;
		case 'Z':
			topicChanger.push_back(buffer);
			break;
		case 'W':
			inputRepeat.push_back(buffer);
			break;
		case 'C':
			current_data.contexts.push_back(buffer);
			break;
		case 'A':
			current_data.cmd.push_back(buffer);
			break;
		case 0:
			break;
		default:
            ReportResultAndDie("Unrecognized token",false);
			break;
		}
	}
    	dump_data();
    	m_nTransPosNum = transpos_list.size();
    	m_nCorrectionNum = correction_list.size();
    }
}
inline bool Eliza::bot_repeat() const {
    int pos = rfind(vResponseLog, m_sResponse);
	if(pos != -1) {
		return (pos + 1 < response_list.size());
	}
	return 0;
}

bool Eliza::similar_response() const {
    int len = m_sResponse.length();
	int len2 = m_sPrevResponse.length();
	int repeat_count = 0;
	for(int i = 0; i < len && i < len2; ++i) {
		if(m_sResponse[i] == m_sPrevResponse[i]) {
			++repeat_count;
		} else {
			break;
		}
	}
	return repeat_count > 1;
}

std::string Eliza::start(){
    return "Hello, my name is Eliza. What is your name?";
}
std::string Eliza::respond(str::string text){
    m_sInput = text;
    preProcessInput();
    save_prev_response();

    if(m_sInput.length() == 0) {
		response_list = nullResponse;
	} else if(user_repeat()) {
		handle_user_repetition();
	} else {
		reset_repeat_count();
		find_response();
	}
	
	select_response();
	preProcessResponse();
	handle_repetition();
    return m_sResponse;
}

// select responses randomly from a list of responses
inline void Eliza::select_response() {
    shuffle(response_list, response_list.size());
	m_sResponse = response_list[0];
}

void Eliza::memorise_input() {
    m_sSymbol = "@";
	extract_suffix();
	memory.push(m_sSuffix);
}

void Eliza::save_user_name() {
    if(m_sKeyWord == "MY NAME IS" || m_sKeyWord == "YOU CAN CALL ME") {
		 extract_suffix();
		 m_sUserName = m_sSuffix;
		 trimLR(m_sUserName, " ");
	}
}

inline void Eliza::extract_suffix() {
    int pos = m_sInput.find(m_sKeyWord);
	m_sSuffix.erase();
	if(pos != std::string::npos) {
		if(m_sSymbol == "*") {
			pos += m_sKeyWord.length();
			m_sSuffix = m_sInput.substr(pos, m_sInput.length() - pos);
		} else if(m_sSymbol == "@") {
			m_sSuffix = m_sInput.substr(pos, m_sInput.length());
		} else if(m_sSymbol == "%") {
			m_sSuffix = m_sPrevResponse;
		} 
		trimLR(m_sSuffix, " ");
	}
}



void Eliza::find_response() {
    find_keyword();
	std::string tempStr = m_sKeyWord;
	tempStr.insert(0, " ");
	tempStr.append(" ");
	if(tempStr.find(" MY ") != std::string::npos ||
		tempStr.find(" I'M ") != std::string::npos ||
		tempStr.find(" I ") != std::string::npos) {
		memorise_input();
	}
	save_user_name();
	if(!bot_understand()) {
		handle_unknown_sentence();
	}
}

void Eliza::add_response(vstring v) {
    int size = v.size();
	for(int i = 0; i < size; ++i) {
		response_list.push_back(v[i]);
	}
}

// helper function for the procedure "find_keyword"
inline bool Eliza::isGoodKey(const std::string key, const std::string bkey, int pos, int bestp) const{
    if(pos != std::string::npos && (key.length() > bkey.length() || 
		(pos != bestp && key.length() == bkey.length()))) {
		return 1;
	}
	return 0;
}

// function for finding the best keyword from the database
// this function also handles context and also verifies the
// boundaries of keywords that are found inside the user input
void Eliza::find_keyword() {
    response_list.clear();
	std::string bestKey;
	bool bGeneralKey = 0;
	int bestPos = -1;
	int dbase_size = database.size();
	for(int index = 0; index < dbase_size; ++index) {
		verify_context(database[index].contexts);
		if(good_context()) {
			keyword_list = database[index].keywords;
			size_t keyNum = keyword_list.size();
			for(int i = 0; i < keyNum; ++i) {
				std::string thisKey = keyword_list[i];
				std::string keyword = thisKey;
				trimLR(keyword, "_");
				keyword.insert(0, " ");
				keyword.append(" ");
				if(thisKey == "*" && bestKey == "") {
					keyword = m_sInput;
					bGeneralKey = 1;
				}
				int thisPos = m_sInput.find(keyword);
				if(thisPos == std::string::npos) {
					continue;
				}
				verify_keyword_boundary(thisKey, thisPos);
				if(wrong_boundary()) {
					continue;
				} 
				if(bGeneralKey) {
					thisKey = "*";
					bGeneralKey = 0;
				}
				trimLR(thisKey, "_");
				if(isGoodKey(thisKey, bestKey, thisPos, bestPos)) {
					if(thisKey.length() > bestKey.length()) {
						response_list = database[index].responses;
					} else {
						add_response(database[index].responses);
					}
					bestKey = thisKey;
					bestPos = thisPos;
				}
			}
		}
	}
	bestKey == "*" ? m_sKeyWord = m_sInput : m_sKeyWord = bestKey;
}

// handle the context of the conversation
// (the previous replies of the chatterbot)
void Eliza::verify_context(vstring vContext) { 

	m_bGoodContext = 0;
	
	size_t nNum = vContext.size();

	if(nNum == 0) {
		m_bGoodContext = 1;
		return;
	}
	
	for(int i = 0; i < nNum; ++i) {
		if(is_template(vContext[i])) {
			findSymbol(vContext[i]);
			replace(vContext[i], m_sSymbol, m_sSuffix);
		}

		if(m_sPrevResponse == vContext[i]) {
			m_bGoodContext = 1;
			break;
		}
	}
}

void Eliza::verify_keyword_boundary(std::string keyWord, int pos) {
	bool bFrontWordsAloud = 0;
	bool bBackWordsAloud = 0;

	m_bWrongBoundary = 0;
	
	if(keyWord[0] != '_') {
		bFrontWordsAloud = 1;
	} else {
		trimLeft(keyWord, '_');
	}
	
	char lastChar = keyWord[keyWord.length()-1];
	if(lastChar != '_') {
		bBackWordsAloud = 1;
	} else {
		trimRight(keyWord, '_');
	}

	keyWord.insert(0, " ");
	keyWord.append(" ");
	
	if(!bFrontWordsAloud && pos > 0) {
		m_bWrongBoundary = 1;
	}

	else if(!bBackWordsAloud && (pos + keyWord.length()) != m_sInput.length()) {
		m_bWrongBoundary = 1;
	}

	else if(bBackWordsAloud && (pos + keyWord.length()) == m_sInput.length()) {
		m_bWrongBoundary = 1;
	}
}


inline void Eliza::handle_repetition() {
    while(bot_is_repeating()) {
		if(response_list.size() > 1) {
			response_list.erase(response_list.begin());
		} 
		else {
			break;
		}
		select_response();
		preProcessResponse();
	}
	if(bot_is_repeating()) {
		if(memory.size() > 0) {
			remind_prev_subject();
		} else {
			response_list = topicChanger;
		}
		select_response();
		preProcessResponse();
	}
	vResponseLog.push_back(m_sResponse);
}
void Eliza::handle_user_repetition() {
    ++m_nUserRepeatCount;
	if(m_nUserRepeatCount > 1) {
		response_list = inputRepeat;
	} else {
		find_response();
	}
}

void Eliza::handle_unknown_sentence() {
    if(memory.size() > 0) {
		remind_prev_subject();
	} else {
		response_list = noKeyWord;
	}
}

void Eliza::remind_prev_subject() {
    response_list = subjectRecall;
	m_sSuffix = memory.top();
	m_bMemoryRecall = 1;
	memory.pop();
}

inline void Eliza::dump_data() {
    if(current_data.keywords.size() > 0) {
		database.push_back(current_data);
		clear();
	}
}

void Eliza::clear() {
    current_data.contexts.clear();
	current_data.keywords.clear();
	current_data.responses.clear();
	current_data.cmd.clear();
}

bool Eliza::load_data(){
    pp::CompletionCallback cc = cc_factory_.NewCallback(&Eliza::OnOpen);
    int32_t res = url_loader_.Open(url_request_, cc);
    if (PP_ERROR_WOULDBLOCK != res)
        cc.Run(res);
    return !IsError(res);
}

void Eliza::findSymbol(std::string str) {
    m_sSymbol.erase();
	if(str.find("*") != std::string::npos) {
		m_sSymbol = "*";
	} else if(str.find("@") != std::string::npos) {
		m_sSymbol = "@";
	} else if(str.find("%") != std::string::npos) {
		m_sSymbol = "%";
	}
}

void Eliza::preProcessInput() {
    if(m_sInput.length() > 0) {
		tok.cleanString(m_sInput, " ?!,;");
		trimRight(m_sInput, '.');
		UpperCase(m_sInput);
		m_sInput.insert(0, " ");
		m_sInput.append(" ");
    }
}

void Eliza::preProcessResponse() {
    if(is_template(m_sResponse)) {
		findSymbol(m_sResponse);
		if(m_sKeyWord == m_sInput) {
			m_sSuffix = m_sInput;
		} else if(!m_bMemoryRecall){
			extract_suffix();
		} else {
			m_bMemoryRecall = 0;
		}
		if(m_sSuffix.length() == 0) {
			while(is_template(m_sResponse) && response_list.size() > 1) {
				response_list.erase(response_list.begin());
				m_sResponse = response_list[0];
			}
			if(is_template(m_sResponse)) {
				response_list = topicChanger;
				select_response();
			}
		}
		if(m_sSuffix.length() > 0 && m_sSymbol != "%") {
			transpose_sentence(m_sSuffix);
			correct_sentence(m_sSuffix);
			trimRight(m_sSuffix, ' ');
			m_sSuffix.insert(0, " ");
		}
		replace(m_sResponse, m_sSymbol, m_sSuffix);
	}
	if(m_sUserName != "USER") {
		replace(m_sResponse, "USER", m_sUserName);
	}
}


void Eliza:OnOpen(int32_t result){
    if (result < 0){
      ReportResultAndDie("Error opening the script",false);
    }else
      ReadBody();
}

void Eliza::OnRead(int32_t result) {
  if (result < 0) {
    ReportResultAndDie("Error reading the script",false);
  } else if (result != 0) {
    int32_t num_bytes = result < kBufferSize ? result : sizeof(buffer_);
    url_response_body_.reserve(url_response_body_.size() + num_bytes);
    url_response_body_.insert(url_response_body_.end(),
                              buffer_,
                              buffer_ + num_bytes);
    ReadBody();
  } else {  // result == 0, end of stream
     read_data();
  }
}

void Eliza::ReadBody() {
  // Reads the response body (asynchronous) into this->buffer_.
  // OnRead() will be called when bytes are received or when an error occurs.
  // Look at <ppapi/c/dev/ppb_url_loader> for more details.
  pp::CompletionCallback cc = cc_factory_.NewCallback(&Eliza::OnRead);
  int32_t res = url_loader_.ReadResponseBody(buffer_,
                                             sizeof(buffer_),
                                             cc);
  if (PP_ERROR_WOULDBLOCK != res)
    cc.Run(res);
}

void Eliza::ReportResultAndDie(const std::string& text,
                                       bool success) {
  ReportResult(text, success);
  delete this;
}

void Eliza::ReportResult(const std::string& text,
                                 bool success) {
  if (success)
    printf("Eliza::ReportResult(Ok).\n");
  else
    printf("Eliza::ReportResult(Err). %s\n", text.c_str());
  fflush(stdout);
  pp::Module* module = pp::Module::Get();
  if (NULL == module)
    return;

  pp::Instance* instance = module->InstanceForPPInstance(instance_id_);
  if (NULL == instance)
    return;

  pp::Var window = instance->GetWindowObject();
  // calls JavaScript function reportResult(url, result, success)
  // defined in geturl.html.
  pp::Var exception;
  window.Call("reportResult", text, success, &exception);
}
