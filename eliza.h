#ifndef EXAMPLES_GETURL_GETURL_HANDLER_H_
#define EXAMPLES_GETURL_GETURL_HANDLER_H_

#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/url_loader.h>
#include <ppapi/cpp/url_request_info.h>
#include <ppapi/cpp/instance.h>
#include <string>

class Eliza{
    public:
        Eliza(pp::Instance* instance_);
        ~Eliza();
        bool Start();
    
    private:
        static const int kBufferSize = 4096;
        void OnOpen(int32_t result);
        void OnRead(int32_t result);
        void ReadBody();
        
        void ReportResult(const std::string& fname,
            const std::string& text,
            bool success);
          // Calls JS reportResult() and self-destroy (aka delete this)
        void ReportResultAndDie(const std::string& fname,
            const std::string& text,
            bool success);
                
        PP_Instance instance_id_;
        pp::URLRequestInfo url_request_;
        pp::URLLoader url_loader_;  // URLLoader provides an API to download URLs.
        char buffer_[kBufferSize];  // buffer for pp::URLLoader::ReadResponseBody().
        std::string url_response_body_;  // Contains downloaded data.
        pp::CompletionCallbackFactory<GetURLHandler> cc_factory_;
}

#endif