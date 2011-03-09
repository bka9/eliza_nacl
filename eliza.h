#ifndef __ELIZA__H__
#define __ELIZA__H__


#include "strings.h"

#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/url_loader.h>
#include <ppapi/cpp/url_request_info.h>
#include <ppapi/cpp/instance.h>
#include <ctime>

#define PAUSE(x) Sleep(x);

typedef struct {
    vstring keywords;
	vstring contexts;
	vstring responses;
	vstring cmd;
} data;

typedef struct {
	std::string verbP1;
	std::string verbP2;
} transpos;

typedef struct {
	std::string verbP1;
	std::string verbP2;
} correction;

class Eliza{
    
    public:
    Eliza(pp::Instance* instance_);
    std::string respond(std::string text);
    std::string start();
    bool load_data();
    void preProcessInput();
    void memorise_input();
 
    
    private:
    
    ~Eliza();
    bool bot_repeat() const;

    bool similar_response() const;
    bool bot_understand() const {
        return response_list.size() > 0;
    }
    bool bot_is_repeating() const {
    	return (bot_repeat() || similar_response());
	}
	bool user_repeat() const {
		return (m_sInput.length() > 0 && m_sInput == m_sPrevInput);
	}
    bool good_context() const {
    	return m_bGoodContext;
	}
    bool wrong_boundary() const {
    	return m_bWrongBoundary;
	}
    void save_prev_response(){
        m_sPrevResponse = m_sResponse;
    }
    void save_prev_input(){
        m_sPrevInput = m_sInput;
    }
    void simulate_thinking() {
    	PAUSE(700 + rand() % 3000);
	}
    void seed_random_generator() {
    	srand((unsigned) time(NULL));
	}
    bool is_template(std::string str) {
    	return (str.find("*") != std::string::npos ||
			str.find("@") != std::string::npos ||
			str.find("%") != std::string::npos);
	}
    void reset_repeat_count() {
    	m_nUserRepeatCount = 0;
	}
    void read_data();
    void dump_data();
    void clear();
    
    void add_response(vstring v);
    void findSymbol(std::string str);
    void save_user_name();
    
    void extract_suffix();

    void find_keyword();
    void find_response();
    void handle_user_repetition();
    void handle_unknown_sentenct();
    void select_response();
    
    // Callback fo the pp::URLLoader::Open().
    void OnOpen(int32_t result);
    
    // Callback fo the pp::URLLoader::ReadResponseBody().
    // |result| contains the number of bytes read or an error code.
    // Appends data from this->buffer_ to this->url_response_body_.
    void OnRead(int32_t result);
    
    // Reads the response body (asynchronously) into this->buffer_.
    // OnRead() will be called when bytes are received or when an error occurs.
    void ReadBody();
    
    void ReportResult(const std::string& text,
                    bool success);
    // Calls JS reportResult() and self-destroy (aka delete this;).
  void ReportResultAndDie(const std::string& text,
                          bool success);
                          
    bool isGoodKey(const std::string key, const std::string bkey, int pos, int bestp) const;
    private:
    
    std::vector<transpos>    	transpos_list;
	std::vector<correction>		correction_list;
    
    std::stack<std::string>    	memory;
	
	std::vector<data>			database;
    
    
    std::string    				m_sInput;
    std::string    				m_sResponse;
	std::string					m_sPrevInput;
    std::string    				m_sSuffix;
	std::string    				m_sPrevResponse;
    std::string					m_sKeyWord;
    std::string    				m_sSymbol;
    std::string					m_sUserName;
    
    Tokenizer    				tok;
    
    data    					  current_data;
    correction    				current_correction;
    
    unsigned int    			m_nUserRepeatCount;
	unsigned int				m_nCorrectionNum;
	unsigned int				m_nTransPosNum;
    
    vstring    					nullResponse;
    vstring    					response_list;
    vstring    					command_list;
    vstring    					topicChanger;
    vstring    					inputRepeat;
	vstring						comments;
	vstring    					subjectRecall;
	vstring    					noKeyWord;
	vstring						signOn;
    
    bool    					m_bWrongBoundary;
	bool						m_bGoodContext;
    
    PP_Instance instance_id_;
    pp::URLRequestInfo url_request_;
    pp::URLLoader url_loader_;  // URLLoader provides an API to download URLs.
    char buffer_[kBufferSize];  // buffer for pp::URLLoader::ReadResponseBody().
    std::string url_response_body_;  // Contains downloaded data.
    pp::CompletionCallbackFactory<GetURLHandler> cc_factory_;
};


#endif